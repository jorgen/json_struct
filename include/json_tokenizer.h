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

#ifndef JSON_TOKENIZER_H
#define JSON_TOKENIZER_H

#include <stddef.h>
#include <functional>
#include <list>
#include <string>

#include <assert.h>

namespace JT {

struct Data
{
    Data()
        : data("")
        , size(0)
    {}

    Data(const char *data, size_t size)
        : data(data)
        , size(size)
    {}

    template <size_t N>
    static Data asData(const char (&data)[N])
    {
        return Data(data, N - 1);
    }

    static Data asData(const std::string &str)
    {
        return Data(str.c_str(), str.size());
    }

    const char *data;
    size_t size;
};

struct Token
{
    enum Type {
        Error,
        String,
        Ascii,
        Number,
        ObjectStart,
        ObjectEnd,
        ArrayStart,
        ArrayEnd,
        Bool,
        Null
    };

    Token();

    Type name_type;
    Data name;
    Type value_type;
    Data value;
};

struct IntermediateToken
{
    IntermediateToken()
    { }

    void clear() {
        active = false;
        name_type_set = false;
        data_type_set = false;
        name_type = Token::Error;
        data_type = Token::Error;
        name.clear();
        data.clear();
    }

    bool active = false;
    bool name_type_set = false;
    bool data_type_set = false;
    Token::Type name_type = Token::Error;
    Token::Type data_type = Token::Error;
    std::string name;
    std::string data;
};

class TypeChecker
{
public:
    TypeChecker()
        : m_null("null")
        , m_true("true")
        , m_false("false")
    {}

    Token::Type type(Token::Type type, const char *data, size_t length) {
        if (type != Token::Ascii)
            return type;
        if (m_null.size() == length) {
            if (strncmp(m_null.c_str(),data,length) == 0) {
                return Token::Null;
            } else if (strncmp(m_true.c_str(),data,length) == 0) {
                return Token::Bool;
            }
        }
        if (m_false.size() == length) {
            if (strncmp(m_false.c_str(),data,length) == 0)
                return Token::Bool;
        }
        return Token::Ascii;
    }
private:
    const std::string m_null;
    const std::string m_true;
    const std::string m_false;
};

enum class Error {
        NoError,
        NeedMoreData,
        InvalidToken,
        ExpectedPropertyName,
        ExpectedDelimiter,
        ExpectedDataToken,
        IlligalPropertyName,
        IlligalPropertyType,
        IlligalDataValue,
        EncounteredIlligalChar,
        CouldNotCreateNode,
        NodeNotFound,
        MissingPropertyName,
        UnknownError
};

class Tokenizer
{
public:
    Tokenizer();
    ~Tokenizer();

    void allowAsciiType(bool allow);
    void allowNewLineAsTokenDelimiter(bool allow);
    void allowSuperfluousComma(bool allow);

    void addData(const char *data, size_t size);
    size_t registered_buffers() const;
    void registerRelaseCallback(std::function<void(const char *)> callback);

    Error nextToken(Token &next_token);
    void registerTokenTransformer(std::function<void(Token &next_token)> token_transformer);

private:
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

    void resetForNewToken();
    void resetForNewValue();
    Error findStringEnd(const Data &json_data, size_t *chars_ahead);
    Error findAsciiEnd(const Data &json_data, size_t *chars_ahead);
    Error findNumberEnd(const Data &json_data, size_t *chars_ahead);
    Error findStartOfNextValue(Token::Type *type,
                               const Data &json_data,
                               size_t *chars_ahead);
    Error findDelimiter(const Data &json_data, size_t *chars_ahead);
    Error findTokenEnd(const Data &json_data, size_t *chars_ahead);
    void releaseFirstData();
    Error populateFromData(Data &data, Token::Type *type, const Data &json_data);
    static void populate_annonymous_token(const Data &data, Token::Type type, Token &token);
    Error populateNextTokenFromData(Token &next_token, const Data &json_data);

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
    bool allow_superfluous_comma = false;
    bool expecting_prop_or_annonymous_data = false;
    bool continue_after_need_more_data = false;
    IntermediateToken intermediate_token;
    std::function<void(Token &next_token)> token_transformer;
    TypeChecker type_checker;
};

class SerializerOptions
{
public:
    SerializerOptions(bool pretty = false, bool ascii = false);

