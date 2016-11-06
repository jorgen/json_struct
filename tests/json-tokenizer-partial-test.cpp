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
    JT_ASSERT(token.value_type == JT::Type::ObjectEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NeedMoreData);

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
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectStart);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    std::string foo(token.name.data, token.name.size);
    JT_ASSERT(assert_token(token,JT::Type::String,"foo", JT::Type::String, "bar") == 0);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::String, "color", JT::Type::String, "red") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NeedMoreData);

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
    JT_ASSERT(token.value_type == JT::Type::ObjectEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NeedMoreData);

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
    JT_ASSERT(token.value_type == JT::Type::ObjectEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NeedMoreData);

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
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectStart);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(assert_token(token,JT::Type::String,"foo", JT::Type::String, "bar") == 0);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "color", JT::Type::String, "red") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NeedMoreData);

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
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectStart);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(assert_token(token,JT::Type::String,"foo", JT::Type::String, "bar") == 0);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "color", JT::Type::Bool, "true") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NeedMoreData);

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
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectStart);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(assert_token(token,JT::Type::String,"foo", JT::Type::String, "bar") == 0);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "color", JT::Type::Bool, "true") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NeedMoreData);

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
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectStart);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(assert_token(token,JT::Type::String,"foo", JT::Type::String, "bar") == 0);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::String, "array", JT::Type::ArrayStart, "[") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "", JT::Type::String, "one") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "", JT::Type::String, "two") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "", JT::Type::String, "three") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT((assert_token(token, JT::Type::Ascii, "", JT::Type::ArrayEnd, "]") == 0));

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectEnd);

    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NeedMoreData);

    return 0;
}

void check_remove_callback()
{
    JT::Error error = JT::Error::NoError;
    JT::Tokenizer tokenizer;
    tokenizer.allowAsciiType(true);
    tokenizer.allowNewLineAsTokenDelimiter(true);
    tokenizer.addData(json_data_partial_8_1, sizeof(json_data_partial_8_1));
    tokenizer.addData(json_data_partial_8_2, sizeof(json_data_partial_8_2));
    bool has_been_called = false;
    size_t id = tokenizer.registerNeedMoreDataCallback([&has_been_called] (const JT::Tokenizer &)
                                                       {
                                                            has_been_called = true;
                                                       });
    JT::Token token;
    error = tokenizer.nextToken(token);
    tokenizer.removeNeedMoreDataCallback(id);
    while(error == JT::Error::NoError && token.value_type != JT::Type::ObjectEnd)
        error = tokenizer.nextToken(token);

    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(has_been_called == false);

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

    check_remove_callback();

    return 0;
}
