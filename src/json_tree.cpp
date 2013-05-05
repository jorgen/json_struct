/*
 * Copyright © 2013 Jørgen Lind

 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.

 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
*/

#include "json_tree.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

namespace JT {

static Token::Type json_tree_type_lookup_dic[] = {
        Token::ObjectStart,
        Token::String,
        Token::Ascii,
        Token::Number,
        Token::Bool,
        Token::Null,
        Token::ArrayStart
};

static Property empty_property(Token::Ascii, Data());

std::pair<Node *, Error> TreeBuilder::build(const char *data, size_t data_size) const
{
    Tokenizer tokenizer;
    tokenizer.addData(data, data_size);
    return build(&tokenizer);
}

std::pair<Node *, Error> TreeBuilder::build(Tokenizer *tokenizer) const
{
    Token token;
    auto token_error = tokenizer->nextToken(&token);
    if (token_error != Error::NoError) {
        return std::pair<Node *, Error>(nullptr, token_error);
    }
    return build(&token, tokenizer);
}

std::pair<Node *, Error> TreeBuilder::build(Token *token, Tokenizer *tokenizer) const
{
    std::pair<Node *, Error> return_pair(nullptr, Error::CouldNotCreateNode);
    if (create_root_if_needed) {
        Node *root = nullptr;
        auto first_node = createNode(token, tokenizer);
        if (first_node.second != Error::NoError) {
            delete first_node.first;
            return return_pair;
        }

        Token next_token;
        Error error;
        while ((error = tokenizer->nextToken(&next_token)) == Error::NoError) {
            if (!root) {
                if (!token->name.size) {
                    return_pair.second = Error::MissingPropertyName;
                    return return_pair;
                }
                root = new ObjectNode();
                root->asObjectNode()->insertNode(
                        Property(token->name_type, token->name), first_node.first);
            }
            if (!next_token.name.size) {
                delete root;
                return_pair.second = Error::MissingPropertyName;
                return return_pair;
            }
            auto new_node = createNode(&next_token, tokenizer);
            if (new_node.second != Error::NoError) {
                delete root;
                delete new_node.first;
                return_pair.second = new_node.second;
                return return_pair;
            }
            root->asObjectNode()->insertNode(
                    Property(next_token.name_type, next_token.name), new_node.first);
        }
        if (error == Error::NeedMoreData) {
            if (!root)
                root = first_node.first;;
            return std::pair<Node *, Error>(root,Error::NoError);
        } else {
            return_pair.second = error;
            return return_pair;
        }
    } else {
        if (token->value_type == Token::ArrayStart
                || token->value_type == Token::ObjectStart) {
            return createNode(token,tokenizer);
        }
    }
    return return_pair;
}

TreeSerializer::TreeSerializer()
    : Serializer()
{ }
TreeSerializer::TreeSerializer(char *buffer, size_t size)
    : Serializer(buffer,size)
{ }

bool TreeSerializer::serialize(ObjectNode *rootObject)
{
    Token token;

    rootObject->fillStartToken(&token);
    if (!write(token))
        return false;

    if (!serializeNode(rootObject))
        return false;

    rootObject->fillEndToken(&token);
    return write(token);
}

bool TreeSerializer::serialize(ArrayNode *rootArray)
{
    Token token;
    rootArray->fillStartToken(&token);
    if (!write(token))
        return false;

    if (!serializeNode(rootArray))
        return false;

    rootArray->fillEndToken(&token);
    return write(token);
}

bool TreeSerializer::serializeNode(ObjectNode *objectNode)
{
    Token token;
    for (auto it = objectNode->begin(); it != objectNode->end(); ++it) {
        it.fillToken(&token);
        if (!write(token))
            return false;
        if (token.value_type == Token::ObjectStart) {
            ObjectNode *child_object = (*it).second->asObjectNode();
            assert(child_object);
            if (!serializeNode(child_object))
                return false;
            child_object->fillEndToken(&token);
            if (!write(token))
                return false;
        } else if (token.value_type == Token::ArrayStart) {
            ArrayNode *child_array = (*it).second->asArrayNode();
            assert(child_array);
            if (!serializeNode(child_array))
                return false;
            child_array->fillEndToken(&token);
            if (!write(token))
                return false;
        }
    }
    return true;
}

bool TreeSerializer::serializeNode(ArrayNode *arrayNode)
{
    Token token;
    for (size_t i = 0; i < arrayNode->size(); i++) {
        arrayNode->fillToken(i, &token);
        if (!write(token))
            return false;
        if (token.value_type == Token::ObjectStart) {
            ObjectNode *child_object = arrayNode->index(i)->asObjectNode();
            assert(child_object);
            if (!serializeNode(child_object))
                return false;
            child_object->fillEndToken(&token);
            if (!write(token))
                return false;
        } else if (token.value_type == Token::ArrayStart) {
            ArrayNode *child_array = arrayNode->index(i)->asArrayNode();
            assert(child_array);
            if (!serializeNode(child_array))
                return false;
            child_array->fillEndToken(&token);
            if (!write(token))
                return false;
        }
    }
    return true;
}

std::pair<Node *, Error> TreeBuilder::createNode(Token *token, Tokenizer *tokenizer) const
{
    std::pair<Node *, Error> ret(nullptr, Error::NoError);
    switch (token->value_type) {
        case Token::ArrayStart:
            {
                ArrayNode *return_node = new ArrayNode();
                ret.first = return_node;
                ret.second = return_node->fill(tokenizer, *this);
            }
            break;
        case Token::ObjectStart:
            {
                ObjectNode *return_node = new ObjectNode();
                ret.first = return_node;
                ret.second = return_node->fill(tokenizer, *this);
            }
            break;
        default:
            ret.first = Node::createValueNode(token);
            break;
    }
    return ret;
}

Node::Node(Node::Type type, const Data &data)
    : m_type(type)
    , m_delete_data_buffer(false)
    , m_data(data)
{
    if (m_data.temporary) {
        char *new_data = new char[m_data.size];
        m_data.data = new_data;
        m_data.temporary = false;
        memcpy(new_data, data.data, m_data.size);
        m_delete_data_buffer = true;
    }
}
Node::~Node()
{
    if (m_delete_data_buffer) {
        delete[] m_data.data;
    }
}

StringNode *Node::stringNodeAt(const std::string &path) const
{
    Node *node = nodeAt(path);
    if (node)
        return node->asStringNode();
    return nullptr;
}

static const std::string empty_string;

const std::string &Node::stringAt(const std::string &path) const
{
    StringNode *node = stringNodeAt(path);
    if (node)
        return node->string();
    return empty_string;
}

NumberNode *Node::numberNodeAt(const std::string &path) const
{
    Node *node = nodeAt(path);
    if (node)
        return node->asNumberNode();
    return nullptr;
}

double Node::numberAt(const std::string &path) const
{
    NumberNode *node = numberNodeAt(path);
    if (node)
        return node->number();
    return 0;
}

BooleanNode *Node::booleanNodeAt(const std::string &path) const
{
    Node *node = nodeAt(path);
    if (node)
        return node->asBooleanNode();
    return nullptr;
}

bool Node::booleanAt(const std::string &path) const
{
    BooleanNode *node = booleanNodeAt(path);
    if (node)
        return node->boolean();
    return false;
}

NullNode *Node::nullNodeAt(const std::string &path) const
{
    Node *node = nodeAt(path);
    if (node)
        return node->asNullNode();
    return nullptr;
}

bool Node::nullAt(const std::string &path) const
{
    NullNode *node = nullNodeAt(path);
    if (node)
        return true;
    return false;
}

ArrayNode *Node::arrayNodeAt(const std::string &path) const
{
    Node *node = nodeAt(path);
    if (node)
        return node->asArrayNode();
    return nullptr;
}

ObjectNode *Node::objectNodeAt(const std::string &path) const
{
    Node *node = nodeAt(path);
    if (node)
        return node->asObjectNode();
    return nullptr;
}

const Data &Node::data() const
{
    return m_data;
}

bool Node::addValueToObject(const std::string &path, const std::string &value, JT::Token::Type type)
{
    if (type == JT::Token::ObjectStart
            || type == JT::Token::ArrayStart
            || type == JT::Token::ObjectEnd
            || type == JT::Token::ArrayEnd)
        return false;

    JT::ObjectNode *last_node = asObjectNode();
    if (!last_node)
        return false;

    std::vector<std::string> path_vector;

    size_t pos = 0;
    while (pos < path.size()) {
        size_t new_pos = path.find('.', pos);
        path_vector.push_back(path.substr(pos, new_pos - pos));
        pos = new_pos;
        if (new_pos != std::string::npos)
            pos++;
    }

    for (size_t i = 0; i < path_vector.size(); i++) {
        if (i == path_vector.size() -1) {
            JT::Token token;
            token.value.data = value.c_str();
            token.value.size = value.size();
            token.value_type = type;
            token.value.temporary = true;
            JT::Node *node = Node::createValueNode(&token);
            JT::Property prop(path_vector[i]);
            last_node->insertNode(prop,node,true);
        } else {
            JT::Node *new_child = last_node->node(path_vector[i]);
            JT::ObjectNode *object_child_node = 0;
            if (new_child) {
                object_child_node = new_child->asObjectNode();
            } else {
                JT::Property prop(path_vector[i]);
                object_child_node = new JT::ObjectNode();
                last_node->insertNode(prop,object_child_node,false);
            }
            if (!object_child_node)
                return false;
            last_node = object_child_node;
        }
    }
    return true;
}

Node *Node::createValueNode(Token *token)
{
    Node *return_node;
    switch (token->value_type) {
        case Token::String:
        case Token::Ascii:
            return_node = new StringNode(token);
            break;
        case Token::Number:
            return_node = new NumberNode(token);
            break;
        case Token::Bool:
            return_node = new BooleanNode(token);
            break;
        case Token::Null:
            return_node = new NullNode(token);
            break;
        default:
            return_node = 0;
            break;
    }
    return return_node;
}

Node *Node::nodeAt(const std::string &path) const
{
    (void) path;
    return nullptr;
}

StringNode *Node::asStringNode()
{
    if (m_type == String)
        return static_cast<StringNode *>(this);
    return nullptr;
}

const StringNode *Node::asStringNode() const
{
    if (m_type == String)
        return static_cast<const StringNode *>(this);
    return nullptr;
}

NumberNode *Node::asNumberNode()
{
    if (m_type == Number)
        return static_cast<NumberNode *>(this);
    return nullptr;
}

const NumberNode *Node::asNumberNode() const
{
    if (m_type == Number)
        return static_cast<const NumberNode*>(this);
    return nullptr;
}

BooleanNode *Node::asBooleanNode()
{
    if (m_type == Bool)
        return static_cast<BooleanNode *>(this);
    return nullptr;
}

const BooleanNode *Node::asBooleanNode() const
{
    if (m_type == Bool)
        return static_cast<const BooleanNode *>(this);
    return nullptr;
}

NullNode *Node::asNullNode()
{
    if (m_type == Null)
        return static_cast<NullNode *>(this);
    return nullptr;
}

const NullNode *Node::asNullNode() const
{
    if (m_type == Null)
        return static_cast<const NullNode *>(this);
    return nullptr;
}

ArrayNode *Node::asArrayNode()
{
    if (m_type == Array)
        return static_cast<ArrayNode *>(this);
    return nullptr;
}

const ArrayNode *Node::asArrayNode() const
{
    if (m_type == Array)
        return static_cast<const ArrayNode *>(this);
    return nullptr;
}

ObjectNode *Node::asObjectNode()
{
    if (m_type == Object)
        return static_cast<ObjectNode *>(this);
    return nullptr;
}

const ObjectNode *Node::asObjectNode() const
{
    if (m_type == Object)
        return static_cast<const ObjectNode *>(this);
    return nullptr;
}

Property::Property(Token::Type type, const Data data)
    : m_type(type)
    , m_delete_data_buffer(false)
    , m_data(data)
{
    if (data.temporary) {
        char *new_data = new char[m_data.size];
        m_data.data = new_data;
        memcpy(new_data, data.data,m_data.size);
        m_delete_data_buffer = true;
    }
}

Property::Property(const std::string string)
    : m_type(Token::Ascii)
    , m_delete_data_buffer(true)
{
    if (string.front() == '"') {
        m_type = Token::String;
    }

    char *new_data = new char[string.size()];
    m_data.data = new_data;
    m_data.size = string.size();
    m_data.temporary = true;
    memcpy(new_data, string.c_str(),  string.size());
}

Property::Property(const Property &other)
    : m_type(other.m_type)
    , m_delete_data_buffer(other.m_delete_data_buffer)
    , m_data(other.m_data)
{
    if (m_delete_data_buffer) {
        char *new_data = new char[m_data.size];
        m_data.data = new_data;
        memcpy(new_data, other.m_data.data,m_data.size);
    }
}

Property::Property(Property &&other)
    : m_type(other.m_type)
    , m_delete_data_buffer(other.m_delete_data_buffer)
    , m_data(other.m_data)
{
    other.m_delete_data_buffer = false;
}

Property::~Property()
{
    if (m_delete_data_buffer)
        delete[] m_data.data;
}

Token::Type Property::type() const
{
    return m_type;
}

std::string Property::string() const
{
    if (m_type == Token::String)
        return std::string(m_data.data+1, m_data.size -2);
    else
        return std::string(m_data.data, m_data.size);
}

double Property::number(double defaultValue) const
{
    if (m_type == Token::Number) {
        char *end;
        double number = strtod(m_data.data,&end);
        if (end == m_data.data)
            return defaultValue;
        return number;
    }
    return defaultValue;
}

bool Property::boolean(bool defaultValue) const
{
    if (m_type == Token::Bool) {
        if (*m_data.data == 'T' || *m_data.data == 't')
            return true;
        else
            return false;
    }
    return defaultValue;
}

bool Property::isNull(bool defaultValue) const
{
    if(m_type == Token::Null)
        return true;
    return defaultValue;
}

bool Property::isEmpty() const
{
    return m_data.size == 0;
}

const Property &Property::get(const std::string &) const
{
    return empty_property;
}

Property &Property::get(const std::string &)
{
    return empty_property;
}

const Property &Property::get(int) const
{
    return empty_property;
}

Property &Property::get(int)
{
    return empty_property;
}

void Property::remove(const std::string &)
{ }

void Property::remove(int)
{ }

void Property::insert(const std::string &, const Property &, const Property &)
{ }

void Property::insert(const Property &, const Property &)
{ }

bool Property::compareData(const Property &property) const
{
    if (property.m_data.size != m_data.size)
        return false;
    return memcmp(m_data.data, property.m_data.data, m_data.size);
}
bool Property::compareString(const Property &property) const
{
    const char *other_data = property.m_data.data;
    size_t other_size = property.m_data.size;
    if (property.m_type == Token::String) {
        other_data++; other_size -= 2;
    }

    const char *this_data = m_data.data;
    size_t this_size = m_data.size;
    if (m_type == Token::String) {
        this_data++; this_size -= 2;
    }

    if (this_size != other_size)
        return false;
    return memcmp(this_data, other_data, this_size) == 0;
}

bool Property::compareString(const std::string &property_name) const
{
    const char *this_data = m_data.data;
    size_t this_size = m_data.size;
    if (m_type == Token::String) {
        this_data++; this_size -= 2;
    }

    if (this_size != property_name.size())
        return false;
    return memcmp(this_data, property_name.c_str(), this_size) == 0;
}

const Data &Property::data() const
{
    return m_data;
}

Property &Property::operator= (const Property &other)
{
    if (m_delete_data_buffer)
        delete[] m_data.data;
    m_type = other.m_type;
    m_delete_data_buffer = other.m_delete_data_buffer;
    m_data = other.m_data;
    if (m_delete_data_buffer) {
        char *new_data = new char[m_data.size];
        m_data.data = new_data;
        memcpy(new_data, other.m_data.data,m_data.size);
    }
    return *this;
}

ObjectNode::ObjectNode()
    : Node(Node::Object, Data("{",1,false))
{
}

ObjectNode::~ObjectNode()
{
    for (auto it = m_data.begin();
            it != m_data.end(); it++) {
       delete it->second;
    }
}

Node *ObjectNode::nodeAt(const std::string &path) const
{
    size_t first_dot = path.find('.');

    if (first_dot == 0)
        return nullptr;

    if (first_dot == std::string::npos) {
        return findNode(path);
    }

    std::string first_node(path.substr(0,first_dot));
    Node *child_node = findNode(first_node);
    if (!child_node)
        return nullptr;
    return child_node->nodeAt(path.substr(first_dot+1));
}

Node *ObjectNode::node(const std::string &child_node) const
{
    return findNode(child_node);
}

void ObjectNode::insertNode(const Property &name, Node *node, bool replace)
{
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        if ((*it).first.compareString(name)) {
            if (replace) {
                (*it).second = node;
            }
            return;
        }
    }
    m_data.push_back(std::pair<Property, Node *>(name,node));
}

