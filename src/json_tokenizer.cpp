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

class AsciiState
{
public:
    AsciiState()
        : m_null("null")
        , m_true("true")
        , m_false("false")
    { }
    JsonToken::Type type(const char *data, int length) {
        if (m_null.compare(0,m_null.size(), data, 0, length) == 0)
            return JsonToken::Null;
        if (m_true.compare(0,m_true.size(), data, 0, length) == 0)
            return JsonToken::Bool;
        if (m_false.compare(0,m_false.size(), data, 0, length) == 0)
            return JsonToken::Bool;
        return JsonToken::Ascii;
    }
private:
    std::string m_null;
    std::string m_true;
    std::string m_false;
};

struct JsonData
{
    JsonData(const char *data, size_t size, void *user_handle)
        : data(data)
        , size(size)
        , user_handle(user_handle)
    {}

    const char *data;
    int size;
    void *user_handle;
};

struct JsonIntermediateToken
{
    JsonIntermediateToken()
        : intermedia_set(false)
        , name_type_set(false)
        , data_type_set(false)
        , name_type(JsonToken::Error)
        , data_type(JsonToken::Error)
    { }

    void clear() {
        intermedia_set = false;
        name_type_set = false;
        data_type_set = false;
        name_type = JsonToken::Error;
        data_type = JsonToken::Error;
        name.clear();
        data.clear();
    }

    bool intermedia_set;
    bool name_type_set;
    bool data_type_set;
    JsonToken::Type name_type;
    JsonToken::Type data_type;
    std::string name;
    std::string data;
};

struct JsonTokenizerPrivate
{
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

    JsonTokenizerPrivate(ReleaseDataCallback release_data_callback)
        : cursor_index(0)
        , current_data_start(0)
        , token_state(FindingName)
        , property_state(NoStartFound)
        , property_type(JsonToken::Error)
        , is_escaped(false)
        , allow_ascii_properties(false)
        , allow_new_lines(false)
        , expecting_prop_or_annonymous_data(false)
        , continue_after_need_more_data(false)
        , release_data_callback(release_data_callback)
    {
    }

