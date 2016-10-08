/*
 * Copyright © 2013 Jorgen Lind
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

const char json_data1[] = u8R"({
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
        ],
        "optional_float" : 300
    },
    "OptionalButWithData" : [ 17.5 ]
    })";

struct SubStruct
{
    std::string SubString;
    int SubNumber;
    std::vector<double> Array;
    JT::OptionalChecked<float> optional_float;
    JT_STRUCT(SubStruct,
              JT_FIELD(SubString),
              JT_FIELD(SubNumber),
              JT_FIELD(Array),
              JT_FIELD(optional_float));
};

struct JsonData1
{
    std::string StringNode;
    double NumberNode;
    bool BooleanTrue;
    bool BooleanFalse;
    JT::Optional<int> OptionalInt;
    SubStruct TestStruct;
    JT::Optional<std::vector<double>> OptionalButWithData;


    JT_STRUCT(JsonData2,
              JT_FIELD(StringNode),
              JT_FIELD(NumberNode),
              JT_FIELD(BooleanTrue),
              JT_FIELD(BooleanFalse),
              JT_FIELD(OptionalInt),
              JT_FIELD(OptionalButWithData),
              JT_FIELD(TestStruct));
};

static int check_json_tree_nodes()
{
    JT::Error error;
    JsonData1 data = JT::parseData<JsonData1>(json_data1, sizeof(json_data1), error);

    float foo;
    for (double x : data.TestStruct.Array)
        fprintf(stderr, "x is %f\n", x);
    JT_ASSERT(data.StringNode == "Some test data");
    JT_ASSERT(error == JT::Error::NoError);
    return 0;
}

static int check_json_tree_no_root()
{
    return 0;
};

int main(int, char **)
{
    check_json_tree_nodes();
    //check_json_tree_no_root();
    return 0;
}
