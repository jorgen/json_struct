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

#include "json_tools.h"

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
JT_STRUCT_EXTERNAL(TestStructT,
	JT_MEMBER(SubString),
	JT_MEMBER(SubNumber));

struct TestStructSub : public TestStructT
{
    std::vector<int> Array;
};
JT_STRUCT_EXTERNAL_WITH_SUPER(TestStructSub,
        JT_SUPER_CLASSES(JT_SUPER_CLASS(TestStructT)),
        JT_MEMBER(Array));

struct JsonData1
{
    std::string StringNode;
    double NumberNode;
    bool BooleanTrue;
    bool BooleanFalse;
    TestStructSub TestStruct;
};
JT_STRUCT_EXTERNAL(JsonData1,
        JT_MEMBER(StringNode),
        JT_MEMBER(NumberNode),
        JT_MEMBER(BooleanTrue),
        JT_MEMBER(BooleanFalse),
        JT_MEMBER(TestStruct));

static int check_json_tree_nodes()
{
    JT::ParseContext context(json_data1);
    JsonData1 data;
    context.parseTo(data);

	JT_ASSERT(context.error == JT::Error::NoError);
    JT_ASSERT(data.StringNode == "Some test data");
	JT_ASSERT(data.TestStruct.SubNumber == 500);
	JT_ASSERT(data.TestStruct.Array.size() == 4);
	JT_ASSERT(data.TestStruct.Array[2] == 3);
    JT_ASSERT(context.error == JT::Error::NoError);

    std::string json = JT::serializeStruct(data);
    fprintf(stderr, "%s\n", json.c_str());
    return 0;
}

int main(int, char **)
{
    check_json_tree_nodes();
    return 0;
}