Node *ObjectNode::take(const std::string &name)
{
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        if ((*it).first.compareString(name)) {
            Node *return_node = (*it).second;
            m_data.erase(it);
            return return_node;
        }
    }
    return nullptr;
}

Error ObjectNode::fill(Tokenizer *tokenizer, const TreeBuilder &builder)
{
    Token token;
    Error error;
    while ((error = tokenizer->nextToken(&token)) == Error::NoError) {
        if (token.value_type == Token::ObjectEnd) {
            return Error::NoError;
        }
        auto created = builder.createNode(&token, tokenizer);
        if (created.second != Error::NoError) {
            return created.second;
        }

        assert(token.name.size);
        insertNode(Property(token.name_type, token.name), created.first, true);
    }
    return error;
}

ObjectNode::Iterator::Iterator(std::vector<std::pair<Property, Node *>>::const_iterator it)
    : m_it(it)
{ }

void ObjectNode::Iterator::fillToken(Token *token) const
{
    const Property &name = m_it->first;
    token->name = name.data();
    token->name_type = name.type();

    token->value = m_it->second->data();
    token->value_type = json_tree_type_lookup_dic[m_it->second->type()];
}

const std::pair<Property, Node *> &ObjectNode::Iterator::operator*() const
{
    return *m_it;
}

