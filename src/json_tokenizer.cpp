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

#include <list>
#include <string>

#include <assert.h>

#include <stdio.h>
#include <stdint.h>

namespace JT {

static inline void trim_string_chars(Data &data)
{
    if (data.size > 2) {
        data.data++; data.size-=2;
    } else {
        data.data = ""; data.size = 0;
    }
}
static inline void fix_data_for_token(Token &token)
{
    if (token.name_type == Token::String)
        trim_string_chars(token.name);
    if (token.value_type == Token::String)
        trim_string_chars(token.value);
}

static inline void populate_anonymous_token(const Data &data, Token::Type type, Token &token)
{
    token.name = Data("",0,false);
    token.name_type = Token::String;
    token.value = data;
    token.value_type = type;
}

class TypeChecker
{
public:
    static Token::Type type(Token::Type type, const char *data, int length) {
        if (type != Token::Ascii)
            return type;
        if (m_null.compare(0,m_null.size(), data, 0, length) == 0)
            return Token::Null;
        if (m_true.compare(0,m_true.size(), data, 0, length) == 0)
            return Token::Bool;
        if (m_false.compare(0,m_false.size(), data, 0, length) == 0)
            return Token::Bool;
        return Token::Ascii;
    }
private:
    static const std::string m_null;
    static const std::string m_true;
    static const std::string m_false;
};

const std::string TypeChecker::m_null = "null";
const std::string TypeChecker::m_true = "true";
const std::string TypeChecker::m_false = "false";

struct IntermediateToken
{
    IntermediateToken()
    { }

    void clear() {
        intermedia_set = false;
        name_type_set = false;
        data_type_set = false;
        name_type = Token::Error;
        data_type = Token::Error;
        name.clear();
        data.clear();
    }

    bool intermedia_set = false;
    bool name_type_set = false;
    bool data_type_set = false;
    Token::Type name_type = Token::Error;
    Token::Type data_type = Token::Error;
    std::string name;
    std::string data;
};

class TokenizerPrivate
{
public:
    enum InTokenState {
        FindingName,
        FindingDelimiter,
        FindingData,
        FindingTokenEnd
    };

    enum InPropertyState {
        NoStartFound,
        FindingEnd,
        FoundEnd
    };

    TokenizerPrivate()
    {
    }

    ~TokenizerPrivate()
    {
    }

    void resetForNewToken()
    {
        intermediate_token.clear();
        resetForNewValue();
    }

    void resetForNewValue()
    {
        property_state = NoStartFound;
        property_type = Token::Error;
        current_data_start = 0;
    }

    Error findStringEnd(const Data &json_data, size_t *chars_ahead)
    {
        for (size_t end = cursor_index; end < json_data.size; end++) {
            if (is_escaped) {
                is_escaped = false;
                continue;
            }
            switch (*(json_data.data + end)) {
                case '\\':
                    is_escaped = true;
                    break;
                case '"':
                    *chars_ahead = end + 1 - cursor_index;
                    return Error::NoError;

                default:
                    break;
            }
        }
        return Error::NeedMoreData;
    }

    Error findAsciiEnd(const Data &json_data, size_t *chars_ahead)
    {
        assert(property_type == Token::Ascii);
        for (size_t end = cursor_index; end < json_data.size; end++) {
            char ascii_code = *(json_data.data + end);
            if ((ascii_code >= 'A' && ascii_code <= 'Z') ||
                    (ascii_code >= '^' && ascii_code <= 'z') ||
                    (ascii_code >= '0' && ascii_code <= '9')) {
                continue;
            } else if (ascii_code == '\0') {
                *chars_ahead = end - cursor_index;
                return Error::NeedMoreData;
            } else {
                *chars_ahead = end - cursor_index;
                return Error::NoError;
            }
        }
        return Error::NeedMoreData;
    }

    Error findNumberEnd(const Data &json_data, size_t *chars_ahead)
    {
        for (size_t end = cursor_index; end < json_data.size; end++) {
            char number_code = *(json_data.data + end);
            if ((number_code >= '0' && number_code <= '9'))
                continue;
            switch(number_code) {
                case '.':
                case '+':
                case '-':
                case 'e':
                case 'E':
                    continue;
                default:
                    *chars_ahead = end - cursor_index;
                    return Error::NoError;
            }
        }
        return Error::NeedMoreData;
    }

