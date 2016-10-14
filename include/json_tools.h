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

#ifndef JSON_TOOLS_H
#define JSON_TOOLS_H

#include <stddef.h>
#include <functional>
#include <vector>
#include <string>
#include <algorithm>
#include <stdlib.h>

#include <assert.h>

namespace JT {

struct DataRef
{
    DataRef()
        : data("")
        , size(0)
    {}

    DataRef(const char *data, size_t size)
        : data(data)
        , size(size)
    {}

    template <size_t N>
    static DataRef asDataRef(const char (&data)[N])
    {
        return DataRef(data, N - 1);
    }

    static DataRef asDataRef(const std::string &str)
    {
        return DataRef(str.c_str(), str.size());
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
    DataRef name;
    Type value_type;
    DataRef value;
};

struct IntermediateToken
{
    IntermediateToken()
    { }

    void clear() {
        if (!active)
            return;
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

    Token::Type type(Token::Type type, const char *data, size_t length) const {
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
    ExpectedObjectStart,
    ExpectedObjectEnd,
    ExpectedArrayStart,
    ExpectedArrayEnd,
    IlligalPropertyName,
    IlligalPropertyType,
    IlligalDataValue,
    EncounteredIlligalChar,
    CouldNotCreateNode,
    NodeNotFound,
    MissingPropertyMember,
    FailedToParseBoolen,
    FailedToParseDouble,
    FailedToParseFloat,
    FailedToParseInt,
    UnassignedRequiredMember,
    UnknownError,
    UserDefinedErrors
};

class ErrorContext
{
public:
    size_t line = 0;
    size_t character = 0;
    Error error = Error::NoError;
    std::vector<std::string> lines;

    void clear()
    {
        line = 0;
        character = 0;
        error = Error::NoError;
        lines.clear();
    }
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
    template<size_t N>
    void addData(const char (&data)[N]);
    size_t registered_buffers() const;
    void registerNeedMoreDataCallback(std::function<void(Tokenizer &)> callback, bool oneShotCallback);
    void registerRelaseCallback(std::function<void(const char *)> callback);

    Error nextToken(Token &next_token);
    void registerTokenTransformer(std::function<void(Token &next_token)> token_transformer);
    std::string currentErrorStringContext();

    std::string makeErrorString() const;
    void setErrorContextConfig(size_t lineContext, size_t rangeContext);
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
    Error findStringEnd(const DataRef &json_data, size_t *chars_ahead);
    Error findAsciiEnd(const DataRef &json_data, size_t *chars_ahead);
    Error findNumberEnd(const DataRef &json_data, size_t *chars_ahead);
    Error findStartOfNextValue(Token::Type *type,
                               const DataRef &json_data,
                               size_t *chars_ahead);
    Error findDelimiter(const DataRef &json_data, size_t *chars_ahead);
    Error findTokenEnd(const DataRef &json_data, size_t *chars_ahead);
    void requestMoreData();
    void releaseFirstDataRef();
    Error populateFromDataRef(DataRef &data, Token::Type *type, const DataRef &json_data);
    static void populate_annonymous_token(const DataRef &data, Token::Type type, Token &token);
    Error populateNextTokenFromDataRef(Token &next_token, const DataRef &json_data);
    void updateErrorContext(Error error);

    std::vector<DataRef> data_list;
    std::vector<std::function<void(const char *)>> release_callbacks;
    std::vector<std::pair<std::function<void(Tokenizer &)>, bool>> need_more_data_callabcks;
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
    std::string current_error_context_string;
    ErrorContext error_context;
    size_t line_context = 4;
    size_t line_range_context = 256;
    size_t range_context = 38;
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
    const std::vector<SerializerBuffer> &buffers() const;
    void clearBuffers();
private:
    void askForMoreBuffers();
    void markCurrentSerializerBufferFull();
    bool writeAsString(const DataRef &data);
    bool write(Token::Type type, const DataRef &data);
    bool write(const char *data, size_t size);
    bool write(const std::string &str) { return write(str.c_str(), str.size()); }

    std::vector <std::function<void(Serializer *)>> m_request_buffer_callbacks;
    std::vector <SerializerBuffer *> m_unused_buffers;
    std::vector <SerializerBuffer> m_all_buffers;

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
    data_list.push_back(DataRef(data, data_size));
}

template<size_t N>
inline void Tokenizer::addData(const char (&data)[N])
{
    data_list.push_back(DataRef::asDataRef(data));
}

inline size_t Tokenizer::registered_buffers() const
{
    return data_list.size();
}

inline void Tokenizer::registerNeedMoreDataCallback(std::function<void(Tokenizer &)> callback, bool oneShotCallback)
{
    need_more_data_callabcks.push_back(std::make_pair(callback, oneShotCallback));
}
inline void Tokenizer::registerRelaseCallback(std::function<void(const char *)> callback)
{
    release_callbacks.push_back(callback);
}

inline Error Tokenizer::nextToken(Token &next_token)
{
    if (data_list.empty()) {
        requestMoreData();
    }

    error_context.clear();

    if (data_list.empty()) {
        return Error::NeedMoreData;
    }

    if (!continue_after_need_more_data)
        resetForNewToken();

    Error error = Error::NeedMoreData;
    while (error == Error::NeedMoreData && data_list.size()) {
        const DataRef &json_data = data_list.front();
        error = populateNextTokenFromDataRef(next_token, json_data);

        if (error != Error::NoError && error != Error::NeedMoreData)
            updateErrorContext(error);

        if (error != Error::NoError) {
            releaseFirstDataRef();
        }
        if (error == Error::NeedMoreData) {
            requestMoreData();
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

inline std::string Tokenizer::currentErrorStringContext()
{
    return current_error_context_string;
}

inline std::string Tokenizer::makeErrorString() const
{
    static const char *error_strings[] = {
        "NoError",
        "NeedMoreData",
        "InvalidToken",
        "ExpectedPropertyName",
        "ExpectedDelimiter",
        "ExpectedDataToken",
        "ExpectedObjectStart",
        "ExpectedObjectEnd",
        "ExpectedArrayStart",
        "ExpectedArrayEnd",
        "IlligalPropertyName",
        "IlligalPropertyType",
        "IlligalDataValue",
        "EncounteredIlligalChar",
        "CouldNotCreateNode",
        "NodeNotFound",
        "MissingPropertyMember",
        "FailedToParseBoolen",
        "FailedToParseDouble",
        "FailedToParseFloat",
        "FailedToParseInt",
        "UnassignedRequiredMember",
        "UnknownError",
    };
    static_assert(sizeof(error_strings) / sizeof*error_strings == size_t(Error::UserDefinedErrors) , "Please add missing error message");

    std::string retString("Error ");
    retString += error_strings[int(error_context.error)] + std::string(":\n");
    for (int i = 0; i < error_context.lines.size(); i++)
    {
        retString += error_context.lines[i] + "\n";
        if (i == error_context.line) {
            std::string pointing(error_context.character + 1, ' ');
            pointing[error_context.character - 1] = '^';
            pointing[error_context.character] = '\n';
            retString += pointing;
        }
    }
    return retString;
}
    
inline void Tokenizer::setErrorContextConfig(size_t lineContext, size_t rangeContext)
{
    line_context = lineContext;
    range_context = rangeContext;
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

inline Error Tokenizer::findStringEnd(const DataRef &json_data, size_t *chars_ahead)
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

inline Error Tokenizer::findAsciiEnd(const DataRef &json_data, size_t *chars_ahead)
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

inline Error Tokenizer::findNumberEnd(const DataRef &json_data, size_t *chars_ahead)
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
                                      const DataRef &json_data,
                                      size_t *chars_ahead)
{

    assert(property_state == NoStartFound);

    for (size_t current_pos  = cursor_index; current_pos < json_data.size; current_pos++) {
        switch (*(json_data.data + current_pos)) {
        case ' ':
        case '\n':
        case '\t':
        case '\0':
            break;
        case '"':
            *type = Token::String;
            *chars_ahead = current_pos - cursor_index + 1;
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

inline Error Tokenizer::findDelimiter(const DataRef &json_data, size_t *chars_ahead)
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
        case '\t':
        case '\0':
            break;
        default:
            return Error::ExpectedDelimiter;
            break;
        }
    }
    return Error::NeedMoreData;
}

inline Error Tokenizer::findTokenEnd(const DataRef &json_data, size_t *chars_ahead)
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
        case '\t':
        case '\0':
            break;
        default:
            *chars_ahead = end + 1 - cursor_index;
            return Error::InvalidToken;
        }
    }
    return Error::NeedMoreData;
}

inline void Tokenizer::requestMoreData()
{
    for (auto pairs: need_more_data_callabcks)
    {
        pairs.first(*this);
    }
    need_more_data_callabcks.erase(std::remove_if(need_more_data_callabcks.begin(),
                              need_more_data_callabcks.end(),
                              [](decltype(*need_more_data_callabcks.begin()) &pair)
                                { return pair.second; }), need_more_data_callabcks.end());
}

inline void Tokenizer::releaseFirstDataRef()
{
    const char *data_to_release = 0;
    if (!data_list.empty()) {
        const DataRef &json_data = data_list.front();
        data_to_release = json_data.data;
        data_list.erase(data_list.begin());
    }
    for (auto function : release_callbacks) {
        function(data_to_release);
    }
    cursor_index = 0;
    current_data_start = 0;
}

inline Error Tokenizer::populateFromDataRef(DataRef &data, Token::Type *type, const DataRef &json_data)
{
    size_t diff = 0;
    Error error = Error::NoError;
    data.size = 0;
    data.data = json_data.data + cursor_index;
    if (property_state == NoStartFound) {
        error = findStartOfNextValue(type, json_data, &diff);
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

    int size_adjustment = 0;
    if (property_state == FindingEnd) {
        switch (*type) {
        case Token::String:
            error = findStringEnd(json_data, &diff);
            size_adjustment = -1;
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
        data.size = cursor_index - current_data_start + size_adjustment;
        property_state = FoundEnd;
    }

    return Error::NoError;
}

inline void Tokenizer::populate_annonymous_token(const DataRef &data, Token::Type type, Token &token)
{
    token.name = DataRef();
    token.name_type = Token::Ascii;
    token.value = data;
    token.value_type = type;
}

inline Error Tokenizer::populateNextTokenFromDataRef(Token &next_token, const DataRef &json_data)
{
    Token tmp_token;
    while (cursor_index < json_data.size) {
        size_t diff = 0;
        DataRef data;
        Token::Type type;
        Error error;
        switch (token_state) {
        case FindingName:
            type = intermediate_token.name_type;
            error = populateFromDataRef(data, &type, json_data);
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
                data = DataRef::asDataRef(intermediate_token.name);
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
                return error;
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
            error = populateFromDataRef(data, &type, json_data);
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
                tmp_token.name = DataRef::asDataRef(intermediate_token.name);
                tmp_token.name_type = intermediate_token.name_type;
                data = DataRef::asDataRef(intermediate_token.data);
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

struct lines
{
    size_t start;
    size_t end;
};

inline void Tokenizer::updateErrorContext(Error error)
{
    error_context.error = error;
    std::vector<lines> lines;
    lines.push_back({0, cursor_index});
    const DataRef &json_data = data_list.front();
    const size_t stop_back = cursor_index - std::min(cursor_index, line_range_context);
    const size_t stop_forward = std::min(cursor_index + line_range_context, json_data.size);
    assert(cursor_index <= json_data.size);
    size_t lines_back = 0;
    size_t lines_forward = 0;
    size_t cursor_back;
    size_t cursor_forward;
    for (cursor_back = cursor_index - 1; cursor_back > stop_back; cursor_back--)
    {
        if (*(json_data.data + cursor_back) == '\n') {
            lines.front().start = cursor_back + 1;
            lines_back++;
            if (lines_back == 1)
                error_context.character = cursor_index - cursor_back;
            if (lines_back == line_context) {
                lines_back--;
                break;
            }

            lines.insert(lines.begin(), {0, cursor_back});
        }
    }
    if (lines.front().start == 0)
        lines.front().start = cursor_back;
    for (cursor_forward = cursor_index; cursor_forward < stop_forward; cursor_forward++)
    {
        if (*(json_data.data + cursor_forward) == '\n') {
            lines.back().end = cursor_forward;
            lines_forward++;
            if (lines_forward == line_context)
                break;
            lines.push_back({cursor_forward, 0});
        }
    }
    if (lines.back().end == 0)
        lines.back().end = cursor_forward - 1;

    if (lines.size() > 1) {
        error_context.lines.reserve(lines.size());
        for (auto &line : lines)
        {
            error_context.lines.push_back(std::string(json_data.data + line.start, line.end - line.start));
        }
        error_context.line = lines_back;
    } else {
        error_context.line = 0;

        size_t left = cursor_index > range_context ? cursor_index - range_context : 0;
        size_t right = cursor_index + range_context > json_data.size ? json_data.size : cursor_index + range_context;
        error_context.character = cursor_index - left;
        error_context.lines.push_back(std::string(json_data.data + left, right - left));
    }
}

inline SerializerOptions::SerializerOptions(bool pretty, bool ascii)
    : m_shift_size(4)
    , m_depth(0)
    , m_pretty(pretty)
    , m_ascii(ascii)
    , m_token_delimiter(",")
{
    m_value_delimiter = m_pretty? std::string(" : ") : std::string(":");
    m_postfix = m_pretty? std::string("\n") : std::string("");
}

inline int SerializerOptions::shiftSize() const { return m_shift_size; }

inline int SerializerOptions::depth() const { return m_depth; }

inline bool SerializerOptions::pretty() const { return m_pretty; }
inline void SerializerOptions::setPretty(bool pretty)
{
    m_pretty = pretty;
    m_postfix = m_pretty? std::string("\n") : std::string("");
    m_value_delimiter = m_pretty? std::string(" : ") : std::string(":");
    setDepth(m_depth);
}

inline bool SerializerOptions::ascii() const { return m_ascii; }
inline void SerializerOptions::setAscii(bool ascii)
{
    m_ascii = ascii;
}

inline void SerializerOptions::skipDelimiter(bool skip)
{
    if (skip)
        m_token_delimiter = "";
    else
        m_token_delimiter = ",";
}

inline void SerializerOptions::setDepth(int depth)
{
    m_depth = depth;
    m_prefix = m_pretty? std::string(depth * m_shift_size, ' ') : std::string();
}

inline const std::string &SerializerOptions::prefix() const { return m_prefix; }
inline const std::string &SerializerOptions::tokenDelimiter() const { return m_token_delimiter; }
inline const std::string &SerializerOptions::valueDelimiter() const { return m_value_delimiter; }
inline const std::string &SerializerOptions::postfix() const { return m_postfix; }

inline bool SerializerBuffer::append(const char *data, size_t size)
{
    if (used + size > this->size)
        return false;

    memcpy(buffer + used, data, size);
    used += size;
    return true;
}

inline Serializer::Serializer()
{
}

inline Serializer::Serializer(char *buffer, size_t size)
{
    appendBuffer(buffer,size);
}

inline void Serializer::appendBuffer(char *buffer, size_t size)
{
    m_all_buffers.push_back({buffer,size,0});
    m_unused_buffers.push_back(&m_all_buffers.back());
}

inline void Serializer::setOptions(const SerializerOptions &option)
{
    m_option = option;
}

inline bool Serializer::write(const Token &in_token)
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

inline void Serializer::registerTokenTransformer(std::function<const Token&(const Token &)> token_transformer)
{
    this->m_token_transformer = token_transformer;
}

inline void Serializer::addRequestBufferCallback(std::function<void(Serializer *)> callback)
{
    m_request_buffer_callbacks.push_back(callback);
}

inline const std::vector<SerializerBuffer> &Serializer::buffers() const
{
    return m_all_buffers;
}

inline void Serializer::clearBuffers()
{
    m_all_buffers.clear();
    m_unused_buffers.clear();
}

inline void Serializer::askForMoreBuffers()
{
    for (auto it = m_request_buffer_callbacks.begin();
            it != m_request_buffer_callbacks.end();
            ++it) {
        (*it)(this);
    }
}

inline void Serializer::markCurrentSerializerBufferFull()
{
    m_unused_buffers.erase(m_unused_buffers.begin());
    if (m_unused_buffers.size() == 0)
        askForMoreBuffers();
}

inline bool Serializer::writeAsString(const DataRef &data)
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

inline bool Serializer::write(Token::Type type, const DataRef &data)
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

inline bool Serializer::write(const char *data, size_t size)
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

template<typename T>
struct Optional
{
    Optional()
        : data()
    {
    }
    Optional(const T &t)
        : data(t)
    {
    }

    Optional<T> &operator= (const T &other)
    {
        data = other;
        return *this;
    }

    T data;
    T &operator()() { return data; }
    const T &operator()() const { return data; }
    typedef bool HasJTOptionalValue;
};

template<typename T>
struct OptionalChecked
{
    OptionalChecked()
        : data()
        , assigned(false)
    { }
    OptionalChecked(const T &t)
        : data(t)
        , assigned(true)
    { }
    OptionalChecked<T> &operator= (const T &other)
    {
        data = other;
        assigned = true;
        return *this;
    }

    T data;
    bool assigned;
    typedef bool HasJTOptionalValue;
};

template <typename T>
struct HasJTOptionalValue{
    typedef char yes[1];
    typedef char no[2];

    template <typename C>
    static constexpr yes& test(typename C::HasJTOptionalValue*);

    template <typename>
    static constexpr no& test(...);

    static constexpr const bool value = sizeof(test<T>(nullptr)) == sizeof(yes);
};

struct ParseContext
{
    Tokenizer tokenizer;
    Token token;
    Error error = Error::NoError;
    std::vector<std::string> missing_members;
    std::vector<std::string> unassigned_required_members;
    bool allow_missing_members = true;
    bool allow_unnasigned_required__members = true;
};

#define JT_FIELD(name) JT::makeMemberInfo(#name, &T::name)
#define JT_SUPER_CLASSES(...) JT::MemberFieldsTuple<__VA_ARGS__>::create()

#define JT_STRUCT(...) \
    template<typename T> \
    struct JsonToolsBase \
    { \
       static const decltype(std::make_tuple(__VA_ARGS__)) _fields() \
       { static auto ret = std::make_tuple(__VA_ARGS__); return ret; } \
    };

#define JT_STRUCT_WITH_SUPER(super_list, ...) \
    template<typename T> \
    struct JsonToolsBase \
    { \
        static const decltype(std::tuple_cat(std::make_tuple(__VA_ARGS__), super_list)) &_fields() \
        { static auto ret = std::tuple_cat(std::make_tuple(__VA_ARGS__), super_list); return ret; } \
    };

template <typename T>
struct HasJsonToolsBase {
    typedef char yes[1];
    typedef char no[2];

    template <typename C>
    static constexpr yes& test(typename C::template JsonToolsBase<C>*);

    template <typename>
    static constexpr no& test(...);

    static constexpr const bool value = sizeof(test<T>(nullptr)) == sizeof(yes);
};

template<typename T, typename U, size_t NAME_SIZE>
struct MemberInfo
{
    const char *name;
    T U::* member;
    typedef T type;
};

template<typename T, typename U, size_t NAME_SIZE>
constexpr MemberInfo<T, U, NAME_SIZE - 1> makeMemberInfo(const char (&name)[NAME_SIZE], T U::* member)
{
    return {name, member};
}

template<typename... Args>
struct MemberFieldsTuple;

template<typename T, typename... Args>
struct MemberFieldsTuple<T, Args...>
{
    static auto create() -> decltype(std::tuple_cat(T::template JsonToolsBase<T>::_fields(), MemberFieldsTuple<Args...>::create()))
    {
        static_assert(HasJsonToolsBase<T>::value, "Type is not a json struct type");
        return std::tuple_cat(T::template JsonToolsBase<T>::_fields(), MemberFieldsTuple<Args...>::create());
    }
};

template<>
struct MemberFieldsTuple<>
{
    static auto create() -> decltype(std::make_tuple())
    {
        return std::make_tuple();
    }
};

template<typename T, typename specifier>
class TokenParser
{
public:
    static inline Error unpackToken(T &to_type, ParseContext &context)
    {
        static_assert(sizeof(T) == 0xffffffff, "Missing TokenParser specialisation\n");
        return Error::NoError;
    }

    static inline void serializeToken(const T &from_type, Token &token, Serializer &serializer)
    {
        static_assert(sizeof(T) == 0xffffffff, "Missing TokenParser specialisation\n");
    }
};

template<typename T, typename MI_T, typename MI_M, size_t MI_S>
inline Error unpackField(T &to_type, const MemberInfo<MI_T, MI_M, MI_S> &memberInfo, ParseContext &context,  size_t index, bool *assigned_fields)
{
    if (memcmp(memberInfo.name, context.token.name.data, MI_S) == 0)
    {
        assigned_fields[index] = true;
        return TokenParser<MI_T, MI_T>::unpackToken(to_type.*memberInfo.member, context);
    }
    return Error::MissingPropertyMember;
}

template<typename MI_T, typename MI_M, size_t MI_S>
inline Error verifyField(const MemberInfo<MI_T, MI_M, MI_S> &memberInfo, size_t index, bool *assigned_fields, std::vector<std::string> &missing_fields)
{
    if (assigned_fields[index])
        return Error::NoError;
    if (HasJTOptionalValue<MI_T>::value)
        return Error::NoError;
    missing_fields.push_back(std::string(memberInfo.name, MI_S));
    return Error::UnassignedRequiredMember;
}

template<typename T, typename MI_T, typename MI_M, size_t MI_S>
inline void serializeField(const T &from_type, const MemberInfo<MI_T, MI_M, MI_S> &memberInfo, Token &token, Serializer serializer)
{
    token.name.data = memberInfo.name;
    token.name.size = MI_S - 1;

    TokenParser<MI_T, MI_T>::serializeToken(from_type.*memberInfo.member, token, serializer);
}

template<typename T, typename Fields, size_t INDEX>
struct FieldChecker
{
    static Error unpackFields(T &to_type, const Fields &fields, ParseContext &context, bool *assigned_fields)
    {
        Error error = unpackField(to_type, std::get<INDEX>(fields), context, INDEX, assigned_fields);
        if (error != Error::MissingPropertyMember)
            return error;

        return FieldChecker<T, Fields, INDEX - 1>::unpackFields(to_type, fields, context, assigned_fields);
    }

    static Error verifyFields(const Fields &fields, bool *assigned_fields, std::vector<std::string> &missing_fields)
    {
        Error fieldError = verifyField(std::get<INDEX>(fields), INDEX, assigned_fields, missing_fields);
        Error error = FieldChecker<T, Fields, INDEX - 1>::verifyFields(fields, assigned_fields, missing_fields);
        if (fieldError != Error::NoError)
            return fieldError;
        return error;
    }
    static void serializeFields(const T &from_type, const Fields &fields, Token &token, Serializer &serializer)
    {
        serializeField(from_type, std::get<INDEX>(fields), token, serializer);
        FieldChecker<T, Fields, INDEX -1>::serializeFields(from_type, fields, token, serializer);
    }
};

template<typename T, typename Fields>
struct FieldChecker<T, Fields, 0>
{
    static Error unpackFields(T &to_type, const Fields &fields, ParseContext &context, bool *assigned_fields)
    {
        return unpackField(to_type, std::get<0>(fields), context, 0, assigned_fields);
    }

    static Error verifyFields(const Fields &fields, bool *assigned_fields, std::vector<std::string> &missing_fields)
    {
        return verifyField(std::get<0>(fields), 0, assigned_fields, missing_fields);
    }

    static void serializeFields(const T &from_type, const Fields &fields, Token &token, Serializer &serializer)
    {
        serializeField(from_type, std::get<0>(fields), token, serializer);
    }
};

template<typename T>
class TokenParser<T, typename std::enable_if<HasJsonToolsBase<T>::value, T>::type>
{
public:
    static inline Error unpackToken(T &to_type, ParseContext &context)
    {
        if (context.token.value_type != JT::Token::ObjectStart)
            return Error::ExpectedObjectStart;
        Error error = context.tokenizer.nextToken(context.token);
        if (error != JT::Error::NoError)
            return error;
        auto fields = T::template JsonToolsBase<T>::_fields();
        bool assigned_fields[std::tuple_size<decltype(fields)>::value];
        memset(assigned_fields, 0, sizeof(assigned_fields));
        while(context.token.value_type != JT::Token::ObjectEnd)
        {
            std::string token_name(context.token.name.data, context.token.name.size);
            error = FieldChecker<T, decltype(fields), std::tuple_size<decltype(fields)>::value - 1>::unpackFields(to_type, fields, context, assigned_fields);
            if (error == Error::MissingPropertyMember) {
                context.missing_members.push_back(token_name);
                if (!context.allow_missing_members)
                    return error;
            } else if (error != Error::NoError) {
                return error;
            }
            error = context.tokenizer.nextToken(context.token);
            if (error != Error::NoError)
                return error;
        }
        assert(error == Error::NoError);
        std::vector<std::string> unassigned_required_fields;
        error = FieldChecker<T, decltype(fields), std::tuple_size<decltype(fields)>::value - 1>::verifyFields(fields, assigned_fields, unassigned_required_fields);
        if (error == Error::UnassignedRequiredMember) {
            context.unassigned_required_members.insert(context.unassigned_required_members.end(),unassigned_required_fields.begin(), unassigned_required_fields.end());
            if (context.allow_unnasigned_required__members)
                error = Error::NoError;
        }
        return error;
    }

    static inline void serializeToken(const T &from_type, Token &token, Serializer &serializer)
    {
        static const char objectStart[] = "{";
        static const char objectEnd[] = "}";
        token.value_type = Token::ObjectStart;
        token.value = DataRef::asDataRef(objectStart);
        serializer.write(token);
        auto fields = T::template JsonToolsBase<T>::_fields();
        FieldChecker<T, decltype(fields), std::tuple_size<decltype(fields)>::value - 1>::serializeFields(from_type, fields, token, serializer);
    }
};

template<>
inline Error TokenParser<std::string, std::string>::unpackToken(std::string &to_type, ParseContext &context)
{
    to_type = std::string(context.token.value.data, context.token.value.size);
    return Error::NoError;
}

template<>
inline void TokenParser<std::string, std::string>::serializeToken(const std::string &, Token &token, Serializer &serializer)
{
    fprintf(stderr, "serialize string\n");
}

template<>
inline Error TokenParser<double, double>::unpackToken(double &to_type, ParseContext &context)
{
    char *pointer;
    to_type = strtod(context.token.value.data, &pointer);
    if (context.token.value.data == pointer)
        return Error::FailedToParseFloat;
    return Error::NoError;
}

template<>
inline Error TokenParser<float, float>::unpackToken(float &to_type, ParseContext &context)
{
    char *pointer;
    to_type = strtof(context.token.value.data, &pointer);
    if (context.token.value.data == pointer)
        return Error::FailedToParseFloat;
    return Error::NoError;
}

template<>
inline void TokenParser<float, float>::serializeToken(const float &, Token &token, Serializer &serializer)
{
    fprintf(stderr, "serialize float\n");
}

template<>
inline Error TokenParser<int, int>::unpackToken(int &to_type, ParseContext &context)
{
    char *pointer;
    to_type = strtol(context.token.value.data, &pointer, 10);
    if (context.token.value.data == pointer)
        return Error::FailedToParseInt;
    return Error::NoError;
}

template<typename T>
class TokenParser<Optional<T>, Optional<T>>
{
public:
    static inline Error unpackToken(Optional<T> &to_type, ParseContext &context)
    {
        return TokenParser<T,T>::unpackToken(to_type.data, context);
    }
};

template<typename T>
class TokenParser<OptionalChecked<T>, OptionalChecked<T>>
{
public:
    static inline Error unpackToken(OptionalChecked<T> &to_type, ParseContext &context)
    {
        to_type.assigned = true;
        return TokenParser<T,T>::unpackToken(to_type.data, context);
    }
};

template<>
inline Error TokenParser<bool, bool>::unpackToken(bool &to_type, ParseContext &context)
{
    if (memcmp("true", context.token.value.data, sizeof("true") - 1))
        to_type = true;
    else if (memcmp("false", context.token.value.data, sizeof("false") - 1))
        to_type = false;
    else
        return Error::FailedToParseBoolen;

    return Error::NoError;
}

template<typename T>
class TokenParser<std::vector<T>, std::vector<T>>
{
public:
    static inline Error unpackToken(std::vector<T> &to_type, ParseContext &context)
    {
        if (context.token.value_type != JT::Token::ArrayStart)
            return Error::ExpectedArrayStart;
        Error error = context.tokenizer.nextToken(context.token);
        while(context.token.value_type != JT::Token::ArrayEnd)
        {
            to_type.push_back(T());
            error = TokenParser<T,T>::unpackToken(to_type.back(), context);
            if (error != JT::Error::NoError)
                break;
            error = context.tokenizer.nextToken(context.token);
            if (error != JT::Error::NoError)
                break;
        }

        return error;
    }
};

inline ParseContext makeParseContextForData(const char *json_data, size_t size)
{
    ParseContext ret;
    ret.tokenizer.registerNeedMoreDataCallback([json_data, size](Tokenizer &tokenizer) {
                                                   tokenizer.addData(json_data, size);
                                                   }, true);
    return ret;
}

template<typename T>
void parseData(T &to_type, ParseContext &context)
{
    context.error = context.tokenizer.nextToken(context.token);
    if (context.error != JT::Error::NoError)
        return;
    context.error = TokenParser<T,T>::unpackToken(to_type, context);
    return;
}

template<typename T>
T parseData(ParseContext &context)
{
    T ret;
    parseData(ret, context);
    return ret;
}

template<typename T>
std::string serializeStruct(const T &from_type)
{
    char out_buffer[512];
    Serializer serializer(out_buffer, sizeof(out_buffer));
    Token token;
    TokenParser<T,T>::serializeToken(from_type, token, serializer);
    return std::string();
}

} //Namespace
#endif //JSON_TOOLS_H
