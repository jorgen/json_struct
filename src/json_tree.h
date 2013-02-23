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
class Printer;

class PrinterOption
{
public:
    PrinterOption(bool pretty = false, bool ascii_name = false)
        : m_shift_size(4)
        , m_pretty(pretty)
        , m_ascii_name(ascii_name)
    { }

    short shiftSize() const { return m_shift_size; }
    bool pretty() const { return m_pretty; }
    bool ascii_name() const { return m_ascii_name; }

private:
    short m_shift_size;
    bool m_pretty;
    bool m_ascii_name;
};

class PrintBuffer
{
public:
    bool canFit(size_t amount) const { return size - end >= amount; }
    bool append(const char *data, size_t size);
    char *buffer;
    size_t size;
    size_t end;
};

class PrintHandler
{
public:
    PrintHandler(char *buffer, size_t size);

    void appendBuffer(char *buffer, size_t size);

    const PrintBuffer &currentPrintBuffer() const { return m_buffers.front(); }
    PrintBuffer &currentPrintBuffer() { return m_buffers.front(); }
    bool canFit(size_t amount);
    bool write(const char *data, size_t size);
    void markCurrentPrintBufferFull();

    const PrintBuffer &firstFinishedBuffer() const;

private:
    std::list<std::function<void(PrintHandler *)>> m_request_buffer_callbacks;
    std::list<PrintBuffer> m_buffers;
    std::list<PrintBuffer> m_finished_buffers;
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

    Node(Node::Type type);
    virtual ~Node();

    Node::Type type() const
    { return m_type; };

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

    static std::pair<Node *, Error> create(Token *from_token,
                                                       Tokenizer *tokenizer);
    static std::pair<Node *, Error> create(Tokenizer *tokenizer);

    virtual size_t printSize(const PrinterOption &option, int depth = 0) = 0;
    virtual bool print(PrintHandler &buffers, const PrinterOption &option , int depth = 0) = 0;
protected:
    Node::Type m_type;
};

class ObjectNode : public Node
{
public:
    ObjectNode();
    ~ObjectNode();

    Node *nodeAt(const std::string &path) const;

    Node *node(const std::string &child_node) const;

    void insertNode(const std::string &name, Node *node, bool replace = false);
    Node *take(const std::string &name);

    Error fill(Tokenizer *tokenizer);

    size_t printSize(const PrinterOption &option, int depth);
    bool print(PrintHandler &buffers, const PrinterOption &option , int depth = 0);
private:
    std::map<std::string, Node *> m_map;
};

class StringNode : public Node
{
public:
    StringNode(Token *token);

    const std::string &string() const;
    void setString(const std::string &string);

    size_t printSize(const PrinterOption &option, int depth);
    bool print(PrintHandler &buffers, const PrinterOption &option , int depth = 0);
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

    size_t printSize(const PrinterOption &option, int depth);
    bool print(PrintHandler &buffers, const PrinterOption &option , int depth = 0);
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

    size_t printSize(const PrinterOption &option, int depth);
    bool print(PrintHandler &buffers, const PrinterOption &option , int depth = 0);
protected:
    bool m_boolean;
};

class NullNode : public Node
{
public:
    NullNode(Token *token);

    size_t printSize(const PrinterOption &option, int depth);
    bool print(PrintHandler &buffers, const PrinterOption &option , int depth = 0);
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

    Error fill(Tokenizer *tokenizer);

    size_t printSize(const PrinterOption &option, int depth);
    bool print(PrintHandler &buffers, const PrinterOption &option , int depth = 0);
private:
    std::vector<Node *> m_vector;
};

} // Namespace

#endif //JSON_TREE_H