    int shiftSize() const;

    bool pretty() const;
    void setPretty(bool pretty);

    int depth() const;
    void setDepth(int depth);

    bool ascii() const;
    void setAscii(bool ascii);

    void skipDelimiter(bool skip);

    const std::string &prefix() const;
    const std::string &tokenDelimiter() const;
    const std::string &valueDelimiter() const;
    const std::string &postfix() const;

private:
    int m_shift_size;
    int m_depth;
    bool m_pretty;
    bool m_ascii;

    std::string m_prefix;
    std::string m_token_delimiter;
    std::string m_value_delimiter;
    std::string m_postfix;
};

class SerializerBuffer
{
public:
    bool free() const { return size - used; }
    bool append(const char *data, size_t size);
    char *buffer;
    size_t size;
    size_t used;
};

class Serializer
{
public:
    Serializer();
    Serializer(char *buffer, size_t size);

    void appendBuffer(char *buffer, size_t size);
    void setOptions(const SerializerOptions &option);
    SerializerOptions options() const { return m_option; }

    bool write(const Token &token);
    void registerTokenTransformer(std::function<const Token&(const Token &)> token_transformer);

    void addRequestBufferCallback(std::function<void(Serializer *)> callback);
    const std::list<SerializerBuffer> &buffers() const;
    void clearBuffers();
private:
    void askForMoreBuffers();
    void markCurrentSerializerBufferFull();
    bool writeAsString(const Data &data);
    bool write(Token::Type type, const Data &data);
    bool write(const char *data, size_t size);
    bool write(const std::string &str) { return write(str.c_str(), str.size()); }

    std::list<std::function<void(Serializer *)>> m_request_buffer_callbacks;
    std::list<SerializerBuffer *> m_unused_buffers;
    std::list<SerializerBuffer> m_all_buffers;