ObjectNode::Iterator &ObjectNode::Iterator::operator++()
{
    ++m_it;
    return *this;
}

ObjectNode::Iterator ObjectNode::Iterator::operator++(int)
{
    ObjectNode::Iterator self = *this;
    m_it++;
    return self;
}

ObjectNode::Iterator &ObjectNode::Iterator::operator--()
{
    --m_it;
    return *this;
}

ObjectNode::Iterator ObjectNode::Iterator::operator--(int)
{
    ObjectNode::Iterator self = *this;
    m_it--;
    return self;
}

bool ObjectNode::Iterator::operator==(const Iterator &other) const
{
    return m_it == other.m_it;
}

bool ObjectNode::Iterator::operator!=(const Iterator &other) const
{
    return m_it != other.m_it;
}

ObjectNode::Iterator ObjectNode::begin() const
{
    return ObjectNode::Iterator(m_data.begin());
}

ObjectNode::Iterator ObjectNode::end() const
{
    return Iterator(m_data.end());
}

void ObjectNode::fillStartToken(Token *token)
{
    token->name = Data();
    token->name_type = Token::String;
    token->value = {"{",1,true};
    token->value_type = Token::ObjectStart;
}

void ObjectNode::fillEndToken(Token *token)
{
    token->name = Data();
    token->name_type = Token::String;
    token->value = {"}",1,true};
    token->value_type = Token::ObjectEnd;
}

