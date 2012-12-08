/*
 * Copyright © 2012 Jorgen Lind
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifdef NDEBUG
#error "These tests uses assert. Please remove define NDEBUG"
#endif

#include "json_tokenizer.h"
#include <assert.h>
#include <string>

const char json_data[] =
"{"
"   \"foo\": \"bar\","
"   \"color\" : \"red\"\n"
"   weather: \"clear\"\n"
"   weather1 : \"clear1\"\n"
"   ToBeTrue: true,"
"   HeresANull : null\n"
"   ThisIsFalse: false,\n\n"
"   EscapedString: \"contains \\\"\","
"   ThisIsANumber: 3.14\n"
"   ThisIsAnObject: {"
"       ThisIsASubType: \"red\""
"   },"
"   AnotherProp: \"prop\"\n"
"   ThisIsAnotherObject: {"
"       ThisIsAnotherASubType: \"blue\""
"   },"
"   ThisIsAnArray: ["
"       12.4,"
"       3,"
"       43.2"
"   ]\n"
"   ThisIsAnObjectArray: ["
"       { Test1: \"Test2\", Test3: \"Test4\" },"
"       { Test5: true, Test7: false }"
"   ]"
"}";

static int assert_token(const JsonToken &token, JsonToken::Type name_type, std::string property, JsonToken::Type data_type, std::string data)
{
    if (token.name_type != name_type) {
        fprintf(stderr, "token.name_type is: %d, expected %d\n", token.name_type, name_type);
        return -1;
    }

    if (token.name_length != property.size()) {
        fprintf(stderr, "token.name_length is: %lu, expected: %lu\n", token.name_length, property.size());
        return -1;
    }
    std::string token_property(token.name, token.name_length);
    if (property.compare(token_property) != 0) {
        std::string name(token.name, token.name_length);
        fprintf(stderr, "token.property: %s is unequal to %s\n", name.c_str(), property.c_str());
        return -1;
    }

    if (token.data_type != data_type) {
        fprintf(stderr, "token.data_type is: %d, expected %d\n", token.data_type, data_type);
        return -1;
    }

    std::string token_data(token.data, token.data_length);
    if (data.compare(token_data) != 0) {
        std::string data_name(token.data, token.data_length);
        fprintf(stderr, "token.data: %s is unequal to %s\n", data_name.c_str(), data.c_str());
        return -1;
    }

    if (token.data_length != data.size()) {
        fprintf(stderr, "token.data_length is: %lu, expected: %lu\n", token.data_length, data.size());
        return -1;
    }
return 0;
}


static int check_json_with_string_and_ascii()
{
    JsonTokenizer::Error error;
    JsonTokenizer tokenizer;
    tokenizer.addData(json_data, sizeof(json_data), 0);

    JsonToken token;
    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ObjectStart);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(assert_token(token,JsonToken::String,"foo", JsonToken::String, "bar") == 0);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::String, "color", JsonToken::String, "red") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "weather", JsonToken::String, "clear") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "weather1", JsonToken::String, "clear1") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "ToBeTrue", JsonToken::Bool, "true") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "HeresANull", JsonToken::Null, "null") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "ThisIsFalse", JsonToken::Bool, "false") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "EscapedString", JsonToken::String, "contains \\\"") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "ThisIsANumber", JsonToken::Number, "3.14") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "ThisIsAnObject", JsonToken::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "ThisIsASubType", JsonToken::String, "red") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "AnotherProp", JsonToken::String, "prop") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "ThisIsAnotherObject", JsonToken::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "ThisIsAnotherASubType", JsonToken::String, "blue") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "ThisIsAnArray", JsonToken::ArrayStart, "[") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::String, "", JsonToken::Number, "12.4") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::String, "", JsonToken::Number, "3") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::String, "", JsonToken::Number, "43.2") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ArrayEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "ThisIsAnObjectArray", JsonToken::ArrayStart, "[") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::String, "", JsonToken::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "Test1", JsonToken::String, "Test2") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "Test3", JsonToken::String, "Test4") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ObjectEnd);

    error = tokenizer.nextToken(&token);
    fprintf(stderr, "%d\n", error);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::String, "", JsonToken::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "Test5", JsonToken::Bool, "true") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "Test7", JsonToken::Bool, "false") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ArrayEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NeedMoreData);

    return 0;
}

int main(int argc, char **argv)
{
    return check_json_with_string_and_ascii();
}
