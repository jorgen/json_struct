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
                        std::string(token->name.data, token->name.size), first_node.first);
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
                    std::string(next_token.name.data, next_token.name.size), new_node.first);
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
        case Token::String:
        case Token::Ascii:
            ret.first = new StringNode(token);
            break;
        case Token::Number:
            ret.first = new NumberNode(token);
            break;
        case Token::Bool:
            ret.first = new BooleanNode(token);
            break;
        case Token::Null:
            ret.first = new NullNode(token);
            break;
        default:
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

NumberNode *Node::numberNodeAt(const std::string &path) const
{
    Node *node = nodeAt(path);
    if (node)
        return node->asNumberNode();
    return nullptr;
}

BooleanNode *Node::booleanNodeAt(const std::string &path) const
{
    Node *node = nodeAt(path);
    if (node)
        return node->asBooleanNode();
    return nullptr;
}

NullNode *Node::nullNodeAt(const std::string &path) const
{
    Node *node = nodeAt(path);
    if (node)
        return node->asNullNode();
    return nullptr;
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

Data Node::data() const
{
    return m_data;
}

Node *Node::nodeAt(const std::string &path) const
{
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

void ObjectNode::insertNode(const std::string &name, Node *node, bool replace)
{
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        if ((*it).first == name) {
            if (replace) {
                (*it).second = node;
            }
            return;
        }
    }
    m_data.push_back(std::pair<std::string,Node *>(name,node));
}

Node *ObjectNode::take(const std::string &name)
{
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        if ((*it).first == name) {
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
        insertNode(std::string(token.name.data, token.name.size), created.first, true);
    }
    return error;
}

size_t ObjectNode::printSize(const PrinterOption &option, int depth)
{
    depth++;
    size_t return_size = 0;
    bool first = true;
    int shift_width = depth * option.shiftSize();

    if (option.pretty()) {
        return_size += 2;
    } else {
        return_size++;
    }

    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        const std::string &property = (*it).first;
        if (first) {
            first = false;
        } else {
            if (option.pretty()) {
                return_size+=2;
            } else {
                return_size++;
            }
        }
        if (option.pretty()) {
            return_size += shift_width;
        }
        return_size += property.size() + 2;

        if (option.pretty()) {
            return_size += 3;
        } else {
            return_size += 1;
        }

        return_size += (*it).second->printSize(option,depth);
    }
    if (option.pretty()) {
        return_size += 1 + (shift_width - option.shiftSize());
    }
    return_size += 1;
    return return_size;
}

bool ObjectNode::print(PrintHandler &buffers, const PrinterOption &option , int depth)
{
    depth++;
    if (option.pretty()) {
        if (!buffers.write("{\n",2))
            return false;
    } else {
        if (!buffers.write("{", 1))
            return false;
    }

    int shift_width = option.shiftSize() * depth;

    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        const std::string &property = (*it).first;
        if (it != m_data.begin()) {
            if (option.pretty()) {
                if (!buffers.write(",\n",2))
                    return false;
           } else {
                if (!buffers.write(",",1))
                    return false;
            }
        }
        if (option.pretty()) {
            if (!buffers.write(
                        std::string(shift_width,' ').c_str(),shift_width))
                return false;
        }
        if (!buffers.write("\"",1))
            return false;
        if (!buffers.write(property.c_str(), property.size()))
            return false;
        if (!buffers.write("\"",1))
            return false;
        if (option.pretty()) {
            const char delimiter[] = " : ";
            if (!buffers.write(delimiter, 3))
                return false;
        } else {
            const char delimiter[] = ":";
            if (!buffers.write(delimiter, 1))
                return false;
        }
        Node *print_node = node(property);
        if (!print_node->print(buffers,option,depth))
            return false;
    }

    if (option.pretty()) {
        std::string before_close_bracket("\n");
        before_close_bracket.append(std::string(shift_width - option.shiftSize(),' '));
        if (!buffers.write(before_close_bracket.c_str(), before_close_bracket.size()))
            return false;
    }
    if (!buffers.write("}",1))
        return false;
    return true;
}

void ObjectNode::Iterator::fillToken(Token *token) const
{
    std::string name = m_it->first;
    token->name = Data(name.c_str(),name.size(), false);
    token->name_type = Token::String;

    token->value = m_it->second->data();
    token->value_type = json_tree_type_lookup_dic[m_it->second->type()];
}

