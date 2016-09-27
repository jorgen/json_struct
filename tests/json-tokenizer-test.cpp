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

#include "json_tools.h"
#include "tokenizer-test-util.h"
#include "json-test-data.h"

static int check_json_with_string_and_ascii()
{
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data1, sizeof(json_data1));

    JT::Token token;
    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectStart);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(assert_token(token,JT::Token::String,"\"foo\"", JT::Token::String, "\"bar\"") == 0);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::String, "\"color\"", JT::Token::String, "\"red\"") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "weather", JT::Token::String, "\"clear\"") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "weather1", JT::Token::String, "\"clear1\"") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "ToBeTrue", JT::Token::Bool, "true") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "HeresANull", JT::Token::Null, "null") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "ThisIsFalse", JT::Token::Bool, "false") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "EscapedString", JT::Token::String, "\"contains \\\"\"") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "ThisIsANumber", JT::Token::Number, "3.14") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "ThisIsAnObject", JT::Token::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "ThisIsASubType", JT::Token::String, "\"red\"") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "AnotherProp", JT::Token::String, "\"prop\"") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "ThisIsAnotherObject", JT::Token::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "ThisIsAnotherASubType", JT::Token::String, "\"blue\"") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "ThisIsAnArray", JT::Token::ArrayStart, "[") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "", JT::Token::Number, "12.4") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "", JT::Token::Number, "3") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "", JT::Token::Number, "43.2") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ArrayEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "ThisIsAnObjectArray", JT::Token::ArrayStart, "[") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "", JT::Token::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "Test1", JT::Token::String, "\"Test2\"") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "Test3", JT::Token::String, "\"Test4\"") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "", JT::Token::ObjectStart, "{") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "Test5", JT::Token::Bool, "true") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "Test7", JT::Token::Bool, "false") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ArrayEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NeedMoreData);

    return 0;
}

int main(int, char **)
{
    return check_json_with_string_and_ascii();
}
