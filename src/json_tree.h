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
#include <list>

namespace JT {

class Node;
class ObjectNode;
class StringNode;
class NumberNode;
class BooleanNode;
class NullNode;
class ArrayNode;

class TreeBuilder
{
public:
    std::pair<Node *, Error> build(const char *data, size_t data_size) const;
    std::pair<Node *, Error> build(Tokenizer *tokenizer) const;
    std::pair<Node *, Error> build(Token *token, Tokenizer *tokenizer) const;

    std::pair<Node *, Error> createNode(Token *token, Tokenizer *tokenizer) const;

    bool create_root_if_needed = false;
};

class TreeSerializer : public Serializer
{
public:
    TreeSerializer();
    TreeSerializer(char *buffer, size_t size);

    bool serialize(ObjectNode *rootObject);
    bool serialize(ArrayNode *rootArray);

    bool serializeNode(ObjectNode *objectNode);
    bool serializeNode(ArrayNode *arrayNode);
};

class Node
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

    Node(Node::Type type, const Data &data);
    virtual ~Node();

    Node::Type type() const
    { return m_type; };

    Data data() const;

    virtual Node *nodeAt(const std::string &path) const;

    StringNode *stringNodeAt(const std::string &path) const;
    NumberNode *numberNodeAt(const std::string &path) const;
    BooleanNode *booleanNodeAt(const std::string &path) const;
    NullNode *nullNodeAt(const std::string &path) const;
    ArrayNode *arrayNodeAt(const std::string &path) const;
    ObjectNode *objectNodeAt(const std::string &path) const;

    StringNode *asStringNode();
    const StringNode *asStringNode() const;
    NumberNode *asNumberNode();
    const NumberNode *asNumberNode() const;
    BooleanNode *asBooleanNode();
    const BooleanNode *asBooleanNode() const;
    NullNode *asNullNode();
    const NullNode *asNullNode() const;
    ArrayNode *asArrayNode();
    const ArrayNode *asArrayNode() const;
    ObjectNode *asObjectNode();
    const ObjectNode *asObjectNode() const;
protected:
    Node::Type m_type;
    bool m_delete_data_buffer;
    Data m_data;
};

class Property
{
public:
    Property(Token::Type type, const Data data);
    Property(const Property &other);
    Property(Property &&other);
    ~Property();

    Token::Type type() const;
    bool comparePropertyData(const Property &property) const;
    bool comparePropertyAscii(const Property &property) const;
    bool comparePropertyAscii(const std::string &property_name) const;

    Data data() const;

    Property &operator= (const Property &other);
private:
    Token::Type m_type;
    bool m_delete_data_buffer;
    Data m_data;
};

class ObjectNode : public Node
{
public:
    ObjectNode();
    ~ObjectNode();

    Node *nodeAt(const std::string &path) const;

    Node *node(const std::string &child_node) const;

    void insertNode(const Property &property, Node *node, bool replace = false);
    Node *take(const std::string &name);

    Error fill(Tokenizer *tokenizer, const TreeBuilder &builder);

    class Iterator {
    public:
        Iterator( std::vector<std::pair<Property, Node *>>::const_iterator it);
        void fillToken(Token *token) const;

        const std::pair<Property, Node *> &operator*() const;
        Iterator &operator++();
        Iterator operator++(int);
        Iterator &operator--();
        Iterator operator--(int);
        bool operator==(const Iterator &other) const;
        bool operator!=(const Iterator &other) const;

    private:
        std::vector<std::pair<Property, Node *>>::const_iterator m_it;
    };
    Iterator begin() const;
    Iterator end() const;

    void fillStartToken(Token *token);
    void fillEndToken(Token *token);
private:
    Node *findNode(const std::string name) const;
    std::vector<std::pair<Property, Node *>> m_data;
};

class StringNode : public Node
{
public:
    StringNode(Token *token);

    const std::string &string() const;
    void setString(const std::string &string);

protected:
    std::string m_string;
};

class NumberNode : public Node
{
public:
    NumberNode(Token *token);

    double number() const
    { return m_number; }

    void setNumber(double number)
    { m_number = number; }

protected:
    double m_number;
};

class BooleanNode : public Node
{
public:
    BooleanNode(Token *token);

    bool boolean() const
    { return m_boolean; }

    void setBoolean(bool boolean)
    { m_boolean = boolean; }

protected:
    bool m_boolean;
};

class NullNode : public Node
{
public:
    NullNode(Token *token);

};

class ArrayNode : public Node
{
public:
    ArrayNode();
    ~ArrayNode();

    void insert(Node *node, size_t index);
    void append(Node *node);

    Node *index(size_t index);
    Node *take(size_t index);

    size_t size();

    void fillToken(size_t index, Token *token) const;
    void fillStartToken(Token *token);
    void fillEndToken(Token *token);

    Error fill(Tokenizer *tokenizer, const TreeBuilder &builder);
private:
    std::vector<Node *> m_vector;
};

} // Namespace

#endif //JSON_TREE_H