const std::pair<std::string, Node *> &ObjectNode::Iterator::operator*() const
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

ObjectNode::Iterator::Iterator()
{
}

Node *ObjectNode::findNode(const std::string name) const
{
    for(auto it = m_data.begin(); it != m_data.end(); ++it) {
        if ((*it).first == name)
            return (*it).second;
    }
    return nullptr;
}

StringNode::StringNode(Token *token)
    : Node(String, token->value)
    , m_string(token->value.data, token->value.size)
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

size_t StringNode::printSize(const PrinterOption &option, int depth)
{
    return m_string.size() + 2;
}

bool StringNode::print(PrintHandler &buffers, const PrinterOption &option , int depth)
{
    if (!buffers.write("\"",1))
        return false;
    if (!buffers.write(m_string.c_str(), m_string.size()))
        return false;
    if (!buffers.write("\"",1))
        return false;
    return true;
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

size_t NumberNode::printSize(const PrinterOption &option, int depth)
{
    char buff[20];
    size_t size  = snprintf(buff, sizeof(buff), "%f", m_number);
    assert(size > 0);
    return size;
}

bool NumberNode::print(PrintHandler &buffers, const PrinterOption &option , int depth)
{
    char buff[20];
    size_t size  = snprintf(buff, sizeof(buff), "%f", m_number);
    return buffers.write(buff,size);
}

BooleanNode::BooleanNode(Token *token)
    : Node(Bool,token->value)
{
    if (*token->value.data == 't' || *token->value.data == 'T')
        m_boolean = true;
    else
        m_boolean = false;
}

size_t BooleanNode::printSize(const PrinterOption &option, int depth)
{
    return m_boolean ? 4 : 5;
}

bool BooleanNode::print(PrintHandler &buffers, const PrinterOption &option , int depth)
{
    if (m_boolean)
        return buffers.write("true",4);
    else
        return buffers.write("false",5);
}

NullNode::NullNode(Token *token)
    : Node(Null, token->value)
{ }

size_t NullNode::printSize(const PrinterOption &option, int depth)
{
    return 4;
}

bool NullNode::print(PrintHandler &buffers, const PrinterOption &option , int depth)
{
    return buffers.write("null",4);
}

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

size_t ArrayNode::printSize(const PrinterOption &option, int depth)
{
    depth++;
    int shift_width = option.shiftSize() * depth;
    size_t return_size = 0;

    if (option.pretty()) {
        return_size += 2;
    }else {
        return_size++;
    }

    bool first = true;
    for (auto it = m_vector.begin(); it != m_vector.end(); ++it) {
        if (first) {
            first = false;
        } else {
            if (option.pretty())
                return_size += 2;
            else
                return_size++;
        }
        if (option.pretty()) {
            return_size += shift_width;
        }

        return_size += (*it)->printSize(option, depth);
    }

    if (option.pretty()) {
        return_size += 1 + (shift_width - option.shiftSize());
    }

    return_size++;

    return return_size;
}

bool ArrayNode::print(PrintHandler &buffers, const PrinterOption &option , int depth)
{
    depth++;
    if (option.pretty()) {
        if (!buffers.write("[\n",2))
            return false;
    } else {
        if (!buffers.write("[",1))
            return false;
    }

    int shift_width = option.shiftSize() * depth;

    for (auto it = m_vector.begin(); it != m_vector.end(); ++it) {
        if (it != m_vector.begin()) {
            if (option.pretty()) {
                if (!buffers.write(",\n",2))
                    return false;
            } else {
                if (!buffers.write(",",1))
                    return false;
            }
        }
        if (option.pretty()) {
            if (!buffers.write(std::string(shift_width,' ').c_str(),shift_width))
                return false;
        }
        if (!(*it)->print(buffers,option,depth))
            return false;
    }

    if (option.pretty()) {
        if (!buffers.write("\n",1))
            return false;
        if (!buffers.write(std::string(shift_width - option.shiftSize(),' ').c_str(),shift_width - option.shiftSize()))
            return false;
    }
    if (!buffers.write("]",1))
        return false;
    return true;

}

} //namespace JT
