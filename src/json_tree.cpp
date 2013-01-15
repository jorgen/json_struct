/*
 * Copyright © 2nullptr12 Jørgen Lind

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

#include <assert.h>

JsonNode::JsonNode(JsonNode::Type type)
    : m_type(type)
{ }

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
    for (std::map<std::string, JsonNode *>::iterator it = m_map.begin();
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
        std::map<std::string, JsonNode *>::const_iterator it = m_map.find(path);
        if (it == m_map.end())
            return nullptr;
        else return it->second;
    }

    std::string first_node(path.substr(0,first_dot));
    std::map<std::string, JsonNode *>::const_iterator it = m_map.find(first_node);
    if (it == m_map.end())
        return nullptr;
    JsonNode *child_node = it->second;
    return child_node->nodeAt(path.substr(first_dot+1));
}

JsonNode *ObjectNode::node(const std::string &child_node) const
{
    std::map<std::string, JsonNode *>::const_iterator it = m_map.find(child_node);
    if (it == m_map.end())
        return nullptr;
    return it->second;
}

void ObjectNode::insertNode(const std::string &name, JsonNode *node, bool replace)
{
    std::pair<std::map<std::string, JsonNode *>::iterator, bool>ret;
    ret = m_map.insert(std::pair<std::string, JsonNode *>(name, node));
    if (ret.second == false && replace) {
        delete ret.first->second;
        m_map.erase(ret.first);
        ret = m_map.insert(std::pair<std::string, JsonNode *>(name, node));
        assert(ret.second == true);
    }
}

JsonNode *ObjectNode::take(const std::string &name)
{
    std::map<std::string, JsonNode *>::iterator it = m_map.find(name);
    if (it == m_map.end())
        return nullptr;
    JsonNode *child_node = it->second;
    m_map.erase(it);
    return child_node;
}

StringNode::StringNode(JsonToken *token)
    : JsonNode(String)
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

BooleanNode::BooleanNode(JsonToken *token)
    : JsonNode(Bool)
{
    if (*token->data == 't' || *token->data == 'T')
        m_boolean = true;
    else
        m_boolean = false;
}

NullNode::NullNode()
    : JsonNode(Null)
{ }

ArrayNode::ArrayNode()
    : JsonNode(Array)
{
}

ArrayNode::~ArrayNode()
{
    for (std::vector<JsonNode *>::iterator it = m_vector.begin();
            it != m_vector.end(); it++) {
        delete *it;
    }
}

void ArrayNode::insert(JsonNode *node, int index)
{
    if (index >= m_vector.size()) {
        m_vector.push_back(node);
        return;
    }

    std::vector<JsonNode *>::iterator it = m_vector.begin();
    m_vector.insert(it + index, node);
}

void ArrayNode::append(JsonNode *node)
{
    m_vector.push_back(node);
}

JsonNode *ArrayNode::index(int index)
{
    if (index >= m_vector.size()) {
        return nullptr;
    }
    std::vector<JsonNode *>::iterator it = m_vector.begin();
    return *(it+index);
}

JsonNode *ArrayNode::take(int index)
{
    if (index >= m_vector.size()) {
        return nullptr;
    }

    std::vector<JsonNode *>::iterator it = m_vector.begin();
    JsonNode *return_node = *(it + index);
    m_vector.erase(it+index);
    return return_node;
}

size_t ArrayNode::size()
{
    return m_vector.size();
}

