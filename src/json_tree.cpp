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
#include <assert.h>

JsonNode::JsonNode(JsonNode::Type type)
    : m_type(type)
{ }
JsonNode::~JsonNode()
{
}

StringNode *JsonNode::stringNodeAt(const std::string &path) const
{
    JsonNode *node = nodeAt(path);
    if (node)
        return node->asStringNode();
    return nullptr;
}

NumberNode *JsonNode::numberNodeAt(const std::string &path) const
{
    JsonNode *node = nodeAt(path);
    if (node)
        return node->asNumberNode();
    return nullptr;
}

BooleanNode *JsonNode::booleanNodeAt(const std::string &path) const
{
    JsonNode *node = nodeAt(path);
    if (node)
        return node->asBooleanNode();
    return nullptr;
}

NullNode *JsonNode::nullNodeAt(const std::string &path) const
{
    JsonNode *node = nodeAt(path);
    if (node)
        return node->asNullNode();
    return nullptr;
}

ArrayNode *JsonNode::arrayNodeAt(const std::string &path) const
{
    JsonNode *node = nodeAt(path);
    if (node)
        return node->asArrayNode();
    return nullptr;
}

ObjectNode *JsonNode::objectNodeAt(const std::string &path) const
{
    JsonNode *node = nodeAt(path);
    if (node)
        return node->asObjectNode();
    return nullptr;
}

JsonNode *JsonNode::nodeAt(const std::string &path) const
{
    return nullptr;
}

StringNode *JsonNode::asStringNode()
{
    if (m_type == String)
        return static_cast<StringNode *>(this);
    return nullptr;
}

const StringNode *JsonNode::asStringNode() const
{
    if (m_type == String)
        return static_cast<const StringNode *>(this);
    return nullptr;
}

NumberNode *JsonNode::asNumberNode()
{
    if (m_type == Number)
        return static_cast<NumberNode *>(this);
    return nullptr;
}

const NumberNode *JsonNode::asNumberNode() const
{
    if (m_type == Number)
        return static_cast<const NumberNode*>(this);
    return nullptr;
}

BooleanNode *JsonNode::asBooleanNode()
{
    if (m_type == Bool)
        return static_cast<BooleanNode *>(this);
    return nullptr;
}

const BooleanNode *JsonNode::asBooleanNode() const
{
    if (m_type == Bool)
        return static_cast<const BooleanNode *>(this);
    return nullptr;
}

NullNode *JsonNode::asNullNode()
{
    if (m_type == Null)
        return static_cast<NullNode *>(this);
    return nullptr;
}

const NullNode *JsonNode::asNullNode() const
{
    if (m_type == Null)
        return static_cast<const NullNode *>(this);
    return nullptr;
}

ArrayNode *JsonNode::asArrayNode()
{
    if (m_type == Array)
        return static_cast<ArrayNode *>(this);
    return nullptr;
}

const ArrayNode *JsonNode::asArrayNode() const
{
    if (m_type == Array)
        return static_cast<const ArrayNode *>(this);
    return nullptr;
}

ObjectNode *JsonNode::asObjectNode()
{
    if (m_type == Object)
        return static_cast<ObjectNode *>(this);
    return nullptr;
}

const ObjectNode *JsonNode::asObjectNode() const
{
    if (m_type == Object)
        return static_cast<const ObjectNode *>(this);
    return nullptr;
}

ObjectNode::ObjectNode()
    : JsonNode(JsonNode::Object)
{
}

ObjectNode::~ObjectNode()
{
    for (auto it = m_map.begin();
            it != m_map.end(); it++) {
       delete it->second;
    }
}

JsonNode *ObjectNode::nodeAt(const std::string &path) const
{
    size_t first_dot = path.find('.');

    if (first_dot == 0)
        return nullptr;

    if (first_dot == std::string::npos) {
        auto it = m_map.find(path);
        if (it == m_map.end())
            return nullptr;
        else return it->second;
    }

    std::string first_node(path.substr(0,first_dot));
    auto it = m_map.find(first_node);
    if (it == m_map.end())
        return nullptr;
    JsonNode *child_node = it->second;
    return child_node->nodeAt(path.substr(first_dot+1));
}

JsonNode *ObjectNode::node(const std::string &child_node) const
{
    auto it = m_map.find(child_node);
    if (it == m_map.end())
        return nullptr;
    return it->second;
}

