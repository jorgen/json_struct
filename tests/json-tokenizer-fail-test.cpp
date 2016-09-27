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

const char json_with_ascii_property[] =
"{"
"   \"foo\": \"bar\","
"   color : \"red\""
"}";

static int check_fail_json_with_ascii_property()
{
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.addData(json_with_ascii_property, sizeof(json_with_ascii_property));

    JT::Token token;
    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectStart);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(assert_token(token,JT::Token::String,"\"foo\"", JT::Token::String, "\"bar\"") == 0);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::IlligalPropertyName);

    return 0;
}

const char json_with_ascii_data[] =
"{"
"   \"foo\": \"bar\","
"   \"color\": red"
"}";

static int check_fail_json_with_ascii_data()
{
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.addData(json_with_ascii_data, sizeof(json_with_ascii_data));

    JT::Token token;
    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectStart);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(assert_token(token,JT::Token::String,"\"foo\"", JT::Token::String, "\"bar\"") == 0);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::IlligalDataValue);

    return 0;
}

const char json_with_new_line_seperator[] =
"{"
"   \"foo\": \"bar\"\n"
"   \"color\" : \"red\""
"}";

static int check_fail_json_with_new_line_seperator()
{
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.addData(json_with_new_line_seperator, sizeof(json_with_new_line_seperator));

    JT::Token token;
    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectStart);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(assert_token(token,JT::Token::String,"\"foo\"", JT::Token::String, "\"bar\"") == 0);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::InvalidToken);

    return 0;
}

const char json_with_comma_before_obj_end[] =
"{"
"   \"foo\": \"bar\","
"   \"color\" : \"red\","
"}";

static int check_fail_json_with_comma_before_obj_end()
{
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.addData(json_with_comma_before_obj_end, sizeof(json_with_comma_before_obj_end));

    JT::Token token;
    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectStart);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(assert_token(token,JT::Token::String,"\"foo\"", JT::Token::String, "\"bar\"") == 0);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(assert_token(token,JT::Token::String,"\"color\"", JT::Token::String, "\"red\"") == 0);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::ExpectedDataToken);

    return 0;
}

const char json_with_illigal_chars[] =
"{"
"   \"foo\": \"bar\","
" ,  \"color\" : \"red\","
"}";

static int check_fail_json_with_illigal_chars()
{
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.addData(json_with_illigal_chars, sizeof(json_with_illigal_chars));

    JT::Token token;
    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectStart);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(assert_token(token,JT::Token::String,"\"foo\"", JT::Token::String, "\"bar\"") == 0);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::EncounteredIlligalChar);

    return 0;
}



int main(int, char **)
{
    check_fail_json_with_ascii_property();
    check_fail_json_with_ascii_data();
    check_fail_json_with_new_line_seperator();
    check_fail_json_with_comma_before_obj_end();
    check_fail_json_with_illigal_chars();

    return 0;
}

