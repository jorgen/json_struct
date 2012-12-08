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
    size_t size;
    void *user_handle;
};

struct JsonIntermediateToken
{
    JsonIntermediateToken()
        : intermedia_set(false)
    { }

    void clear() {
        intermedia_set = false;
        name.clear();
        value.clear();
    }

    bool intermedia_set;
    std::string name;
    std::string value;
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
        , allow_ascii_properties(true)
        , allow_new_lines(true)
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

    size_t findStringEnd(const JsonData &json_data)
    {
        for (int i = cursor_index; i < json_data.size; i++) {
            if (is_escaped) {
                is_escaped = false;
                continue;
            }
            switch (*(json_data.data + i)) {
                case '\\':
                    is_escaped = true;
                    break;
                case '"':
                    return i;
                default:
                    break;
            }
        }
        return -1;
    }

    int findAsciiEnd(const JsonData &json_data)
    {
        assert(property_type == JsonToken::Ascii);
        for (int i = cursor_index; i < json_data.size; i++) {
            char ascii_code = *(json_data.data + i);
            if ((ascii_code >= 'A' && ascii_code <= 'Z') ||
                    (ascii_code >= '^' && ascii_code <= 'z') ||
                    (ascii_code >= '0' && ascii_code <= '9')) {
                continue;
            } else {
                return i;
            }
        }
        return -1;
    }

    int findNumberEnd(const JsonData &json_data)
    {
        for (int i = cursor_index; i < json_data.size; i++) {
            char number_code = *(json_data.data + i);
            if ((number_code >= '0' && number_code <= '9'))
                continue;
            switch(number_code) {
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
                    continue;
                case '.':
                case '+':
                case '-':
                case 'e':
                case 'E':
                    continue;
                default:
                    return i;
            }
        }
        return -1;
    }

    int findStartOfNextValue(JsonToken::Type *type, const JsonData &json_data)
    {

        assert(property_state == NoStartFound);

        for (int i = cursor_index; i < json_data.size; i++) {
            switch (*(json_data.data + i)) {
                case ' ':
                    continue;
                case '"':
                    *type = JsonToken::String;
                    return i;
                case '{':
                    *type = JsonToken::ObjectStart;
                    return i;
                case '}':
                    *type = JsonToken::ObjectEnd;
                    return i;
                case '[':
                    *type = JsonToken::ArrayStart;
                    return i;
                case ']':
                    *type = JsonToken::ArrayEnd;
                    return i;
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
                    return i;
                default:
                    char ascii_code = *(json_data.data + i);
                    if ((ascii_code >= 'A' && ascii_code <= 'Z') ||
                            (ascii_code >= '^' && ascii_code <= 'z')) {
                        *type = JsonToken::Ascii;
                        return i;
                    }
                    break;
            }

        }
        return -1;
    }

    size_t findDelimiter(const JsonData &json_data)
    {
        for (int i = cursor_index; i < json_data.size; i++) {
            switch(*(json_data.data + i)) {
                case ':':
                    token_state = FindingData;
                    return i;
                case ',':
                case ']':
                    token_state = FindingName;
                    return i;
                default:
                    break;
            }
        }
        return -1;
    }

    int findTokenEnd(const JsonData &json_data)
    {
        for (int i = cursor_index; i < json_data.size; i++) {
            switch(*(json_data.data + i)) {
                case ',':
                    return i + 1;
                case '\n':
                    if (allow_new_lines)
                        return i + 1;
                    break;
                case '}':
                case ']':
                    return i;
                case ' ':
                case '\0':
                    break;
                default:
                    fprintf(stderr, "Expecting end of pair. Valid chars are ' ' ',' '\\n' '}' ']'. Found '%d'\n", *(json_data.data + i));
                    return -2;
            }
        }
        return -1;
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
        int new_index_pos = 0;
        *length = 0;
        if (property_state == NoStartFound) {
            new_index_pos = findStartOfNextValue(type, json_data);
            if (new_index_pos < 0) {
                return JsonTokenizer::NeedMoreData;
            }
            *data = json_data.data + new_index_pos;
            cursor_index = new_index_pos + 1;
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
                new_index_pos = findStringEnd(json_data);
                break;
            case JsonToken::Ascii:
                new_index_pos = findAsciiEnd(json_data) - 1;
                break;
            case JsonToken::Number:
                new_index_pos = findNumberEnd(json_data) - 1;
                break;
            default:
                fprintf(stderr, "Parsing error, unknown property name "
                        "type determined\n");
                return JsonTokenizer::InvalidToken;
            }

            if (new_index_pos < 0) {
                return JsonTokenizer::NeedMoreData;
            }

            //+1 since we do an inclusive range, ie. [start, end]
            *length = new_index_pos - (cursor_index - 1) + 1;
            cursor_index = new_index_pos + 1;
            property_state = FoundEnd;
        }

