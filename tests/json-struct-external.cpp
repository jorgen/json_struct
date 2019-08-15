/*
 * Copyright © 2019 Jorgen Lind
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

#include "json_struct.h"

#include "assert.h"

const char json_data1[] = R"json(
{
    "StringNode" : "Some test data",
    "NumberNode" : 4676.4,
    "BooleanTrue" : true,
    "BooleanFalse" : false,
    "TestStruct" : {
        "SubString" : "Some other string",
        "SubNumber" : 500,
        "Array" : [
            5,
            6,
            3,
            6
        ]
	}
}
)json";
struct TestStructT
{
    std::string SubString;
    int SubNumber;
};
JS_OBJECT_EXTERNAL(TestStructT,
	JS_MEMBER(SubString),
	JS_MEMBER(SubNumber));

struct TestStructSub : public TestStructT
{
    std::vector<int> Array;
};
JS_OBJECT_EXTERNAL_WITH_SUPER(TestStructSub,
        JS_SUPER_CLASSES(JS_SUPER_CLASS(TestStructT)),
        JS_MEMBER(Array));

struct JsonData1
{
    std::string StringNode;
    double NumberNode;
    bool BooleanTrue;
    bool BooleanFalse;
    TestStructSub TestStruct;
};
JS_OBJECT_EXTERNAL(JsonData1,
        JS_MEMBER(StringNode),
        JS_MEMBER(NumberNode),
        JS_MEMBER(BooleanTrue),
        JS_MEMBER(BooleanFalse),
        JS_MEMBER(TestStruct));

static int check_json_tree_nodes()
{
    JS::ParseContext context(json_data1);
    JsonData1 data;
    context.parseTo(data);

	JS_ASSERT(context.error == JS::Error::NoError);
    JS_ASSERT(data.StringNode == "Some test data");
	JS_ASSERT(data.TestStruct.SubNumber == 500);
	JS_ASSERT(data.TestStruct.Array.size() == 4);
	JS_ASSERT(data.TestStruct.Array[2] == 3);
    JS_ASSERT(context.error == JS::Error::NoError);

    std::string json = JS::serializeStruct(data);
    fprintf(stderr, "%s\n", json.c_str());
    return 0;
}

int main(int, char **)
{
    check_json_tree_nodes();
    return 0;
}
