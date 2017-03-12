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
    
    JT::ParseContext context(buffer.c_str(), buffer.size());
    fprintf(stderr, "buffer %s\n", buffer.c_str());
    SubObject subObj;
    context.parseTo(subObj);
    
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
    std::function<void (JT::Tokenizer &)> func = [&offset, &func] (JT::Tokenizer &tok)
    {
        if (offset + 2 > sizeof(json)) {
            tok.addData(json + offset, sizeof(json) - offset);
            offset += sizeof(json) - offset;
        } else {
            tok.addData(json + offset, 2);
            offset += 2;
        }
    };
    auto ref = tokenizer.registerNeedMoreDataCallback(func);

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
    };
    auto ref = tokenizer.registerNeedMoreDataCallback(func);
    
    jt_validate_json(tokenizer);
}

const char json2[] =
R"json({
	"test" : true,
	"more" : {
		"sub_object_prop1" : true,
		"sub_object_prop2" : 456
	},
	"int_value" : 65
})json";

struct Child
{
	bool sub_object_prop1;
	int sub_object_prop2;
	JT_STRUCT(JT_MEMBER(sub_object_prop1),
			  JT_MEMBER(sub_object_prop2));
};

struct Parent
{
	bool test;
	Child more;
	int int_value;
	JT_STRUCT(JT_MEMBER(test),
		      JT_MEMBER(more),
		      JT_MEMBER(int_value));
};

void jt_copy_parsed()
{
	JT::Tokenizer tokenizer;
	tokenizer.addData(json2);

	JT::Token token;;
	JT::Error error = JT::Error::NoError;
	std::vector<JT::Token> tokens;
	while (error == JT::Error::NoError) {
		error = tokenizer.nextToken(token);
		tokens.push_back(token);
	}

	JT::ParseContext context;
	context.tokenizer.addData(&tokens);
	Parent parent;
	context.parseTo(parent);

	JT_ASSERT(context.error == JT::Error::NoError);
	JT_ASSERT(parent.test == true);
	JT_ASSERT(parent.more.sub_object_prop1 == true);
	JT_ASSERT(parent.more.sub_object_prop2 == 456);
	JT_ASSERT(parent.int_value == 65);
}

const char json_token_copy[] = R"json(
{
    "number" : 45,
    "valid" : false,
    "child" : {
        "some_more": "world",
        "another_int" : 495
    },
    "more_data" : "string data",
    "super_data" : "hello"
}
)json";

struct SecondChild
{
    std::string some_more;
    int another_int;
    JT_STRUCT(JT_MEMBER(some_more),
        JT_MEMBER(another_int));
};
struct SecondParent
{
    int number;
    bool valid;
    JT::JsonTokens child;
    std::string more_data;
    std::string super_data;

    JT_STRUCT(JT_MEMBER(number),
        JT_MEMBER(valid),
        JT_MEMBER(child),
        JT_MEMBER(more_data),
        JT_MEMBER(super_data));
};

void jt_copy_tokens()
{
    SecondParent parent;
    JT::ParseContext parseContext(json_token_copy);
    parseContext.parseTo(parent);

    JT_ASSERT(parseContext.error == JT::Error::NoError);
    JT_ASSERT(parent.child.size() == 4);

    JT::ParseContext childContext;
    childContext.tokenizer.addData(&parent.child);
    SecondChild child;
    childContext.parseTo(child);
    JT_ASSERT(child.another_int == 495);
    JT_ASSERT(child.some_more == "world");
     
}
int main()
{
    jt_copy_full();
    jt_partial_1();
    jt_partial_2();
    jt_partial_3();

	jt_copy_parsed();
    jt_copy_tokens();
    return 0;
}
