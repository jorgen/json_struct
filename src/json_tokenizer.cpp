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
        //trusting std::string to just return false of different sizes
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
    }

    JsonTokenizer::Error findStringEnd(const JsonData &json_data, size_t *chars_ahead)
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
                    *chars_ahead = end - cursor_index;
                    return JsonTokenizer::NoError;

                default:
                    break;
            }
        }
        return JsonTokenizer::NeedMoreData;
    }

    JsonTokenizer::Error findAsciiEnd(const JsonData &json_data, size_t *chars_ahead)
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
                return JsonTokenizer::NeedMoreData;
            } else {
                *chars_ahead = end - cursor_index - 1;
                std::string ascii(json_data.data + cursor_index, end + 1 - cursor_index);
                return JsonTokenizer::NoError;
            }
        }
        return JsonTokenizer::NeedMoreData;
    }

    JsonTokenizer::Error findNumberEnd(const JsonData &json_data, size_t *chars_ahead)
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
                    *chars_ahead = end - cursor_index -1;
                    return JsonTokenizer::NoError;
            }
        }
        return JsonTokenizer::NeedMoreData;
    }

    JsonTokenizer::Error findStartOfNextValue(JsonToken::Type *type,
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
                    *chars_ahead = current_pos - cursor_index;;
                    return JsonTokenizer::NoError;
                case '{':
                    *type = JsonToken::ObjectStart;
                    *chars_ahead = current_pos - cursor_index;;
                    return JsonTokenizer::NoError;
                case '}':
                    *type = JsonToken::ObjectEnd;
                    *chars_ahead = current_pos - cursor_index;;
                    return JsonTokenizer::NoError;
                case '[':
                    *type = JsonToken::ArrayStart;
                    *chars_ahead = current_pos - cursor_index;;
                    return JsonTokenizer::NoError;
                case ']':
                    *type = JsonToken::ArrayEnd;
                    *chars_ahead = current_pos - cursor_index;;
                    return JsonTokenizer::NoError;
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
                    *chars_ahead = current_pos - cursor_index;;
                    return JsonTokenizer::NoError;
                default:
                    char ascii_code = *(json_data.data + current_pos);
                    if ((ascii_code >= 'A' && ascii_code <= 'Z') ||
                            (ascii_code >= '^' && ascii_code <= 'z')) {
                        *type = JsonToken::Ascii;
                        *chars_ahead = current_pos - cursor_index;;
                        return JsonTokenizer::NoError;
                    } else {
                        *chars_ahead = current_pos - cursor_index;
                        return JsonTokenizer::EncounteredIlligalChar;
                    }
                    break;
            }

        }
        return JsonTokenizer::NeedMoreData;
    }

    JsonTokenizer::Error findDelimiter(const JsonData &json_data, size_t *chars_ahead)
    {
        for (size_t end = cursor_index; end < json_data.size; end++) {
            switch(*(json_data.data + end)) {
                case ':':
                    token_state = FindingData;
                    *chars_ahead = end - cursor_index;
                    return JsonTokenizer::NoError;
                case ',':
                    token_state = FindingName;
                    *chars_ahead = end - cursor_index;
                    return JsonTokenizer::NoError;
                case ']':
                    token_state = FindingName;
                    *chars_ahead = end - cursor_index - 1;
                    return JsonTokenizer::NoError;
                case ' ':
                    break;
                default:
                    return JsonTokenizer::ExpectedDelimiter;
                    break;
            }
        }
        return JsonTokenizer::NeedMoreData;
    }

    JsonTokenizer::Error findTokenEnd(const JsonData &json_data, size_t *chars_ahead)
    {
        for (size_t end = cursor_index; end < json_data.size; end++) {
            switch(*(json_data.data + end)) {
                case ',':
                    expecting_prop_or_annonymous_data = true;
                    *chars_ahead = end - cursor_index;
                    return JsonTokenizer::NoError;
                case '\n':
                    if (allow_new_lines) {
                        *chars_ahead = end - cursor_index;
                        return JsonTokenizer::NoError;
                    }
                    break;
                case ']':
                case '}':
                    *chars_ahead = end - cursor_index - 1;
                    return JsonTokenizer::NoError;
                case ' ':
                case '\0':
                    break;
                default:
                    *chars_ahead = end - cursor_index;
                    return JsonTokenizer::InvalidToken;
            }
        }
        return JsonTokenizer::NeedMoreData;
    }

    void releaseFirstJsonData()
    {
        const JsonData &json_data = data_list.front();
        if (release_data_callback) {
            release_data_callback(json_data.data, json_data.user_handle);
        }
        data_list.pop_front();
        cursor_index = 0;
    }

    JsonTokenizer::Error populateFromData(const char **data, int *length, JsonToken::Type *type, const JsonData &json_data)
    {
        size_t diff = 0;
        JsonTokenizer::Error error = JsonTokenizer::NoError;
        *length = 0;
        *data = json_data.data + cursor_index;
        if (property_state == NoStartFound) {
            JsonTokenizer::Error error = findStartOfNextValue(type, json_data, &diff);
            if (error != JsonTokenizer::NoError) {
                *type = JsonToken::Error;
                return error;
            }

            *data = json_data.data + cursor_index + diff;
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
                return JsonTokenizer::InvalidToken;
            }

            if (error != JsonTokenizer::NoError) {
                return error;
            }

            cursor_index += diff + 1;
            // + 2 because diff is last index - second index
            *length = diff + 2;
            property_state = FoundEnd;
        }

        return JsonTokenizer::NoError;
    }

    JsonTokenizer::Error populateNextTokenFromData(JsonToken *next_token, const JsonData &json_data)
    {
        while (cursor_index < json_data.size) {
            size_t diff = 0;
            const char *data;
            int data_length;
            JsonToken::Type type;
            JsonTokenizer::Error error;
            switch (token_state) {
                case FindingName:
                    error = populateFromData(&data, &data_length, &type, json_data);
                    if (error == JsonTokenizer::NeedMoreData) {
                        if (property_state > NoStartFound) {
                            intermediate_token.intermedia_set = true;
                            size_t to_null = strnlen(data , json_data.size - cursor_index);
                            intermediate_token.name.append(json_data.data + cursor_index , to_null);
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
                                return JsonTokenizer::ExpectedDataToken;
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
                            return JsonTokenizer::NoError;

                        case JsonToken::String:
                            next_token->name = data + 1;
                            next_token->name_length = data_length - 2;
                            break;
                        default:
                            next_token->name = data;
                            next_token->name_length = data_length;
                            break;
                    }

                    if (error != JsonTokenizer::NoError)
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
                    if (error != JsonTokenizer::NoError) {
                        if (intermediate_token.intermedia_set == false) {
                            intermediate_token.name.append(next_token->name, next_token->name_length);
                            intermediate_token.name_type = next_token->name_type;
                            intermediate_token.intermedia_set = true;
                        }
                        return JsonTokenizer::NeedMoreData;
                    }
                    cursor_index += diff +1;
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
                        return JsonTokenizer::NoError;
                    } else {
                        if (next_token->name_type != JsonToken::String) {
                            if (!allow_ascii_properties || next_token->name_type != JsonToken::Ascii) {
                                return JsonTokenizer::IlligalPropertyName;
                            }
                        }
                    }
                    break;

                case FindingData:
                    error = populateFromData(&data, &data_length, &type, json_data);
                    if (error == JsonTokenizer::NeedMoreData) {
                        if (intermediate_token.intermedia_set == false) {
                            intermediate_token.name.append(next_token->name, next_token->name_length);
                            intermediate_token.name_type = next_token->name_type;
                            intermediate_token.intermedia_set = true;
                        }
                        if (property_state > NoStartFound) {
                            size_t data_length = strnlen(json_data.data + cursor_index, json_data.size - cursor_index);
                            intermediate_token.data.append(json_data.data + cursor_index, data_length);
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
                        std::string test(data +1, data_length -2);
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
                    if (type == JsonToken::Ascii && !allow_ascii_properties) {
                        return JsonTokenizer::IlligalDataValue;
                    }
                    if (type == JsonToken::ObjectStart || type == JsonToken::ArrayStart)
                        token_state = FindingName;

                    if (error != JsonTokenizer::NoError)
                        return error;

                    if (next_token->data_type == JsonToken::ObjectStart
                            || next_token->data_type == JsonToken::ArrayStart) {
                        return JsonTokenizer::NoError;
                    }
                    token_state = FindingTokenEnd;
                    return JsonTokenizer::NoError;
                case FindingTokenEnd:
                    error = findTokenEnd(json_data, &diff);
                    if (error != JsonTokenizer::NoError) {
                        return error;
                    }
                    cursor_index += diff + 1;
                    token_state = FindingName;
            }
        }
        return JsonTokenizer::NeedMoreData;
    }

    std::list<JsonData> data_list;
    size_t cursor_index;
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

JsonTokenizer::Error JsonTokenizer::nextToken(JsonToken *next_token)
{
    if (!m_private->data_list.size()) {
        return NeedMoreData;
    }

    if (!m_private->continue_after_need_more_data)
        m_private->resetForNewToken();

    JsonTokenizer::Error error = NeedMoreData;
    int trying = 0;
    while (error == NeedMoreData && m_private->data_list.size()) {
        const JsonData &json_data = m_private->data_list.front();
        error = m_private->populateNextTokenFromData(next_token, json_data);

        if (error != NoError) {
            m_private->releaseFirstJsonData();
        }
    }

    m_private->continue_after_need_more_data = error == NeedMoreData;

    return error;
}
