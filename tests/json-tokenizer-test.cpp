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

#include "json_tools.h"
#include "tokenizer-test-util.h"
#include "json-test-data.h"

#include "assert.h"

static int check_json_with_string_and_ascii()
{
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data1, sizeof(json_data1));

    JT::Token token;
    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectStart);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(assert_token(token,JT::Type::String,"foo", JT::Type::String, "bar") == 0);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::String, "color", JT::Type::String, "red") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "weather", JT::Type::String, "clear") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "weather1", JT::Type::String, "clear1") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "ToBeTrue", JT::Type::Bool, "true") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "HeresANull", JT::Type::Null, "null") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "ThisIsFalse", JT::Type::Bool, "false") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "EscapedString", JT::Type::String, "contains \\\"") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::String, "\\\"EscapedName\\\"", JT::Type::Bool, "true") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::String, "EscapedProp", JT::Type::String, "\\\"Hello\\\"") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "ThisIsANumber", JT::Type::Number, "3.14") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "ThisIsAnObject", JT::Type::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "ThisIsASubType", JT::Type::String, "red") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "AnotherProp", JT::Type::String, "prop") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "ThisIsAnotherObject", JT::Type::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "ThisIsAnotherASubType", JT::Type::String, "blue") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "ThisIsAnArray", JT::Type::ArrayStart, "[") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "", JT::Type::Number, "12.4") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "", JT::Type::Number, "3") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "", JT::Type::Number, "43.2") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ArrayEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "ThisIsAnObjectArray", JT::Type::ArrayStart, "[") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "", JT::Type::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "Test1", JT::Type::String, "Test2") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "Test3", JT::Type::String, "Test4") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "", JT::Type::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "Test5", JT::Type::Bool, "true") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "Test7", JT::Type::Bool, "false") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ArrayEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NeedMoreData);

    return 0;
}

int main(int, char **)
{
    return check_json_with_string_and_ascii();
}
