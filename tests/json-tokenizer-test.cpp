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
#include "tokenizer-test-util.h"

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

static int check_json_with_string_and_ascii()
{
    JsonTokenizer::Error error;
    JsonTokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data, sizeof(json_data), 0);

    JsonToken token;
    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonType::ObjectStart);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(assert_token(token,JsonType::String,"foo", JsonType::String, "bar") == 0);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::String, "color", JsonType::String, "red") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "weather", JsonType::String, "clear") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "weather1", JsonType::String, "clear1") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "ToBeTrue", JsonType::Bool, "true") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "HeresANull", JsonType::Null, "null") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "ThisIsFalse", JsonType::Bool, "false") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "EscapedString", JsonType::String, "contains \\\"") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "ThisIsANumber", JsonType::Number, "3.14") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "ThisIsAnObject", JsonType::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "ThisIsASubType", JsonType::String, "red") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonType::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "AnotherProp", JsonType::String, "prop") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "ThisIsAnotherObject", JsonType::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "ThisIsAnotherASubType", JsonType::String, "blue") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonType::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "ThisIsAnArray", JsonType::ArrayStart, "[") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::String, "", JsonType::Number, "12.4") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::String, "", JsonType::Number, "3") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::String, "", JsonType::Number, "43.2") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonType::ArrayEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "ThisIsAnObjectArray", JsonType::ArrayStart, "[") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::String, "", JsonType::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "Test1", JsonType::String, "Test2") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "Test3", JsonType::String, "Test4") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonType::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::String, "", JsonType::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "Test5", JsonType::Bool, "true") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonType::Ascii, "Test7", JsonType::Bool, "false") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonType::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonType::ArrayEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonType::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NeedMoreData);

    return 0;
}

int main(int argc, char **argv)
{
    return check_json_with_string_and_ascii();
}