    Error findStartOfNextValue(Token::Type *type,
            const Data &json_data,
            size_t *chars_ahead)
    {

        assert(property_state == NoStartFound);

        for (size_t current_pos  = cursor_index; current_pos < json_data.size; current_pos++) {
            switch (*(json_data.data + current_pos)) {
                case ' ':
                case '\n':
                case '\0':
                    break;
                case '"':
                    *type = Token::String;
                    *chars_ahead = current_pos - cursor_index;
                    return Error::NoError;
                case '{':
                    *type = Token::ObjectStart;
                    *chars_ahead = current_pos - cursor_index;
                    return Error::NoError;
                case '}':
                    *type = Token::ObjectEnd;
                    *chars_ahead = current_pos - cursor_index;
                    return Error::NoError;
                case '[':
                    *type = Token::ArrayStart;
                    *chars_ahead = current_pos - cursor_index;
                    return Error::NoError;
                case ']':
                    *type = Token::ArrayEnd;
                    *chars_ahead = current_pos - cursor_index;
                    return Error::NoError;
                case '-':
                case '+':
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    *type = Token::Number;
                    *chars_ahead = current_pos - cursor_index;
                    return Error::NoError;
                default:
                    char ascii_code = *(json_data.data + current_pos);
                    if ((ascii_code >= 'A' && ascii_code <= 'Z') ||
                            (ascii_code >= '^' && ascii_code <= 'z')) {
                        *type = Token::Ascii;
                        *chars_ahead = current_pos - cursor_index;;
                        return Error::NoError;
                    } else {
                        *chars_ahead = current_pos - cursor_index;
                        return Error::EncounteredIlligalChar;
                    }
                    break;
            }

        }
        return Error::NeedMoreData;
    }

    Error findDelimiter(const Data &json_data, size_t *chars_ahead)
    {
        for (size_t end = cursor_index; end < json_data.size; end++) {
            switch(*(json_data.data + end)) {
                case ':':
                    token_state = FindingData;
                    *chars_ahead = end + 1 - cursor_index;
                    return Error::NoError;
                case ',':
                    token_state = FindingName;
                    *chars_ahead = end + 1 - cursor_index;
                    return Error::NoError;
                case ']':
                    token_state = FindingName;
                    *chars_ahead = end - cursor_index;
                    return Error::NoError;
                case ' ':
                case '\n':
                    break;
                default:
                    return Error::ExpectedDelimiter;
                    break;
            }
        }
        return Error::NeedMoreData;
    }

    Error findTokenEnd(const Data &json_data, size_t *chars_ahead)
    {
        for (size_t end = cursor_index; end < json_data.size; end++) {
            switch(*(json_data.data + end)) {
                case ',':
                    expecting_prop_or_annonymous_data = true;
                    *chars_ahead = end + 1 - cursor_index;
                    return Error::NoError;
                case '\n':
                    if (allow_new_lines) {
                        *chars_ahead = end + 1 - cursor_index;
                        return Error::NoError;
                    }
                    break;
                case ']':
                case '}':
                    *chars_ahead = end - cursor_index;
                    return Error::NoError;
                case ' ':
                case '\0':
                    break;
                default:
                    *chars_ahead = end + 1 - cursor_index;
                    return Error::InvalidToken;
            }
        }
        return Error::NeedMoreData;
    }

    void releaseFirstData()
    {
        const Data &json_data = data_list.front();
        for (auto it = release_callback_list.begin(); it != release_callback_list.end(); ++it){
            (*it)(json_data.data);
        }
        data_list.pop_front();
        cursor_index = 0;
        current_data_start = 0;
    }

    Error populateFromData(Data &data, Token::Type *type, const Data &json_data)
    {
        size_t diff = 0;
        Error error = Error::NoError;
        data.size = 0;
        data.data = json_data.data + cursor_index;
        data.temporary = json_data.temporary;
        if (property_state == NoStartFound) {
            Error error = findStartOfNextValue(type, json_data, &diff);
            if (error != Error::NoError) {
                *type = Token::Error;
                return error;
            }

            data.data = json_data.data + cursor_index + diff;
            current_data_start = cursor_index + diff;
            cursor_index += diff + 1;
            property_type = *type;


            if (*type == Token::ObjectStart || *type == Token::ObjectEnd
                    || *type == Token::ArrayStart || *type == Token::ArrayEnd) {
                data.size = 1;
                property_state = FoundEnd;
            } else {
                property_state = FindingEnd;
            }
        }

        if (property_state == FindingEnd) {
            switch (*type) {
            case Token::String:
                error = findStringEnd(json_data, &diff);
                break;
            case Token::Ascii:
                error = findAsciiEnd(json_data, &diff);
                break;
            case Token::Number:
                error = findNumberEnd(json_data, &diff);
                break;
            default:
                return Error::InvalidToken;
            }

            if (error != Error::NoError) {
                return error;
            }

            cursor_index += diff;
            data.size = cursor_index - current_data_start;
            property_state = FoundEnd;
        }

        return Error::NoError;
    }