void ObjectNode::insertNode(const std::string &name, JsonNode *node, bool replace)
{
    auto ret = m_map.insert(std::pair<std::string, JsonNode *>(name, node));
    if (ret.second == false && replace) {
        delete ret.first->second;
        m_map.erase(ret.first);
        ret = m_map.insert(std::pair<std::string, JsonNode *>(name, node));
        assert(ret.second == true);
    }
}

JsonNode *ObjectNode::take(const std::string &name)
{
    auto it = m_map.find(name);
    if (it == m_map.end())
        return nullptr;
    JsonNode *child_node = it->second;
    m_map.erase(it);
    return child_node;
}

JsonNodeError ObjectNode::fill(JsonTokenizer *tokenizer, JsonNode *continue_from)
{
    //TODO: make this coincide with ArrayNode::fill and make it shorter
    if (continue_from && continue_from != this) {
        for (auto it = m_map.begin(); it != m_map.end() && continue_from; it++) {
            switch ((*it).second->type()) {
                case Object:
                    {
                        JsonNodeError ret = (*it).second->asObjectNode()->fill(tokenizer, continue_from);
                        if (ret.error == JsonError::NoError) {
                            continue_from = nullptr;
                        } else if (ret.error != JsonError::NodeNotFound) {
                            return ret;
                        }
                    }
                    break;
                case Array:
                    {
                        JsonNodeError ret = (*it).second->asArrayNode()->fill(tokenizer,continue_from);
                        if (ret.error == JsonError::NoError) {
                            continue_from = nullptr;
                        } else if (ret.error != JsonError::NodeNotFound) {
                            return ret;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    JsonToken token;
    JsonError error;
    while ((error = tokenizer->nextToken(&token)) == JsonError::NoError) {
        if (token.data_type == JsonToken::ObjectEnd) {
            return JsonNodeError();
        }
        auto created = JsonNode::create(&token, tokenizer);
        if (created.second.error == JsonError::NeedMoreData) {
            if (created.second.errorNode == nullptr)
                created.second.errorNode = this;
            return created.second;
        }

        if (created.first == 0) {
            created.second.errorNode = this;
            return created.second;
        }
        assert(token.name_length);
        insertNode(std::string(token.name, token.name_length), created.first, true);
    }
    return JsonNodeError(error, this);
}

size_t ObjectNode::printSize(const JsonPrinterOption &option, int depth)
{
    size_t object_size = 2;
    bool first = true;
    int inc_depth = depth + 1;
    for (auto it = m_map.begin(); it != m_map.end(); ++it) {
        if (first) {
            first = false;
        } else {
            object_size += 2;
        }
        object_size += inc_depth * option.shiftSize() + (*it).first.size() + 3 + (*it).second->printSize(option, inc_depth);
        if (!option.ascii_name())
            object_size += 2;
    }
    object_size += depth * option.shiftSize() + 1;
    return object_size;
}

StringNode::StringNode(JsonToken *token)
    : JsonNode(String)
    , m_string(token->data, token->data_length)
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

size_t StringNode::printSize(const JsonPrinterOption &option, int depth)
{
    return m_string.size() + 2;
}

NumberNode::NumberNode(JsonToken *token)
    : JsonNode(Number)
{
    std::string null_terminated(token->data, token->data_length);
    char **success = 0;
    m_number = strtod(null_terminated.c_str(), success);
    if ((char *)success == null_terminated.c_str()) {
        fprintf(stderr, "numbernode failed to convert token to double\n");
    }
}

size_t NumberNode::printSize(const JsonPrinterOption &option, int depth)
{
    char buff[20];
    size_t size  = snprintf(buff, sizeof(buff), "%f", m_number);
    assert(size > 0);
    return size;
}
BooleanNode::BooleanNode(JsonToken *token)
    : JsonNode(Bool)
{
    if (*token->data == 't' || *token->data == 'T')
        m_boolean = true;
    else
        m_boolean = false;
}

size_t BooleanNode::printSize(const JsonPrinterOption &option, int depth)
{
    return m_boolean ? 4 : 5;
}

NullNode::NullNode(JsonToken *token)
    : JsonNode(Null)
{ }

size_t NullNode::printSize(const JsonPrinterOption &option, int depth)
{
    return 4;
}

ArrayNode::ArrayNode()
    : JsonNode(Array)
{
}

ArrayNode::~ArrayNode()
{
    for (auto it = m_vector.begin(); it != m_vector.end(); it++) {
        delete *it;
    }
}

void ArrayNode::insert(JsonNode *node, size_t index)
{
    if (index >= m_vector.size()) {
        m_vector.push_back(node);
        return;
    }

    auto it = m_vector.begin();
    m_vector.insert(it + index, node);
}

void ArrayNode::append(JsonNode *node)
{
    m_vector.push_back(node);
}

JsonNode *ArrayNode::index(size_t index)
{
    if (index >= m_vector.size()) {
        return nullptr;
    }
    auto it = m_vector.begin();
    return *(it+index);
}

JsonNode *ArrayNode::take(size_t index)
{
    if (index >= m_vector.size()) {
        return nullptr;
    }

    auto it = m_vector.begin();
    JsonNode *return_node = *(it + index);
    m_vector.erase(it+index);
    return return_node;
}

size_t ArrayNode::size()
{
    return m_vector.size();
}

JsonNodeError ArrayNode::fill(JsonTokenizer *tokenizer, JsonNode *continue_from)
{
    if (continue_from && continue_from != this) {
        for (auto it = m_vector.begin(); it != m_vector.end() && continue_from; it++) {
            switch ((*it)->type()) {
                case Object:
                    {
                        JsonNodeError ret = (*it)->asObjectNode()->fill(tokenizer, continue_from);
                        if (ret.error == JsonError::NoError) {
                            continue_from = nullptr;
                        } else if (ret.error != JsonError::NodeNotFound) {
                            return ret;
                        }
                    }
                    break;
                case Array:
                    {
                        JsonNodeError ret = (*it)->asArrayNode()->fill(tokenizer,continue_from);
                        if (ret.error == JsonError::NoError) {
                            continue_from = nullptr;
                        } else if (ret.error != JsonError::NodeNotFound) {
                            return ret;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    JsonToken token;
    JsonError error;
    while ((error = tokenizer->nextToken(&token)) == JsonError::NoError) {
        if (token.data_type == JsonToken::ArrayEnd) {
            return JsonNodeError();
        }
        auto created = JsonNode::create(&token, tokenizer);
        if (created.second.error == JsonError::NeedMoreData) {
            if (created.second.errorNode == nullptr)
                created.second.errorNode = this;
            return created.second;
        }

        if (created.first == 0) {
            created.second.errorNode = this;
            return created.second;
        }

        m_vector.push_back(created.first);
    }
    return JsonNodeError(error, this);
}

size_t ArrayNode::printSize(const JsonPrinterOption &option, int depth)
{
    size_t return_size = 2;

    bool first = true;
    int inc_depth = depth + 1;
    for (auto it = m_vector.begin(); it != m_vector.end(); ++it) {
        if (first) {
            first = false;
        } else {
            return_size += 2;
        }

        return_size += inc_depth * option.shiftSize() + (*it)->printSize(option, inc_depth);
    }
    return_size += depth * option.shiftSize() + 1;
    return return_size;
}

std::pair<JsonNode *, JsonNodeError> JsonNode::create(JsonToken *token, JsonTokenizer *tokenizer, JsonNode *continue_from)
{
    std::pair<JsonNode *, JsonNodeError> ret(nullptr, JsonError::NoError);
    switch (token->data_type) {
        case JsonToken::ArrayStart:
            {
                ArrayNode *return_node = new ArrayNode();
                ret.first = return_node;
                ret.second = return_node->fill(tokenizer);
            }
            break;
        case JsonToken::ObjectStart:
            {
                ObjectNode *return_node = new ObjectNode();
                ret.first = return_node;
                ret.second = return_node->fill(tokenizer);
            }
            break;
        case JsonToken::String:
        case JsonToken::Ascii:
            ret.first = new StringNode(token);
            break;
        case JsonToken::Number:
            ret.first = new NumberNode(token);
            break;
        case JsonToken::Bool:
            ret.first = new BooleanNode(token);
            break;
        case JsonToken::Null:
            ret.first = new NullNode(token);
            break;
        default:
            break;
    }
    return ret;
}

std::pair<JsonNode *, JsonNodeError> JsonNode::create(JsonTokenizer *tokenizer, JsonNode *continue_from)
{
    JsonToken token;
    auto token_error = tokenizer->nextToken(&token);
    if (token_error != JsonError::NoError) {
        return std::pair<JsonNode *, JsonNodeError>(nullptr, token_error);
    }
    return JsonNode::create(&token, tokenizer, continue_from);
}
