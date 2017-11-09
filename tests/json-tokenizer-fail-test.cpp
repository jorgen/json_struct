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

#include "assert.h"

const char json_with_ascii_property[] =
"{"
"   \"foo\": \"bar\","
"   color : \"red\""
"}";

static int check_fail_json_with_ascii_property()
{
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.addData(json_with_ascii_property);

    JT::Token token;
    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectStart);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(assert_token(token,JT::Type::String,"foo", JT::Type::String, "bar") == 0);

    error = tokenizer.nextToken(token);
    fprintf(stderr, "Error %s\n", tokenizer.makeErrorString().c_str());
    JT_ASSERT(error == JT::Error::IllegalPropertyName);

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
    tokenizer.addData(json_with_ascii_data);

    JT::Token token;
    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectStart);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(assert_token(token,JT::Type::String,"foo", JT::Type::String, "bar") == 0);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::IllegalDataValue);

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
    tokenizer.addData(json_with_new_line_seperator);

    JT::Token token;
    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectStart);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(assert_token(token,JT::Type::String,"foo", JT::Type::String, "bar") == 0);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::InvalidToken);

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
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectStart);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(assert_token(token,JT::Type::String,"foo", JT::Type::String, "bar") == 0);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(assert_token(token,JT::Type::String,"color", JT::Type::String, "red") == 0);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::ExpectedDataToken);

    return 0;
}

const char json_with_illegal_chars[] =
"{"
"   \"foo\": \"bar\","
" ,  \"color\" : \"red\","
"}";

static int check_fail_json_with_illegal_chars()
{
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.addData(json_with_illegal_chars, sizeof(json_with_illegal_chars));

    JT::Token token;
    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectStart);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(assert_token(token,JT::Type::String,"foo", JT::Type::String, "bar") == 0);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::EncounteredIllegalChar);

    return 0;
}

const char json_with_illegal_comma_in_array[] =
"{"
"    \"foo\": [,4,5,6]"
"}";
static int check_fail_json_with_empty_array()
{
	JT::Error error;
	JT::Tokenizer tokenizer;
	tokenizer.addData(json_with_illegal_comma_in_array);

	JT::Token token;
	error = tokenizer.nextToken(token);
	JT_ASSERT(error == JT::Error::NoError);
	JT_ASSERT(token.value_type == JT::Type::ObjectStart);

	error = tokenizer.nextToken(token);
	JT_ASSERT(error == JT::Error::NoError);
	JT_ASSERT(token.value_type == JT::Type::ArrayStart);

	error = tokenizer.nextToken(token);
	JT_ASSERT(error == JT::Error::EncounteredIllegalChar);

	return 0;
}


int main(int, char **)
{
    check_fail_json_with_ascii_property();
    check_fail_json_with_ascii_data();
    check_fail_json_with_new_line_seperator();
    check_fail_json_with_comma_before_obj_end();
    check_fail_json_with_illegal_chars();
	check_fail_json_with_empty_array();
    return 0;
}