    Error populateNextTokenFromData(Token *next_token, const Data &json_data)
    {
        Token tmp_token;
        while (cursor_index < json_data.size) {
            size_t diff = 0;
            Data data;
            Token::Type type;
            Error error;
            switch (token_state) {
                case FindingName:
                    type = intermediate_token.name_type;
                    error = populateFromData(data, &type, json_data);
                    if (error == Error::NeedMoreData) {
                        if (property_state > NoStartFound) {
                            intermediate_token.intermedia_set = true;
                            size_t to_null = strnlen(data.data , json_data.size - current_data_start);
                            intermediate_token.name.append(data.data , to_null);
                            if (!intermediate_token.name_type_set) {
                                intermediate_token.name_type = type;
                                intermediate_token.name_type_set = true;
                            }
                        }
                        return error;
                    } else if (error != Error::NoError) {
                        return error;
                    }

                    if (intermediate_token.intermedia_set) {
                        intermediate_token.name.append(data.data, data.size);
                        data.size = intermediate_token.name.length();
                        data.data  = intermediate_token.name.c_str();
                        data.temporary = true;
                        type = intermediate_token.name_type;
                    }

                    if (type == Token::ObjectEnd || type == Token::ArrayEnd
                            || type == Token::ArrayStart || type == Token::ObjectStart) {
                        switch (type) {
                            case Token::ObjectEnd:
                            case Token::ArrayEnd:
                                if (expecting_prop_or_annonymous_data) {
                                    return Error::ExpectedDataToken;
                                }
                                populate_anonymous_token(data,type,*next_token);
                                token_state = FindingTokenEnd;
                                return Error::NoError;

                            case Token::ObjectStart:
                            case Token::ArrayStart:
                                populate_anonymous_token(data,type,*next_token);
                                expecting_prop_or_annonymous_data = false;
                                token_state = FindingName;
                                return Error::NoError;
                            default:
                                return Error::UnknownError;
                        }
                    } else {
                        tmp_token.name = data;
                    }

                    tmp_token.name_type = TypeChecker::type(type, tmp_token.name.data,
                                                            tmp_token.name.size);
                    token_state = FindingDelimiter;
                    resetForNewValue();
                    break;

                case FindingDelimiter:
                    error = findDelimiter(json_data, &diff);
                    if (error != Error::NoError) {
                        if (intermediate_token.intermedia_set == false) {
                            intermediate_token.name.append(tmp_token.name.data, tmp_token.name.size);
                            intermediate_token.name_type = tmp_token.name_type;
                            intermediate_token.intermedia_set = true;
                        }
                        return Error::NeedMoreData;
                    }
                    cursor_index += diff;
                    resetForNewValue();
                    expecting_prop_or_annonymous_data = false;
                    if (token_state == FindingName) {
                        fix_data_for_token(tmp_token);
                        populate_anonymous_token(tmp_token.name, tmp_token.name_type, *next_token);
                        return Error::NoError;
                    } else {
                        if (tmp_token.name_type != Token::String) {
                            if (!allow_ascii_properties || tmp_token.name_type != Token::Ascii) {
                                return Error::IlligalPropertyName;
                            }
                        }
                    }
                    break;

                case FindingData:
                    type = intermediate_token.data_type;
                    error = populateFromData(data, &type, json_data);
                    if (error == Error::NeedMoreData) {
                        if (intermediate_token.intermedia_set == false) {
                            intermediate_token.name.append(tmp_token.name.data, tmp_token.name.size);
                            intermediate_token.name_type = tmp_token.name_type;
                            intermediate_token.intermedia_set = true;
                        }
                        if (property_state > NoStartFound) {
                            size_t data_length = strnlen(data.data , json_data.size - current_data_start);
                            intermediate_token.data.append(data.data, data_length);
                            if (!intermediate_token.data_type_set) {
                                intermediate_token.data_type = type;
                                intermediate_token.data_type_set = true;
                            }
                        }
                        return error;
                    } else if (error != Error::NoError) {
                        return error;
                    }

                    if (intermediate_token.intermedia_set) {
                        intermediate_token.data.append(data.data, data.size);
                        if (!intermediate_token.data_type_set) {
                            intermediate_token.data_type = type;
                            intermediate_token.data_type_set = true;
                        }
                        tmp_token.name.data = intermediate_token.name.c_str();
                        tmp_token.name.size = intermediate_token.name.size();
                        tmp_token.name_type = intermediate_token.name_type;
                        tmp_token.name.temporary = true;
                        data.data = intermediate_token.data.c_str();
                        data.size = intermediate_token.data.length();
                        data.temporary = true;
                        type = intermediate_token.data_type;
                    }

                    tmp_token.value = data;
                    tmp_token.value_type = TypeChecker::type(type, tmp_token.value.data, tmp_token.value.size);

                    if (tmp_token.value_type  == Token::Ascii && !allow_ascii_properties)
                        return Error::IlligalDataValue;

                    if (type == Token::ObjectStart || type == Token::ArrayStart) {
                        token_state = FindingName;
                    } else {
                        token_state = FindingTokenEnd;
                    }
                    fix_data_for_token(tmp_token);
                    *next_token = tmp_token;
                    return Error::NoError;
                case FindingTokenEnd:
                    error = findTokenEnd(json_data, &diff);
                    if (error != Error::NoError) {
                        return error;
                    }
                    cursor_index += diff;
                    token_state = FindingName;
                    break;
            }
        }
        return Error::NeedMoreData;
    }

