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

const char json_data_partial_1_1[] =
"{";
const char json_data_partial_1_2[] =
"   \"foo\": \"bar\","
"   \"color\" : \"red\"\n"
"}";

static int check_json_partial_1()
{
    JsonTokenizer::Error error;
    JsonTokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_1_1, sizeof(json_data_partial_1_1), 0);
    tokenizer.addData(json_data_partial_1_2, sizeof(json_data_partial_1_2), 0);

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
    assert(token.data_type == JsonToken::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NeedMoreData);

    return 0;
}

const char json_data_partial_2_1[] =
"{  \"fo";
const char json_data_partial_2_2[] =
"o\": \"bar\","
"   \"color\" : \"red\"\n"
"}";

static int check_json_partial_2()
{
    JsonTokenizer::Error error;
    JsonTokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_2_1, sizeof(json_data_partial_2_1), 0);
    tokenizer.addData(json_data_partial_2_2, sizeof(json_data_partial_2_2), 0);

    JsonToken token;
    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ObjectStart);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    std::string foo(token.name, token.name_length);
    fprintf(stderr, "token name %s\n", foo.c_str());
    assert(assert_token(token,JsonToken::String,"foo", JsonToken::String, "bar") == 0);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::String, "color", JsonToken::String, "red") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NeedMoreData);

    return 0;
}

const char json_data_partial_3_1[] =
"{  \"foo\"";
const char json_data_partial_3_2[] =
": \"bar\","
"   \"color\" : \"red\"\n"
"}";

static int check_json_partial_3()
{
    JsonTokenizer::Error error;
    JsonTokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_3_1, sizeof(json_data_partial_3_1), 0);
    tokenizer.addData(json_data_partial_3_2, sizeof(json_data_partial_3_2), 0);

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
    assert(token.data_type == JsonToken::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NeedMoreData);

    return 0;
}

const char json_data_partial_4_1[] =
"{  \"foo\": \"bar\"";
const char json_data_partial_4_2[] =
","
"   \"color\" : \"red\"\n"
"}";

static int check_json_partial_4()
{
    JsonTokenizer::Error error;
    JsonTokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_4_1, sizeof(json_data_partial_4_1), 0);
    tokenizer.addData(json_data_partial_4_2, sizeof(json_data_partial_4_2), 0);

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
    assert(token.data_type == JsonToken::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NeedMoreData);

    return 0;
}

const char json_data_partial_5_1[] =
"{  \"foo\": \"bar\","
"   col";
const char json_data_partial_5_2[] =
"or : \"red\"\n"
"}";

static int check_json_partial_5()
{
    JsonTokenizer::Error error;
    JsonTokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_5_1, sizeof(json_data_partial_5_1), 0);
    tokenizer.addData(json_data_partial_5_2, sizeof(json_data_partial_5_2), 0);

    JsonToken token;
    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ObjectStart);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(assert_token(token,JsonToken::String,"foo", JsonToken::String, "bar") == 0);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "color", JsonToken::String, "red") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NeedMoreData);

    return 0;
}

const char json_data_partial_6_1[] =
"{  \"foo\": \"bar\","
"   color : tr";
const char json_data_partial_6_2[] =
"ue"
"}";

static int check_json_partial_6()
{
    JsonTokenizer::Error error;
    JsonTokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_6_1, sizeof(json_data_partial_6_1), 0);
    tokenizer.addData(json_data_partial_6_2, sizeof(json_data_partial_6_2), 0);

    JsonToken token;
    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ObjectStart);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(assert_token(token,JsonToken::String,"foo", JsonToken::String, "bar") == 0);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "color", JsonToken::Bool, "true") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NeedMoreData);

    return 0;
}

const char json_data_partial_7_1[] =
"{  \"foo\": \"bar\","
"   color : true";
const char json_data_partial_7_2[] =
"}";

static int check_json_partial_7()
{
    JsonTokenizer::Error error;
    JsonTokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_7_1, sizeof(json_data_partial_7_1), 0);
    tokenizer.addData(json_data_partial_7_2, sizeof(json_data_partial_7_2), 0);

    JsonToken token;
    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ObjectStart);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(assert_token(token,JsonToken::String,"foo", JsonToken::String, "bar") == 0);

    error = tokenizer.nextToken(&token);
    fprintf(stderr, "Test %d\n", error);
    assert(error == JsonTokenizer::NoError);
    assert((assert_token(token, JsonToken::Ascii, "color", JsonToken::Bool, "true") == 0));

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NoError);
    assert(token.data_type == JsonToken::ObjectEnd);

    error = tokenizer.nextToken(&token);
    assert(error == JsonTokenizer::NeedMoreData);

    return 0;
}

int main(int argc, char **argv)
{
    check_json_partial_1();
    check_json_partial_2();
    check_json_partial_3();
    check_json_partial_4();
    check_json_partial_5();
    check_json_partial_6();
    check_json_partial_7();
    return 0;
}