Node *ObjectNode::findNode(const std::string name) const
{
    for(auto it = m_data.begin(); it != m_data.end(); ++it) {
        if ((*it).first.compareString(name))
            return (*it).second;
    }
    return nullptr;
}

StringNode::StringNode(Token *token)
    : Node(String, token->value)
    , m_string(token->value.data + 1, token->value.size - 2)
{
}

const std::string &StringNode::string() const
{
    return m_string;
}

void StringNode::setString(const std::string &string)
{
    m_string = string;
}

NumberNode::NumberNode(Token *token)
    : Node(Number, token->value)
{
    std::string null_terminated(token->value.data, token->value.size);
    char **success = 0;
    m_number = strtod(null_terminated.c_str(), success);
    if ((char *)success == null_terminated.c_str()) {
        fprintf(stderr, "numbernode failed to convert token to double\n");
    }
}

BooleanNode::BooleanNode(Token *token)
    : Node(Bool,token->value)
{
    if (*token->value.data == 't' || *token->value.data == 'T')
        m_boolean = true;
    else
        m_boolean = false;
}

NullNode::NullNode(Token *token)
    : Node(Null, token->value)
{ }

ArrayNode::ArrayNode()
    : Node(Array, Data("[",1,false))
{
}

ArrayNode::~ArrayNode()
{
    for (auto it = m_vector.begin(); it != m_vector.end(); it++) {
        delete *it;
    }
}