    std::list<Data> data_list;
    std::list<std::function<void(const char *)>> release_callback_list;
    size_t cursor_index = 0;
    size_t current_data_start = 0;
    InTokenState token_state = FindingName;
    InPropertyState property_state = NoStartFound;
    Token::Type property_type = Token::Error;
    bool is_escaped = false;
    bool allow_ascii_properties = false;
    bool allow_new_lines = false;
    bool expecting_prop_or_annonymous_data = false;
    bool continue_after_need_more_data = false;
    IntermediateToken intermediate_token;
};

Token::Token()
    : name_type(String)
    , name("", 0, false)
    , value_type(String)
    , value("", 0, false)
{

}

Tokenizer::Tokenizer()
    : m_private(new TokenizerPrivate())
{
}

Tokenizer::Tokenizer(const Tokenizer &other)
    : m_private(new TokenizerPrivate(*other.m_private))
{
}

Tokenizer::Tokenizer(Tokenizer &&other)
    : m_private(other.m_private)
{
    other.m_private = 0;
}

Tokenizer::~Tokenizer()
{
    delete m_private;
}

Tokenizer &Tokenizer::operator=(const Tokenizer &rhs)
{
    *m_private = *rhs.m_private;
    return *this;
}

Tokenizer &Tokenizer::operator=(Tokenizer &&rhs)
{
    m_private = rhs.m_private;
    rhs.m_private = 0;
    return *this;
}

void Tokenizer::allowAsciiType(bool allow)
{
    m_private->allow_ascii_properties = allow;
}

void Tokenizer::allowNewLineAsTokenDelimiter(bool allow)
{
    m_private->allow_new_lines = allow;
}

void Tokenizer::addData(const char *data, size_t data_size, bool temporary)
{
    m_private->data_list.push_back(Data(data, data_size, temporary));
}

size_t Tokenizer::registered_buffers() const
{
    return m_private->data_list.size();
}

void Tokenizer::registerRelaseCallback(std::function<void(const char *)> function)
{
    m_private->release_callback_list.push_back(function);
}

Error Tokenizer::nextToken(Token *next_token)
{
    if (!m_private->data_list.size()) {
        return Error::NeedMoreData;
    }

    if (!m_private->continue_after_need_more_data)
        m_private->resetForNewToken();

    Error error = Error::NeedMoreData;
    while (error == Error::NeedMoreData && m_private->data_list.size()) {
        const Data &json_data = m_private->data_list.front();
        error = m_private->populateNextTokenFromData(next_token, json_data);

        if (error != Error::NoError) {
            m_private->releaseFirstData();
        }
    }

    m_private->continue_after_need_more_data = error == Error::NeedMoreData;

    return error;
}

PrinterOption::PrinterOption(bool pretty, bool ascii_name)
    : m_shift_size(4)
    , m_pretty(pretty)
    , m_ascii_name(ascii_name)
{ }

bool PrintBuffer::append(const char *data, size_t size)
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

void Serializer::markCurrentPrintBufferFull()
{
    m_unused_buffers.pop_front();
    if (m_unused_buffers.size() == 0) {
    }
}

bool Serializer::canCurrentBufferFit(size_t amount)
{
    return m_unused_buffers.front()->canFit(amount);
}

bool Serializer::write(const char *data, size_t size)
{
    while(m_unused_buffers.size() && !canCurrentBufferFit(size)) {
        markCurrentPrintBufferFull();
    }
    if (!m_unused_buffers.size()) {
        for (auto it = m_request_buffer_callbacks.begin(); it != m_request_buffer_callbacks.end(); ++it) {
            //Ask for new buffer with atleast size
            (*it)(this,size);
        }
    }
    if (!canCurrentBufferFit(size))
        return false;
    m_unused_buffers.front()->append(data,size);
    return true;
}

void Serializer::addRequestBufferCallback(std::function<void(Serializer *, size_t)> callback)
{
    m_request_buffer_callbacks.push_back(callback);
}

const std::list<PrintBuffer> &Serializer::printBuffers() const
{
    return m_all_buffers;
}

} //namespace
