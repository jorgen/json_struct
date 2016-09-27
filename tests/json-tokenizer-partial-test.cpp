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

const char json_data_partial_1_1[] =
"{";
const char json_data_partial_1_2[] =
"   \"foo\": \"bar\","
"   \"color\" : \"red\"\n"
"}";

static int check_json_partial_1()
{
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_1_1, sizeof(json_data_partial_1_1));
    tokenizer.addData(json_data_partial_1_2, sizeof(json_data_partial_1_2));

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
    assert(token.value_type == JT::Token::ObjectEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NeedMoreData);

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
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_2_1, sizeof(json_data_partial_2_1));
    tokenizer.addData(json_data_partial_2_2, sizeof(json_data_partial_2_2));

    JT::Token token;
    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectStart);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    std::string foo(token.name.data, token.name.size);
    assert(assert_token(token,JT::Token::String,"\"foo\"", JT::Token::String, "\"bar\"") == 0);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::String, "\"color\"", JT::Token::String, "\"red\"") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NeedMoreData);

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
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_3_1, sizeof(json_data_partial_3_1));
    tokenizer.addData(json_data_partial_3_2, sizeof(json_data_partial_3_2));

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
    assert(token.value_type == JT::Token::ObjectEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NeedMoreData);

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
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_4_1, sizeof(json_data_partial_4_1));
    tokenizer.addData(json_data_partial_4_2, sizeof(json_data_partial_4_2));

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
    assert(token.value_type == JT::Token::ObjectEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NeedMoreData);

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
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_5_1, sizeof(json_data_partial_5_1));
    tokenizer.addData(json_data_partial_5_2, sizeof(json_data_partial_5_2));

    JT::Token token;
    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectStart);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(assert_token(token,JT::Token::String,"\"foo\"", JT::Token::String, "\"bar\"") == 0);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "color", JT::Token::String, "\"red\"") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NeedMoreData);

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
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_6_1, sizeof(json_data_partial_6_1));
    tokenizer.addData(json_data_partial_6_2, sizeof(json_data_partial_6_2));

    JT::Token token;
    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectStart);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(assert_token(token,JT::Token::String,"\"foo\"", JT::Token::String, "\"bar\"") == 0);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "color", JT::Token::Bool, "true") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NeedMoreData);

    return 0;
}

const char json_data_partial_7_1[] =
"{  \"foo\": \"bar\","
"   color : true";
const char json_data_partial_7_2[] =
"}";

static int check_json_partial_7()
{
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_7_1, sizeof(json_data_partial_7_1));
    tokenizer.addData(json_data_partial_7_2, sizeof(json_data_partial_7_2));

    JT::Token token;
    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectStart);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(assert_token(token,JT::Token::String,"\"foo\"", JT::Token::String, "\"bar\"") == 0);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "color", JT::Token::Bool, "true") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NeedMoreData);

    return 0;
}

const char json_data_partial_8_1[] =
"{  \"foo\": \"bar\","
"   \"array\": ["
"       \"one\","
"       \"two\",";
const char json_data_partial_8_2[] =
"       \"three\""
"    ]"
"}";

static int check_json_partial_8()
{
    JT::Error error;
    JT::Tokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_8_1, sizeof(json_data_partial_8_1));
    tokenizer.addData(json_data_partial_8_2, sizeof(json_data_partial_8_2));

    JT::Token token;
    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectStart);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(assert_token(token,JT::Token::String,"\"foo\"", JT::Token::String, "\"bar\"") == 0);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::String, "\"array\"", JT::Token::ArrayStart, "[") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "", JT::Token::String, "\"one\"") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "", JT::Token::String, "\"two\"") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "", JT::Token::String, "\"three\"") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert((assert_token(token, JT::Token::Ascii, "", JT::Token::ArrayEnd, "]") == 0));

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NoError);
    assert(token.value_type == JT::Token::ObjectEnd);

    error = tokenizer.nextToken(token);
    assert(error == JT::Error::NeedMoreData);

    return 0;
}

int main(int, char **)
{
    check_json_partial_1();
    check_json_partial_2();
    check_json_partial_3();
    check_json_partial_4();
    check_json_partial_5();
    check_json_partial_6();
    check_json_partial_7();
    check_json_partial_8();

    return 0;
}
