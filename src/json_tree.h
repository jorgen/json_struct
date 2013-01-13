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

#ifndef JSON_TREE_H
#define JSON_TREE_H

#include "json_tokenizer.h"

#include <string>
#include <vector>
#include <map>

class ObjectNode;
class StringNode;
class NumberNode;
class BooleanNode;
class NullNode;
class ArrayNode;

class JsonNode
{
public:
    enum Type {
        Object,
        String,
        Ascii,
        Number,
        Bool,
        Null,
        Array
    };

    JsonNode(JsonNode::Type type);

    JsonNode::Type type() const
    { return m_type; };

    virtual JsonNode *nodeAt(const std::string &path) const;

    StringNode *stringNodeAt(const std::string &path) const;
    NumberNode *numberNodeAt(const std::string &path) const;
    BooleanNode *booleanNodeAt(const std::string &path) const;
    NullNode *nullNodeAt(const std::string &path) const;
    ArrayNode *arrayNodeAt(const std::string &path) const;
    ObjectNode *objectNodeAt(const std::string &path) const;

    virtual StringNode *asStringNode();
    virtual NumberNode *asNumberNode();
    virtual BooleanNode *asBooleanNode();
    virtual NullNode *asNullNode();
    virtual ArrayNode *asArrayNode();
    virtual ObjectNode *asObjectNode();
protected:
    JsonNode::Type m_type;
};

class ObjectNode : public JsonNode
{
public:
    ObjectNode();
    ~ObjectNode();

    JsonNode *nodeAt(const std::string &path) const;

    JsonNode *node(const std::string &child_node) const;

    void insertNode(const std::string &name, JsonNode *node, bool replace = false);
    JsonNode *take(const std::string &name);

    ObjectNode *asObjectNode();
private:
    std::map<std::string, JsonNode *> m_map;
};

class StringNode : public JsonNode
{
public:
    StringNode(JsonToken *token);

    const std::string &string() const;
    void setString(const std::string &string);

    StringNode *asStringNode();
protected:
    std::string m_string;
};

class NumberNode : public JsonNode
{
public:
    NumberNode(JsonToken *token);

    double number() const
    { return m_number; }

    void setNumber(double number)
    { m_number = number; }

    NumberNode *asNumberNode();
protected:
    double m_number;
};

class BooleanNode : public JsonNode
{
public:
    BooleanNode(JsonToken *token);

    bool toBoolean() const
    { return m_boolean; }

    void setBoolean(bool boolean)
    { m_boolean = boolean; }

    BooleanNode *asBooleanNode();
protected:
    bool m_boolean;
};

class NullNode : public JsonNode
{
public:
    NullNode();

    NullNode *asNullNode();
};

class ArrayNode : public JsonNode
{
public:
    ArrayNode();
    ~ArrayNode();

    void insert(JsonNode *node, int index);
    void append(JsonNode *node);

    JsonNode *index(int index);
    JsonNode *take(int index);

    size_t size();

    ArrayNode *asArrayNode();
private:
    std::vector<JsonNode *> m_vector;
};

#endif //JSON_TREE_H