void ArrayNode::insert(Node *node, size_t index)
{
    if (index >= m_vector.size()) {
        m_vector.push_back(node);
        return;
    }

    auto it = m_vector.begin();
    m_vector.insert(it + index, node);
}

void ArrayNode::append(Node *node)
{
    m_vector.push_back(node);
}

Node *ArrayNode::index(size_t index)
{
    if (index >= m_vector.size()) {
        return nullptr;
    }
    auto it = m_vector.begin();
    return *(it+index);
}

Node *ArrayNode::take(size_t index)
{
    if (index >= m_vector.size()) {
        return nullptr;
    }

    auto it = m_vector.begin();
    Node *return_node = *(it + index);
    m_vector.erase(it+index);
    return return_node;
}

size_t ArrayNode::size()
{
    return m_vector.size();
}

void ArrayNode::fillToken(size_t index, Token *token) const
{
    token->name = Data();
    token->name_type = Token::String;
    token->value = m_vector.at(index)->data();
    token->value_type = json_tree_type_lookup_dic[m_vector.at(index)->type()];
}

void ArrayNode::fillStartToken(Token *token)
{
    token->name = Data();
    token->name_type = Token::String;
    token->value.data = "[";
    token->value.size = 1;
    token->value_type = Token::ArrayStart;
}

void ArrayNode::fillEndToken(Token *token)
{
    token->name = Data();
    token->name_type = Token::String;
    token->value.data = "]";
    token->value.size = 1;
    token->value_type = Token::ArrayEnd;
}

Error ArrayNode::fill(Tokenizer *tokenizer, const TreeBuilder &builder)
{
    Token token;
    Error error;
    while ((error = tokenizer->nextToken(&token)) == Error::NoError) {
        if (token.value_type == Token::ArrayEnd) {
            return Error::NoError;
        }
        auto created = builder.createNode(&token, tokenizer);

        if (created.second != Error::NoError)
            return created.second;

        m_vector.push_back(created.first);
    }

    return error;
}

} //namespace JT
