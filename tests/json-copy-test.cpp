/*
 * Copyright © 2016 Jorgen Lind
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

#include "assert.h"

const char json[] =
"{\n"
"    \"property_one\" : 432432,\n"
"    \"execute_one\" : {\n"
"        \"number\" : 45,\n"
"        \"valid\" : \"false\"\n"
"    },"
"    \"execute_two\" : 99,\n"
"    \"execute_three\" : [\n"
"        4,\n"
"        6,\n"
"        8\n"
"    ]\n"
"}\n";

struct SubObject
{
    int number;
    bool valid;

    JT_STRUCT(JT_MEMBER(number),
              JT_MEMBER(valid));

};
void jt_validate_json(JT::Tokenizer &tokenizer)
{
    JT::Token token;;
    JT::Error error;
    std::string buffer;
    
    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectStart);
    
    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::Number);
    
    error = tokenizer.nextToken(token);
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectStart);
    tokenizer.copyFromValue(token, buffer);
    
    while(error == JT::Error::NoError && token.value_type != JT::Type::ObjectEnd)
        error = tokenizer.nextToken(token);
    
    JT_ASSERT(error == JT::Error::NoError);
    JT_ASSERT(token.value_type == JT::Type::ObjectEnd);
    tokenizer.copyIncludingValue(token, buffer);
    
    while(error == JT::Error::NoError && token.value_type != JT::Type::ObjectEnd)
        error = tokenizer.nextToken(token);
    
    JT::ParseContext context = JT::makeParseContextForData(buffer.c_str(), buffer.size());
    fprintf(stderr, "buffer %s\n", buffer.c_str());
    SubObject subObj;
    JT::parseData(subObj, context);
    
    JT_ASSERT(context.error == JT::Error::NoError);
    JT_ASSERT(subObj.number == 45);
    JT_ASSERT(subObj.valid == false);
}
void jt_copy_full()
{
    JT::Tokenizer tokenizer;
    tokenizer.addData(json);
    jt_validate_json(tokenizer);
}

void jt_partial_1()
{
    JT::Tokenizer tokenizer;
    tokenizer.addData(json,40);
    tokenizer.addData(json + 40, sizeof(json) - 40);
    jt_validate_json(tokenizer);
}

void jt_partial_2()
{
    JT::Tokenizer tokenizer;
    size_t offset = 0;
    std::function<void (JT::Tokenizer &)> func = [&offset, &func] (JT::Tokenizer &tokenizer)
    {
        if (offset + 2 > sizeof(json)) {
            tokenizer.addData(json + offset, sizeof(json) - offset);
            offset += sizeof(json) - offset;
        } else {
            tokenizer.addData(json + offset, 2);
            offset += 2;
        }
        tokenizer.registerNeedMoreDataCallback(func);
    };
    tokenizer.registerNeedMoreDataCallback(func);

    jt_validate_json(tokenizer);
}

void jt_partial_3()
{
    JT::Tokenizer tokenizer;
    size_t offset = 0;
    std::function<void (JT::Tokenizer &)> func = [&offset, &func] (JT::Tokenizer &tokenizer)
    {
        if (offset + 1 > sizeof(json)) {
            tokenizer.addData(json + offset, sizeof(json) - offset);
            offset += sizeof(json) - offset;
        } else {
            tokenizer.addData(json + offset, 1);
            offset += 1;
        }
        tokenizer.registerNeedMoreDataCallback(func);
    };
    tokenizer.registerNeedMoreDataCallback(func);
    
    jt_validate_json(tokenizer);
}

int main()
{
    jt_copy_full();
    jt_partial_1();
    jt_partial_2();
    jt_partial_3();
    
    return 0;
}
