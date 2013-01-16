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
    JsonError error;
    JsonTokenizer tokenizer;
    tokenizer.addData(json_with_ascii_property, sizeof(json_with_ascii_property), 0);

    JsonToken token;
    error = tokenizer.nextToken(&token);
    assert(error == JsonError::NoError);
    assert(token.data_type == JsonToken::ObjectStart);

    error = tokenizer.nextToken(&token);
    assert(error == JsonError::NoError);
    assert(assert_token(token,JsonToken::String,"foo", JsonToken::String, "bar") == 0);

    error = tokenizer.nextToken(&token);
    assert(error == JsonError::IlligalPropertyName);

    return 0;
}

const char json_with_ascii_data[] =
"{"
"   \"foo\": \"bar\","
"   \"color\": red"
"}";

static int check_fail_json_with_ascii_data()
{
    JsonError error;
    JsonTokenizer tokenizer;
    tokenizer.addData(json_with_ascii_data, sizeof(json_with_ascii_data), 0);

    JsonToken token;
    error = tokenizer.nextToken(&token);
    assert(error == JsonError::NoError);
    assert(token.data_type == JsonToken::ObjectStart);

    error = tokenizer.nextToken(&token);
    assert(error == JsonError::NoError);
    assert(assert_token(token,JsonToken::String,"foo", JsonToken::String, "bar") == 0);

    error = tokenizer.nextToken(&token);
    assert(error == JsonError::IlligalDataValue);

    return 0;
}

const char json_with_new_line_seperator[] =
"{"
"   \"foo\": \"bar\"\n"
"   \"color\" : \"red\""
"}";

static int check_fail_json_with_new_line_seperator()
{
    JsonError error;
    JsonTokenizer tokenizer;
    tokenizer.addData(json_with_new_line_seperator, sizeof(json_with_new_line_seperator), 0);

    JsonToken token;
    error = tokenizer.nextToken(&token);
    assert(error == JsonError::NoError);
    assert(token.data_type == JsonToken::ObjectStart);

    error = tokenizer.nextToken(&token);
    assert(error == JsonError::NoError);
    assert(assert_token(token,JsonToken::String,"foo", JsonToken::String, "bar") == 0);

    error = tokenizer.nextToken(&token);
    assert(error == JsonError::InvalidToken);

    return 0;
}

const char json_with_comma_before_obj_end[] =
"{"
"   \"foo\": \"bar\","
"   \"color\" : \"red\","
"}";

static int check_fail_json_with_comma_before_obj_end()
{
    JsonError error;
    JsonTokenizer tokenizer;
    tokenizer.addData(json_with_comma_before_obj_end, sizeof(json_with_comma_before_obj_end), 0);

    JsonToken token;
    error = tokenizer.nextToken(&token);
    assert(error == JsonError::NoError);
    assert(token.data_type == JsonToken::ObjectStart);

    error = tokenizer.nextToken(&token);
    assert(error == JsonError::NoError);
    assert(assert_token(token,JsonToken::String,"foo", JsonToken::String, "bar") == 0);

    error = tokenizer.nextToken(&token);
    assert(error == JsonError::NoError);
    assert(assert_token(token,JsonToken::String,"color", JsonToken::String, "red") == 0);

    error = tokenizer.nextToken(&token);
    assert(error == JsonError::ExpectedDataToken);

    return 0;
}

const char json_with_illigal_chars[] =
"{"
"   \"foo\": \"bar\","
" ,  \"color\" : \"red\","
"}";

static int check_fail_json_with_illigal_chars()
{
    JsonError error;
    JsonTokenizer tokenizer;
    tokenizer.addData(json_with_illigal_chars, sizeof(json_with_illigal_chars), 0);

    JsonToken token;
    error = tokenizer.nextToken(&token);
    assert(error == JsonError::NoError);
    assert(token.data_type == JsonToken::ObjectStart);

    error = tokenizer.nextToken(&token);
    assert(error == JsonError::NoError);
    assert(assert_token(token,JsonToken::String,"foo", JsonToken::String, "bar") == 0);

    error = tokenizer.nextToken(&token);
    assert(error == JsonError::EncounteredIlligalChar);

    return 0;
}



int main(int argc, char **argv)
{
    check_fail_json_with_ascii_property();
    check_fail_json_with_ascii_data();
    check_fail_json_with_new_line_seperator();
    check_fail_json_with_comma_before_obj_end();
    check_fail_json_with_illigal_chars();

    return 0;
}