    ~JsonTokenizerPrivate()
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
        property_type = JsonToken::Error;
        current_data_start = 0;
    }

    JsonError findStringEnd(const JsonData &json_data, size_t *chars_ahead)
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
                    return JsonError::NoError;

                default:
                    break;
            }
        }
        return JsonError::NeedMoreData;
    }

    JsonError findAsciiEnd(const JsonData &json_data, size_t *chars_ahead)
    {
        assert(property_type == JsonToken::Ascii);
        for (size_t end = cursor_index; end < json_data.size; end++) {
            char ascii_code = *(json_data.data + end);
            if ((ascii_code >= 'A' && ascii_code <= 'Z') ||
                    (ascii_code >= '^' && ascii_code <= 'z') ||
                    (ascii_code >= '0' && ascii_code <= '9')) {
                continue;
            } else if (ascii_code == '\0') {
                *chars_ahead = end - cursor_index;
                return JsonError::NeedMoreData;
            } else {
                *chars_ahead = end - cursor_index;
                return JsonError::NoError;
            }
        }
        return JsonError::NeedMoreData;
    }

    JsonError findNumberEnd(const JsonData &json_data, size_t *chars_ahead)
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
                    return JsonError::NoError;
            }
        }
        return JsonError::NeedMoreData;
    }

    JsonError findStartOfNextValue(JsonToken::Type *type,
            const JsonData &json_data,
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
                    *type = JsonToken::String;
                    *chars_ahead = current_pos - cursor_index;
                    return JsonError::NoError;
                case '{':
                    *type = JsonToken::ObjectStart;
                    *chars_ahead = current_pos - cursor_index;
                    return JsonError::NoError;
                case '}':
                    *type = JsonToken::ObjectEnd;
                    *chars_ahead = current_pos - cursor_index;
                    return JsonError::NoError;
                case '[':
                    *type = JsonToken::ArrayStart;
                    *chars_ahead = current_pos - cursor_index;
                    return JsonError::NoError;
                case ']':
                    *type = JsonToken::ArrayEnd;
                    *chars_ahead = current_pos - cursor_index;
                    return JsonError::NoError;
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
                    *type = JsonToken::Number;
                    *chars_ahead = current_pos - cursor_index;
                    return JsonError::NoError;
                default:
                    char ascii_code = *(json_data.data + current_pos);
                    if ((ascii_code >= 'A' && ascii_code <= 'Z') ||
                            (ascii_code >= '^' && ascii_code <= 'z')) {
                        *type = JsonToken::Ascii;
                        *chars_ahead = current_pos - cursor_index;;
                        return JsonError::NoError;
                    } else {
                        *chars_ahead = current_pos - cursor_index;
                        return JsonError::EncounteredIlligalChar;
                    }
                    break;
            }

        }
        return JsonError::NeedMoreData;
    }

    JsonError findDelimiter(const JsonData &json_data, size_t *chars_ahead)
    {
        for (size_t end = cursor_index; end < json_data.size; end++) {
            switch(*(json_data.data + end)) {
                case ':':
                    token_state = FindingData;
                    *chars_ahead = end + 1 - cursor_index;
                    return JsonError::NoError;
                case ',':
                    token_state = FindingName;
                    *chars_ahead = end + 1 - cursor_index;
                    return JsonError::NoError;
                case ']':
                    token_state = FindingName;
                    *chars_ahead = end - cursor_index;
                    return JsonError::NoError;
                case ' ':
                    break;
                default:
                    return JsonError::ExpectedDelimiter;
                    break;
            }
        }
        return JsonError::NeedMoreData;
    }

    JsonError findTokenEnd(const JsonData &json_data, size_t *chars_ahead)
    {
        for (size_t end = cursor_index; end < json_data.size; end++) {
            switch(*(json_data.data + end)) {
                case ',':
                    expecting_prop_or_annonymous_data = true;
                    *chars_ahead = end + 1 - cursor_index;
                    return JsonError::NoError;
                case '\n':
                    if (allow_new_lines) {
                        *chars_ahead = end + 1 - cursor_index;
                        return JsonError::NoError;
                    }
                    break;
                case ']':
                case '}':
                    *chars_ahead = end - cursor_index;
                    return JsonError::NoError;
                case ' ':
                case '\0':
                    break;
                default:
                    *chars_ahead = end + 1 - cursor_index;
                    return JsonError::InvalidToken;
            }
        }
        return JsonError::NeedMoreData;
    }

    void releaseFirstJsonData()
    {
        const JsonData &json_data = data_list.front();
        if (release_data_callback) {
            release_data_callback(json_data.data, json_data.user_handle);
        }
        data_list.pop_front();
        cursor_index = 0;
        current_data_start = 0;
    }

    JsonError populateFromData(const char **data, int *length, JsonToken::Type *type, const JsonData &json_data)
    {
        size_t diff = 0;
        JsonError error = JsonError::NoError;
        *length = 0;
        *data = json_data.data + cursor_index;
        if (property_state == NoStartFound) {
            JsonError error = findStartOfNextValue(type, json_data, &diff);
            if (error != JsonError::NoError) {
                *type = JsonToken::Error;
                return error;
            }

            *data = json_data.data + cursor_index + diff;
            current_data_start = cursor_index + diff;
            cursor_index += diff + 1;
            property_type = *type;


            if (*type == JsonToken::ObjectStart || *type == JsonToken::ObjectEnd
                    || *type == JsonToken::ArrayStart || *type == JsonToken::ArrayEnd) {
                *length = 1;
                property_state = FoundEnd;
            } else {
                property_state = FindingEnd;
            }
        }

        if (property_state == FindingEnd) {
            switch (*type) {
            case JsonToken::String:
                error = findStringEnd(json_data, &diff);
                break;
            case JsonToken::Ascii:
                error = findAsciiEnd(json_data, &diff);
                break;
            case JsonToken::Number:
                error = findNumberEnd(json_data, &diff);
                break;
            default:
                return JsonError::InvalidToken;
            }

            if (error != JsonError::NoError) {
                return error;
            }

            cursor_index += diff;
            *length = cursor_index - current_data_start;
            property_state = FoundEnd;
        }

        return JsonError::NoError;
    }

    JsonError populateNextTokenFromData(JsonToken *next_token, const JsonData &json_data)
    {
        while (cursor_index < json_data.size) {
            size_t diff = 0;
            const char *data;
            int data_length;
            JsonToken::Type type;
            JsonError error;
            switch (token_state) {
                case FindingName:
                    error = populateFromData(&data, &data_length, &type, json_data);
                    if (error == JsonError::NeedMoreData) {
                        if (property_state > NoStartFound) {
                            intermediate_token.intermedia_set = true;
                            size_t to_null = strnlen(data , json_data.size - current_data_start);
                            intermediate_token.name.append(data , to_null);
                            if (!intermediate_token.name_type_set) {
                                intermediate_token.name_type = type;
                                intermediate_token.name_type_set = true;
                            }
                        }
                        return error;
                    }

                    if (intermediate_token.intermedia_set) {
                        intermediate_token.name.append(data, data_length);
                        data_length = intermediate_token.name.length();
                        data = intermediate_token.name.c_str();
                        type = intermediate_token.name_type;
                    }

                    switch (type) {
                        case JsonToken::ObjectEnd:
                        case JsonToken::ArrayEnd:
                            if (expecting_prop_or_annonymous_data) {
                                return JsonError::ExpectedDataToken;
                            }
                        case JsonToken::ObjectStart:
                        case JsonToken::ArrayStart:
                            next_token->name = "";
                            next_token->name_length = 0;
                            next_token->name_type = JsonToken::String;
                            next_token->data = data;
                            next_token->data_length = data_length;
                            next_token->data_type = type;
                            expecting_prop_or_annonymous_data = false;
                            if (type == JsonToken::ObjectStart || type == JsonToken::ArrayStart)
                                token_state = FindingName;
                            else
                                token_state = FindingTokenEnd;
                            return JsonError::NoError;

                        case JsonToken::String:
                            next_token->name = data + 1;
                            next_token->name_length = data_length - 2;
                            break;
                        default:
                            next_token->name = data;
                            next_token->name_length = data_length;
                            break;
                    }

                    if (error != JsonError::NoError)
                        return error;

                    if (type == JsonToken::Ascii) {
                        next_token->name_type = ascii_state.type(next_token->name, next_token->name_length);
                    } else {
                        next_token->name_type = type;
                    }
                    token_state = FindingDelimiter;
                    resetForNewValue();
                    break;

                case FindingDelimiter:
                    error = findDelimiter(json_data, &diff);
                    if (error != JsonError::NoError) {
                        if (intermediate_token.intermedia_set == false) {
                            intermediate_token.name.append(next_token->name, next_token->name_length);
                            intermediate_token.name_type = next_token->name_type;
                            intermediate_token.intermedia_set = true;
                        }
                        return JsonError::NeedMoreData;
                    }
                    cursor_index += diff;
                    resetForNewValue();
                    expecting_prop_or_annonymous_data = false;
                    if (token_state == FindingName) {
                        //anonymous data object
                        next_token->data = next_token->name;
                        next_token->name = 0;
                        next_token->data_length = next_token->name_length;
                        next_token->name_length = 0;
                        next_token->data_type = next_token->name_type;
                        next_token->name_type = JsonToken::String;
                        return JsonError::NoError;
                    } else {
                        if (next_token->name_type != JsonToken::String) {
                            if (!allow_ascii_properties || next_token->name_type != JsonToken::Ascii) {
                                return JsonError::IlligalPropertyName;
                            }
                        }
                    }
                    break;

                case FindingData:
                    error = populateFromData(&data, &data_length, &type, json_data);
                    if (error == JsonError::NeedMoreData) {
                        if (intermediate_token.intermedia_set == false) {
                            intermediate_token.name.append(next_token->name, next_token->name_length);
                            intermediate_token.name_type = next_token->name_type;
                            intermediate_token.intermedia_set = true;
                        }
                        if (property_state > NoStartFound) {
                            size_t data_length = strnlen(data , json_data.size - current_data_start);
                            intermediate_token.data.append(data, data_length);
                            if (!intermediate_token.data_type_set) {
                                intermediate_token.data_type = type;
                                intermediate_token.data_type_set = true;
                            }
                        }
                        return error;
                    }

                    if (intermediate_token.intermedia_set) {
                        intermediate_token.data.append(data, data_length);
                        if (!intermediate_token.data_type_set) {
                            intermediate_token.data_type = type;
                            intermediate_token.data_type_set = true;
                        }
                        data = intermediate_token.data.c_str();
                        data_length = intermediate_token.data.length();
                        type = intermediate_token.data_type;
                    }

                    if (type == JsonToken::String) {
                        next_token->data = data + 1;
                        next_token->data_length = data_length - 2;
                    } else {
                        next_token->data = data;
                        next_token->data_length = data_length;
                    }
                    if (type == JsonToken::Ascii) {
                        next_token->data_type = ascii_state.type(next_token->data, next_token->data_length);
                    } else {
                        next_token->data_type = type;
                    }

                    if (next_token->data_type  == JsonToken::Ascii && !allow_ascii_properties) 
                        return JsonError::IlligalDataValue;

                    if (type == JsonToken::ObjectStart || type == JsonToken::ArrayStart)
                        token_state = FindingName;

                    if (error != JsonError::NoError)
                        return error;

                    if (next_token->data_type == JsonToken::ObjectStart
                            || next_token->data_type == JsonToken::ArrayStart) {
                        return JsonError::NoError;
                    }
                    token_state = FindingTokenEnd;
                    return JsonError::NoError;
                case FindingTokenEnd:
                    error = findTokenEnd(json_data, &diff);
                    if (error != JsonError::NoError) {
                        return error;
                    }
                    cursor_index += diff;
                    token_state = FindingName;
                    break;
            }
        }
        return JsonError::NeedMoreData;
    }

    std::list<JsonData> data_list;
    size_t cursor_index;
    size_t current_data_start;
    InTokenState token_state;
    InPropertyState property_state;
    JsonToken::Type property_type;
    AsciiState ascii_state;
    bool is_escaped;
    bool allow_ascii_properties;
    bool allow_new_lines;
    bool expecting_prop_or_annonymous_data;
    bool continue_after_need_more_data;
    JsonIntermediateToken intermediate_token;
    ReleaseDataCallback release_data_callback;
};

