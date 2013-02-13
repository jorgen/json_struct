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

class JsonNode;
class ObjectNode;
class StringNode;
class NumberNode;
class BooleanNode;
class NullNode;
class ArrayNode;
class JsonPrinter;

class JsonNodeError
{
public:
    JsonNodeError()
        : error(JsonError::NoError)
        , errorNode(nullptr)
    {}
    JsonNodeError(JsonError error, JsonNode *errorNode = nullptr)
        : error(error)
        , errorNode(errorNode)
    { }
    JsonError error;
    JsonNode *errorNode;
};

class JsonPrinterOption
{
public:
    JsonPrinterOption(bool pretty = false, bool ascii_name = false)
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

class JsonOutBuffer
{
public:
    bool canFit(size_t amount) const { return size - end >= amount; }
    bool append(const char *data, size_t size);
    char *buffer;
    size_t size;
    size_t end;
};


class JsonOutBufferHandler
{
public:
    JsonOutBufferHandler(char *buffer, size_t size);

    void appendBuffer(char *buffer, size_t size);

    const JsonOutBuffer &currentPrintBuffer() const { return m_buffers.front(); }
    JsonOutBuffer &currentPrintBuffer() { return m_buffers.front(); }
    bool canFit(size_t amount);
    bool write(const char *data, size_t size);
    void markCurrentPrintBufferFull();

    const JsonOutBuffer &firstFinishedBuffer() const;

    size_t bufferSize() const { return m_buffers.size(); }

private:
    std::list<std::function<void(JsonOutBufferHandler *)>> m_request_buffer_callbacks;
    std::list<JsonOutBuffer> m_buffers;
    std::list<JsonOutBuffer> m_finished_buffers;
};

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
    virtual ~JsonNode();

    JsonNode::Type type() const
    { return m_type; };

    virtual JsonNode *nodeAt(const std::string &path) const;

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

    static std::pair<JsonNode *, JsonNodeError> create(JsonToken *from_token,
                                                       JsonTokenizer *tokenizer,
                                                       JsonNode *continue_from = nullptr);
    static std::pair<JsonNode *, JsonNodeError> create(JsonTokenizer *tokenizer,
                                                       JsonNode *continue_from = nullptr);

    virtual size_t printSize(const JsonPrinterOption &option, int depth = 0) = 0;
    virtual bool print(JsonOutBufferHandler &buffers, const JsonPrinterOption &option , int depth = 0) = 0;
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

    JsonNodeError fill(JsonTokenizer *tokenizer, JsonNode *continue_from = nullptr);

    size_t printSize(const JsonPrinterOption &option, int depth);
    bool print(JsonOutBufferHandler &buffers, const JsonPrinterOption &option , int depth = 0);
private:
    std::map<std::string, JsonNode *> m_map;
};

class StringNode : public JsonNode
{
public:
    StringNode(JsonToken *token);

    const std::string &string() const;
    void setString(const std::string &string);

    size_t printSize(const JsonPrinterOption &option, int depth);
    bool print(JsonOutBufferHandler &buffers, const JsonPrinterOption &option , int depth = 0);
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

    size_t printSize(const JsonPrinterOption &option, int depth);
    bool print(JsonOutBufferHandler &buffers, const JsonPrinterOption &option , int depth = 0);
protected:
    double m_number;
};

class BooleanNode : public JsonNode
{
public:
    BooleanNode(JsonToken *token);

    bool boolean() const
    { return m_boolean; }

    void setBoolean(bool boolean)
    { m_boolean = boolean; }

    size_t printSize(const JsonPrinterOption &option, int depth);
    bool print(JsonOutBufferHandler &buffers, const JsonPrinterOption &option , int depth = 0);
protected:
    bool m_boolean;
};

class NullNode : public JsonNode
{
public:
    NullNode(JsonToken *token);

    size_t printSize(const JsonPrinterOption &option, int depth);
    bool print(JsonOutBufferHandler &buffers, const JsonPrinterOption &option , int depth = 0);
};

class ArrayNode : public JsonNode
{
public:
    ArrayNode();
    ~ArrayNode();

    void insert(JsonNode *node, size_t index);
    void append(JsonNode *node);

    JsonNode *index(size_t index);
    JsonNode *take(size_t index);

    size_t size();

    JsonNodeError fill(JsonTokenizer *tokenizer, JsonNode *continue_from = nullptr);

    size_t printSize(const JsonPrinterOption &option, int depth);
    bool print(JsonOutBufferHandler &buffers, const JsonPrinterOption &option , int depth = 0);
private:
    std::vector<JsonNode *> m_vector;
};

#endif //JSON_TREE_H