        return JsonTokenizer::NoError;
    }

    JsonTokenizer::Error populateNextTokenFromData(JsonToken *next_token, const JsonData &json_data)
    {
        while (cursor_index < json_data.size) {
            size_t new_index_pos = cursor_index;
            const char *data;
            int data_length;
            JsonToken::Type type;
            JsonTokenizer::Error error;
            switch (token_state) {
                case FindingName:
                        error = populateFromData(&data, &data_length, &type, json_data);
                    if ((error == JsonTokenizer::NeedMoreData && property_state > NoStartFound)
                            || intermediate_token.intermedia_set) {
                        intermediate_token.intermedia_set = true;
                        intermediate_token.name.append(json_data.data + cursor_index, json_data.size - cursor_index);
                    } else if (error == JsonTokenizer::NoError)  {
                        switch (type) {
                            case JsonToken::ObjectStart:
                            case JsonToken::ObjectEnd:
                            case JsonToken::ArrayStart:
                            case JsonToken::ArrayEnd:
                                next_token->name = "";
                                next_token->name_length = 0;
                                next_token->name_type = JsonToken::String;
                                next_token->data = data;
                                next_token->data_length = data_length;
                                next_token->data_type = type;
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
                    new_index_pos = findDelimiter(json_data);
                    if (new_index_pos < 0)
                        return JsonTokenizer::NeedMoreData;
                    cursor_index = new_index_pos;
                    resetForNewValue();
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
                    if ((error == JsonTokenizer::NeedMoreData && property_state > NoStartFound)
                            || intermediate_token.intermedia_set) {
                        intermediate_token.intermedia_set = true;
                        intermediate_token.value.append(json_data.data + cursor_index, json_data.size - cursor_index);
                    } else if (error == JsonTokenizer::NoError)  {
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
                        if (type == JsonToken::ObjectStart || type == JsonToken::ArrayStart)
                            token_state = FindingName;
                    }

                    if (error != JsonTokenizer::NoError)
                        return error;

                    if (next_token->data_type == JsonToken::ObjectStart
                            || next_token->data_type == JsonToken::ArrayStart) {
                        return JsonTokenizer::NoError;
                    }
                    token_state = FindingTokenEnd;
                    return JsonTokenizer::NoError;
                case FindingTokenEnd:
                    new_index_pos = findTokenEnd(json_data);
                    if (new_index_pos < 0) {
                        return new_index_pos == -1 ?
                            JsonTokenizer::NeedMoreData : JsonTokenizer::InvalidToken;
                    }
                    cursor_index = new_index_pos;
                    token_state = FindingName;
            }
        }
        return JsonTokenizer::NeedMoreData;
    }

    std::list<JsonData> data_list;
    int cursor_index;
    InTokenState token_state;
    InPropertyState property_state;
    JsonToken::Type property_type;
    AsciiState ascii_state;
    bool is_escaped;
    bool allow_ascii_properties;
    bool allow_new_lines;
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

JsonTokenizer::Error JsonTokenizer::nextToken(JsonToken *next_token)
{
    if (!m_private->data_list.size()) {
        return NeedMoreData;
    }

    JsonTokenizer::Error error = NeedMoreData;
    while (error == NeedMoreData && m_private->data_list.size()) {
        const JsonData &json_data = m_private->data_list.front();
        error = m_private->populateNextTokenFromData(next_token, json_data);

        if (error != NoError) {
            m_private->releaseFirstJsonData();
        }
    }

    if (error == NoError) {
        m_private->resetForNewToken();
    }

    return error;
}
