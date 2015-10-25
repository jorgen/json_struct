/*
 * Copyright © 2012 Jørgen Lind

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

#include "json_tokenizer.h"

#include <algorithm>
#include <list>
#include <string>

#include <assert.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

namespace JT {

SerializerOptions::SerializerOptions(bool pretty, bool ascii)
    : m_shift_size(4)
    , m_depth(0)
    , m_pretty(pretty)
    , m_ascii(ascii)
    , m_token_delimiter(",")
{
    m_value_delimiter = m_pretty? std::string(" : ") : std::string(":");
    m_postfix = m_pretty? std::string("\n") : std::string("");
}

int SerializerOptions::shiftSize() const { return m_shift_size; }

int SerializerOptions::depth() const { return m_depth; }

bool SerializerOptions::pretty() const { return m_pretty; }
void SerializerOptions::setPretty(bool pretty)
{
    m_pretty = pretty;
    m_postfix = m_pretty? std::string("\n") : std::string("");
    m_value_delimiter = m_pretty? std::string(" : ") : std::string(":");
    setDepth(m_depth);
}

bool SerializerOptions::ascii() const { return m_ascii; }
void SerializerOptions::setAscii(bool ascii)
{
    m_ascii = ascii;
}

void SerializerOptions::skipDelimiter(bool skip)
{
    if (skip)
        m_token_delimiter = "";
    else
        m_token_delimiter = ",";
}

void SerializerOptions::setDepth(int depth)
{
    m_depth = depth;
    m_prefix = m_pretty? std::string(depth * m_shift_size, ' ') : std::string();
}

const std::string &SerializerOptions::prefix() const { return m_prefix; }
const std::string &SerializerOptions::tokenDelimiter() const { return m_token_delimiter; }
const std::string &SerializerOptions::valueDelimiter() const { return m_value_delimiter; }
const std::string &SerializerOptions::postfix() const { return m_postfix; }

bool SerializerBuffer::append(const char *data, size_t size)
{
    if (used + size > this->size)
        return false;

    memcpy(buffer + used, data, size);
    used += size;
    return true;
}

Serializer::Serializer()
{
}

Serializer::Serializer(char *buffer, size_t size)
{
    appendBuffer(buffer,size);
}

void Serializer::appendBuffer(char *buffer, size_t size)
{
    m_all_buffers.push_back({buffer,size,0});
    m_unused_buffers.push_back(&m_all_buffers.back());
}

void Serializer::setOptions(const SerializerOptions &option)
{
    m_option = option;
}

bool Serializer::write(const Token &in_token)
{
    const Token &token =
        m_token_transformer == nullptr? in_token : m_token_transformer(in_token);
    if (!m_token_start) {
        if (token.value_type != Token::ObjectEnd
                && token.value_type != Token::ArrayEnd) {
            if (!write(m_option.tokenDelimiter()))
                return false;
        }
    }

    if (m_first) {
        m_first = false;
    } else {
        if (!write(m_option.postfix()))
            return false;
    }

    if (token.value_type == Token::ObjectEnd
            || token.value_type == Token::ArrayEnd) {
        m_option.setDepth(m_option.depth() - 1);
    }

    if (!write(m_option.prefix()))
        return false;

    if (token.name.size) {
        if (!write(token.name_type, token.name))
            return false;

        if (!write(m_option.valueDelimiter()))
            return false;
    }

    if (!write(token.value_type, token.value))
        return false;

    m_token_start = (token.value_type == Token::ObjectStart
            || token.value_type == Token::ArrayStart);
    if (m_token_start) {
        m_option.setDepth(m_option.depth() + 1);
    }
    return true;
}

void Serializer::registerTokenTransformer(std::function<const Token&(const Token &)> token_transformer)
{
    this->m_token_transformer = token_transformer;
}

void Serializer::addRequestBufferCallback(std::function<void(Serializer *)> callback)
{
    m_request_buffer_callbacks.push_back(callback);
}

const std::list<SerializerBuffer> &Serializer::buffers() const
{
    return m_all_buffers;
}

void Serializer::clearBuffers()
{
    m_all_buffers.clear();
    m_unused_buffers.clear();
}

void Serializer::askForMoreBuffers()
{
    for (auto it = m_request_buffer_callbacks.begin();
            it != m_request_buffer_callbacks.end();
            ++it) {
        (*it)(this);
    }
}

void Serializer::markCurrentSerializerBufferFull()
{
    m_unused_buffers.pop_front();
    if (m_unused_buffers.size() == 0)
        askForMoreBuffers();
}

bool Serializer::writeAsString(const Data &data)
{
    bool written;
    if (*data.data != '"') {
        written = write("\"",1);
        if (!written)
            return false;
    }

    written = write(data.data,data.size);
    if (!written)
        return false;

    if (*data.data != '"') {
        if (!written)
            return false;
        written = write("\"",1);
    }
    return written;
}

bool Serializer::write(Token::Type type, const Data &data)
{
    bool written;
    switch (type) {
        case Token::String:
            written = writeAsString(data);
            break;
        case Token::Ascii:
            if (!m_option.ascii())
                written = writeAsString(data);
            else
                written = write(data.data,data.size);
            break;
        default:
            written = write(data.data,data.size);
            break;
    }
    return written;
}
bool Serializer::write(const char *data, size_t size)
{
    if(!size)
        return true;
    if (m_unused_buffers.size() == 0)
        askForMoreBuffers();
    size_t written = 0;
    while(m_unused_buffers.size() && written < size) {
        SerializerBuffer *first = m_unused_buffers.front();
        size_t free = first->free();
        if (!free) {
            markCurrentSerializerBufferFull();
            continue;
        }
        size_t to_write = std::min(size, free);
        first->append(data + written, to_write);
        written += to_write;
    }
    return written == size;
}

} //namespace