JsonTokenizer::JsonTokenizer(ReleaseDataCallback release_data_callback)
    : m_private(new JsonTokenizerPrivate(release_data_callback))
{
}

JsonTokenizer::~JsonTokenizer()
{
    delete m_private;
}

void JsonTokenizer::addData(const char *data, size_t data_size, void *user_handle)
{
    JsonData json_data(data, data_size, user_handle);
    m_private->data_list.push_back(json_data);
}

void JsonTokenizer::allowAsciiType(bool allow)
{
    m_private->allow_ascii_properties = allow;
}

void JsonTokenizer::allowNewLineAsTokenDelimiter(bool allow)
{
    m_private->allow_new_lines = allow;
}

JsonError JsonTokenizer::nextToken(JsonToken *next_token)
{
    if (!m_private->data_list.size()) {
        return JsonError::NeedMoreData;
    }

    if (!m_private->continue_after_need_more_data)
        m_private->resetForNewToken();

    JsonError error = JsonError::NeedMoreData;
    int trying = 0;
    while (error == JsonError::NeedMoreData && m_private->data_list.size()) {
        const JsonData &json_data = m_private->data_list.front();
        error = m_private->populateNextTokenFromData(next_token, json_data);

        if (error != JsonError::NoError) {
            m_private->releaseFirstJsonData();
        }
    }

    m_private->continue_after_need_more_data = error == JsonError::NeedMoreData;

    return error;
}