    SerializerOptions m_option;
    bool m_first = true;
    bool m_token_start = true;
    std::function<const Token&(const Token &)> m_token_transformer;
};

// IMPLEMENTATION

inline Token::Token()
    : name_type(String)
    , name()
    , value_type(String)
    , value()
{

}

inline Tokenizer::Tokenizer()
{}
inline Tokenizer::~Tokenizer()
{}

inline void Tokenizer::allowAsciiType(bool allow)
{
    allow_ascii_properties = allow;
}

inline void Tokenizer::allowNewLineAsTokenDelimiter(bool allow)
{
    allow_new_lines = allow;
}

inline void Tokenizer::allowSuperfluousComma(bool allow)
{
    allow_superfluous_comma = allow;
}
inline void Tokenizer::addData(const char *data, size_t data_size)
{
    data_list.push_back(Data(data, data_size));
}

inline size_t Tokenizer::registered_buffers() const
{
    return data_list.size();
}

inline void Tokenizer::registerRelaseCallback(std::function<void(const char *)> function)
{
    release_callback_list.push_back(function);
}

inline Error Tokenizer::nextToken(Token &next_token)
{
    if (data_list.empty()) {
        releaseFirstData();
    }

    if (data_list.empty()) {
        return Error::NeedMoreData;
    }

    if (!continue_after_need_more_data)
        resetForNewToken();

    Error error = Error::NeedMoreData;
    while (error == Error::NeedMoreData && data_list.size()) {
        const Data &json_data = data_list.front();
        error = populateNextTokenFromData(next_token, json_data);

        if (error != Error::NoError) {
            releaseFirstData();
        }

    }

    continue_after_need_more_data = error == Error::NeedMoreData;

    if (error == Error::NoError && token_transformer != nullptr) {
        token_transformer(next_token);
    }

    return error;
}

inline void Tokenizer::registerTokenTransformer(std::function<void(Token &next_token)> token_transformer)
{
    token_transformer = token_transformer;
}

inline void Tokenizer::resetForNewToken()
{
    intermediate_token.clear();
    resetForNewValue();
}

inline void Tokenizer::resetForNewValue()
{
    property_state = NoStartFound;
    property_type = Token::Error;
    current_data_start = 0;
}

inline Error Tokenizer::findStringEnd(const Data &json_data, size_t *chars_ahead)
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

inline Error Tokenizer::findAsciiEnd(const Data &json_data, size_t *chars_ahead)
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

inline Error Tokenizer::findNumberEnd(const Data &json_data, size_t *chars_ahead)
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

inline Error Tokenizer::findStartOfNextValue(Token::Type *type,
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

inline Error Tokenizer::findDelimiter(const Data &json_data, size_t *chars_ahead)
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

inline Error Tokenizer::findTokenEnd(const Data &json_data, size_t *chars_ahead)
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

inline void Tokenizer::releaseFirstData()
{
    const char *data_to_release = 0;
    if (!data_list.empty()) {
        const Data &json_data = data_list.front();
        data_to_release = json_data.data;
        data_list.pop_front();
    }
    for (auto it = release_callback_list.begin(); it != release_callback_list.end(); ++it){
        (*it)(data_to_release);
    }
    cursor_index = 0;
    current_data_start = 0;
}

inline Error Tokenizer::populateFromData(Data &data, Token::Type *type, const Data &json_data)
{
    size_t diff = 0;
    Error error = Error::NoError;
    data.size = 0;
    data.data = json_data.data + cursor_index;
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

inline void Tokenizer::populate_annonymous_token(const Data &data, Token::Type type, Token &token)
{
    token.name = Data();
    token.name_type = Token::Ascii;
    token.value = data;
    token.value_type = type;
}

inline Error Tokenizer::populateNextTokenFromData(Token &next_token, const Data &json_data)
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
                    intermediate_token.active = true;
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

            if (intermediate_token.active) {
                intermediate_token.name.append(data.data, data.size);
                data = Data::asData(intermediate_token.name);
                type = intermediate_token.name_type;
            }

            if (type == Token::ObjectEnd || type == Token::ArrayEnd
                || type == Token::ArrayStart || type == Token::ObjectStart) {
                switch (type) {
                case Token::ObjectEnd:
                case Token::ArrayEnd:
                    if (expecting_prop_or_annonymous_data && !allow_superfluous_comma) {
                        return Error::ExpectedDataToken;
                    }
                    populate_annonymous_token(data,type,next_token);
                    token_state = FindingTokenEnd;
                    return Error::NoError;

                case Token::ObjectStart:
                case Token::ArrayStart:
                    populate_annonymous_token(data,type,next_token);
                    expecting_prop_or_annonymous_data = false;
                    token_state = FindingName;
                    return Error::NoError;
                default:
                    return Error::UnknownError;
                }
            } else {
                tmp_token.name = data;
            }

            tmp_token.name_type = type_checker.type(type, tmp_token.name.data,
                                                    tmp_token.name.size);
            token_state = FindingDelimiter;
            resetForNewValue();
            break;

        case FindingDelimiter:
            error = findDelimiter(json_data, &diff);
            if (error != Error::NoError) {
                if (intermediate_token.active == false) {
                    intermediate_token.name.append(tmp_token.name.data, tmp_token.name.size);
                    intermediate_token.name_type = tmp_token.name_type;
                    intermediate_token.active = true;
                }
                return Error::NeedMoreData;
            }
            cursor_index += diff;
            resetForNewValue();
            expecting_prop_or_annonymous_data = false;
            if (token_state == FindingName) {
                populate_annonymous_token(tmp_token.name, tmp_token.name_type, next_token);
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
                if (intermediate_token.active == false) {
                    intermediate_token.name.append(tmp_token.name.data, tmp_token.name.size);
                    intermediate_token.name_type = tmp_token.name_type;
                    intermediate_token.active = true;
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

            if (intermediate_token.active) {
                intermediate_token.data.append(data.data, data.size);
                if (!intermediate_token.data_type_set) {
                    intermediate_token.data_type = type;
                    intermediate_token.data_type_set = true;
                }
                tmp_token.name = Data::asData(intermediate_token.name);
                tmp_token.name_type = intermediate_token.name_type;
                data = Data::asData(intermediate_token.data);
                type = intermediate_token.data_type;
            }

            tmp_token.value = data;
            tmp_token.value_type = type_checker.type(type, tmp_token.value.data, tmp_token.value.size);

            if (tmp_token.value_type  == Token::Ascii && !allow_ascii_properties)
                return Error::IlligalDataValue;

            if (type == Token::ObjectStart || type == Token::ArrayStart) {
                token_state = FindingName;
            } else {
                token_state = FindingTokenEnd;
            }
            next_token = tmp_token;
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

} //Namespace
#endif //JSON_TOKENIZER_H
