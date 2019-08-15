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

#include "json_struct.h"

#include "assert.h"
#include <unordered_map>


const char json_data1[] = "{\n"
    "\"StringNode\" : \"Some test data\",\n"
    "\"NumberNode\" : 4676.4,\n"
    "\"BooleanTrue\" : true,\n"
    "\"BooleanFalse\" : false,\n"
    "\"TestStruct\" : {\n"
        "\"SubString\" : \"Some other string\",\n"
        "\"SubNumber\" : 500,\n"
        "\"Array\" : [\n"
            "5,\n"	
            "6,\n"
            "3,\n"
            "6\n"
        "],\n"
        "\"optional_float\" : 300,\n"
        "\"this_property_does_not_exist\" : true\n"
    "},\n"
    "\"OptionalButWithData\" : [ 17.5 ],\n"
    "\"subStruct2\" : {\n"
        "\"Field1\" : 4,\n"
        "\"Field2\" : true\n"
    "},\n"
    "\"Skipped_sub_object\" : {\n"
        "\"Field3\" : 465\n"
    "}\n"
"}\n";

struct SubStruct
{
    std::string SubString;
    int SubNumber;
    std::vector<double> Array;
    JS::OptionalChecked<float> optional_float;
    JS::OptionalChecked<double> optional_double;
    JS::Optional<double> optional_with_value = 4.5;
    JS_OBJECT(JS_MEMBER(SubString),
              JS_MEMBER(SubNumber),
              JS_MEMBER(Array),
              JS_MEMBER(optional_float),
              JS_MEMBER(optional_with_value));
};

const char sub_struct3_data [] = "{\n"
        "\"Field1\" : 4,\n"
        "\"Field2\" : true,\n"
        "\"Field3\" : \"432\"\n"
    "}\n";

struct SubStruct2
{
    float Field1;
    bool Field2;
    JS_OBJECT(JS_MEMBER(Field1),
              JS_MEMBER_ALIASES(Field2, "hello", "Foobar"));
};

struct SubStruct3 : public SubStruct2
{
    std::string Field3;
    int Field4;
    JS::Optional<std::string> Field5;
    JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(JS_SUPER_CLASS(SubStruct2)),
                         JS_MEMBER(Field3),
                         JS_MEMBER(Field4),
                         JS_MEMBER(Field5));
};

struct JsonData1
{
    std::string StringNode;
    double NumberNode;
    bool BooleanTrue;
    /*!
     *very special comment for BooleanFalse
     *
     *\json
     *{
     *   json
     *}
    **/
    bool BooleanFalse;
    JS::Optional<int> OptionalInt;
    /// Test structur comment
    SubStruct TestStruct;
    JS::Optional<std::vector<double>> OptionalButWithData;
    float unassigned_value;
    std::unique_ptr<SubStruct2> subStruct2;

    int Field3 = 243;
    std::string NodeWithLiteral = "SOME STRING LITERAL!!!";
    
    JS_OBJECT(JS_MEMBER(StringNode),
              JS_MEMBER(NumberNode),
              JS_MEMBER(BooleanTrue),
              JS_MEMBER(BooleanFalse),
              JS_MEMBER(OptionalInt),
              JS_MEMBER(TestStruct),
              JS_MEMBER(OptionalButWithData),
              JS_MEMBER(unassigned_value),
              JS_MEMBER(subStruct2),
              JS_MEMBER(Field3),
              JS_MEMBER(NodeWithLiteral));

};

static int check_json_tree_nodes()
{
    JS::ParseContext context(json_data1);
    JsonData1 data;
    context.parseTo(data);

    for (double x : data.TestStruct.Array)
        fprintf(stderr, "x is %f\n", x);

    fprintf(stderr, "optional with default value %f\n", data.TestStruct.optional_with_value());
    data.TestStruct.optional_with_value = 5;
    fprintf(stderr, "optional with default value second %f\n", data.TestStruct.optional_with_value());
    JS_ASSERT(data.StringNode == "Some test data");
    JS_ASSERT(context.error == JS::Error::NoError);
    
    JS_ASSERT(data.Field3 == 243);

    std::string json = JS::serializeStruct(data);
    fprintf(stderr, "%s\n", json.c_str());
    return 0;
}

const char json_data2[] = "{\n"
    "\"some_int\" : 4,\n"
    "\"sub_object\" : {\n"
        "\"more_data\" : \"some text\",\n"
        "\"a_float\" : 1.2,\n"
        "\"boolean_member\" : false\n"
    "}\n"
"}\n";

template<typename T>
struct OuterStruct
{
    int some_int;
    T sub_object;

    JS_OBJECT(JS_MEMBER(some_int),
              JS_MEMBER(sub_object));
};

struct SubObject
{
    std::string more_data;
    float a_float;
    bool boolean_member;

    JS_OBJECT(JS_MEMBER(more_data),
              JS_MEMBER(a_float),
              JS_MEMBER(boolean_member));
};


static int check_json_tree_template()
{
    JS::ParseContext context(json_data2);
    OuterStruct<SubObject> data;
    context.parseTo(data);
    JS_ASSERT(data.sub_object.more_data == "some text");
    std::string json = JS::serializeStruct(data);
    fprintf(stderr, "%s\n", json.c_str());
    return 0;
};

static int check_json_tree_subclass()
{
    JS::ParseContext context(sub_struct3_data);
    SubStruct3 substruct3;
    context.parseTo(substruct3);
    JS_ASSERT(substruct3.Field3 == std::string("432"));
    return 0;
}

const char json_data3[] = "{\n"
    "\"SuperSuper\" : 5,\n"
    "\"Regular\": 42,\n"
    "\"Super\" : \"This is in the Superclass\"\n"
    "}\n";

struct SuperSuperClass {
    int SuperSuper;
    JS_OBJECT(JS_MEMBER(SuperSuper));
};

struct SuperClass : public SuperSuperClass
{
    std::string Super;
    JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(
                            JS_SUPER_CLASS(SuperSuperClass)),
                         JS_MEMBER(Super));
};

struct RegularClass : public SuperClass
{
    int Regular;
    JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(
                            JS_SUPER_CLASS(SuperClass)),
                         JS_MEMBER(Regular));
};

void check_json_tree_deep_tree()
{
    JS::ParseContext context(json_data3);
    RegularClass regular;
    context.parseTo(regular);
    JS_ASSERT(regular.SuperSuper == 5);
    JS_ASSERT(regular.Super == "This is in the Superclass");
    JS_ASSERT(regular.Regular == 42);
}

const char missing_object_def[] = R"json(
{
    "first" : true,
    "second" : "hello world",
    "third" : {},
    "fourth" : 33
}
)json";

struct MissingObjectDef
{
    bool first;
    std::string second;
    int fourth;

    JS_OBJECT(JS_MEMBER(first),
        JS_MEMBER(second),
        JS_MEMBER(fourth));
};

void check_json_missing_object()
{
    JS::ParseContext context(missing_object_def);
    MissingObjectDef missing;
    context.parseTo(missing);

    JS_ASSERT(context.error == JS::Error::NoError);
    JS_ASSERT(missing.fourth == 33);
}

const char error_in_sub[] = R"json(
{
    "first" : {
        "ffirst" : 4,
        "fsecond" : {},
        "not_assigned" : 555
    },
    "second" : "hello world",
    "third" : 33
}
)json";

struct ErrorInSubChild
{
    int ffirst;
    JS_OBJECT(JS_MEMBER(ffirst));
};

struct ErrorInSub
{
    ErrorInSubChild first;
    std::string second;
    int third;
    JS::Optional<int> not_assigned = 999;
    JS_OBJECT(JS_MEMBER(first),
        JS_MEMBER(second),
        JS_MEMBER(third));
};

void check_json_error_in_sub()
{
    JS::ParseContext context(error_in_sub);
    ErrorInSub sub;
    context.parseTo(sub);
    JS_ASSERT(context.error == JS::Error::NoError);
    JS_ASSERT(sub.second == "hello world");
    JS_ASSERT(sub.third == 33);
    JS_ASSERT(sub.not_assigned.data == 999);
}

struct JsonObjectTester
{
    std::string field;
    JS::JsonObject obj;
    int number = 0;

    JS_OBJECT(JS_MEMBER(field),
              JS_MEMBER(obj),
              JS_MEMBER(number));
};

struct JsonObjectOrArrayObjectTester
{
    std::string field;
    JS::JsonObjectOrArray obj;
    int number = 0;

    JS_OBJECT(JS_MEMBER(field),
              JS_MEMBER(obj),
              JS_MEMBER(number));
};

struct JsonObjectRefTester
{
    std::string field;
    JS::JsonObjectRef obj;
    int number = 0;

    JS_OBJECT(JS_MEMBER(field),
              JS_MEMBER(obj),
              JS_MEMBER(number));
};

struct JsonObjectOrArrayObjectRefTester
{
    std::string field;
    JS::JsonObjectOrArrayRef obj;
    int number = 0;

    JS_OBJECT(JS_MEMBER(field),
              JS_MEMBER(obj),
              JS_MEMBER(number));
};

const char jsonObjectTest[] = R"json({
    "field" : "hello",
    "obj" : {
        "some_sub_filed" : 2,
        "some_sub_array" : [ "a", "b", "c"],
        "some_sub_object" : { "field" : "not hello" }
    },
    "number" : 43
})json";

void check_json_object()
{
    JS::ParseContext context(jsonObjectTest);
    JsonObjectTester obj;
    context.parseTo(obj);
    JS_ASSERT(context.error == JS::Error::NoError);
    JS_ASSERT(obj.field == "hello");
    JS_ASSERT(obj.obj.data.size() > 0);
    JS_ASSERT(obj.number == 43);

    std::string out = JS::serializeStruct(obj);
    JS_ASSERT(out == jsonObjectTest);
}

void check_json_object_or_array_object()
{
    JS::ParseContext context(jsonObjectTest);
    JsonObjectOrArrayObjectTester obj;
    context.parseTo(obj);
    JS_ASSERT(context.error == JS::Error::NoError);
    JS_ASSERT(obj.field == "hello");
    JS_ASSERT(obj.obj.data.size() > 0);
    JS_ASSERT(obj.number == 43);

    std::string out = JS::serializeStruct(obj);
    JS_ASSERT(out == jsonObjectTest);
}

void check_json_object_ref()
{
    JS::ParseContext context(jsonObjectTest);
    JsonObjectRefTester obj;
    context.parseTo(obj);
    JS_ASSERT(context.error == JS::Error::NoError);
    JS_ASSERT(obj.field == "hello");
    JS_ASSERT(obj.obj.ref.size > 0);
    JS_ASSERT(obj.number == 43);

    std::string out = JS::serializeStruct(obj);
    JS_ASSERT(out == jsonObjectTest);
}

void check_json_object_or_array_object_ref()
{
    JS::ParseContext context(jsonObjectTest);
    JsonObjectOrArrayObjectRefTester obj;
    context.parseTo(obj);
    JS_ASSERT(context.error == JS::Error::NoError);
    JS_ASSERT(obj.field == "hello");
    JS_ASSERT(obj.obj.ref.size > 0);
    JS_ASSERT(obj.number == 43);

    std::string out = JS::serializeStruct(obj);
    JS_ASSERT(out == jsonObjectTest);
}

struct JsonArrayTester
{
    std::string string;
    JS::JsonArray array;
    int number = 0;

    JS_OBJECT(JS_MEMBER(string),
              JS_MEMBER(array),
              JS_MEMBER(number));
};

struct JsonObjectOrArrayArrayTester
{
    std::string string;
    JS::JsonObjectOrArray array;
    int number = 0;

    JS_OBJECT(JS_MEMBER(string),
              JS_MEMBER(array),
              JS_MEMBER(number));
};

struct JsonArrayRefTester
{
    std::string string;
    JS::JsonArrayRef array;
    int number = 0;

    JS_OBJECT(JS_MEMBER(string),
              JS_MEMBER(array),
              JS_MEMBER(number));
};

struct JsonObjectOrArrayArrayRefTester
{
    std::string string;
    JS::JsonObjectOrArrayRef array;
    int number = 0;

    JS_OBJECT(JS_MEMBER(string),
              JS_MEMBER(array),
              JS_MEMBER(number));
};

const char jsonArrayTest[] = R"json({
    "string" : "foo",
    "array" : [
        ["a","b","c"],
        {
            "sub object" : 44.50
        },
        12345
    ],
    "number" : 43
})json";

void check_json_array()
{
    JS::ParseContext context(jsonArrayTest);
    JsonArrayTester obj;
    context.parseTo(obj);
    JS_ASSERT(context.error == JS::Error::NoError);
    JS_ASSERT(obj.string == "foo");
    JS_ASSERT(obj.array.data.size() > 0);
    JS_ASSERT(obj.number == 43);

    std::string out = JS::serializeStruct(obj);
    JS_ASSERT(out == jsonArrayTest);
}

void check_json_object_or_array_array()
{
    JS::ParseContext context(jsonArrayTest);
    JsonObjectOrArrayArrayTester obj;
    context.parseTo(obj);
    JS_ASSERT(context.error == JS::Error::NoError);
    JS_ASSERT(obj.string == "foo");
    JS_ASSERT(obj.array.data.size() > 0);
    JS_ASSERT(obj.number == 43);

    std::string out = JS::serializeStruct(obj);
    JS_ASSERT(out == jsonArrayTest);
}

void check_json_array_ref()
{
    JS::ParseContext context(jsonArrayTest);
    JsonArrayRefTester obj;
    context.parseTo(obj);
    JS_ASSERT(context.error == JS::Error::NoError);
    JS_ASSERT(obj.string == "foo");
    JS_ASSERT(obj.array.ref.size > 0);
    JS_ASSERT(obj.number == 43);

    std::string out = JS::serializeStruct(obj);
    JS_ASSERT(out == jsonArrayTest);
}

void check_json_object_or_array_array_ref()
{
    JS::ParseContext context(jsonArrayTest);
    JsonObjectOrArrayArrayRefTester obj;
    context.parseTo(obj);
    JS_ASSERT(context.error == JS::Error::NoError);
    JS_ASSERT(obj.string == "foo");
    JS_ASSERT(obj.array.ref.size > 0);
    JS_ASSERT(obj.number == 43);

    std::string out = JS::serializeStruct(obj);
    JS_ASSERT(out == jsonArrayTest);
}

struct JsonMapTest
{
	std::unordered_map<std::string, JS::JsonTokens> map;

	JS_OBJECT(JS_MEMBER(map));
};

const char jsonMapTest[] = R"json({
    "map" : {
		"hello" : { "some object" : 3 },
		"bye" : [4]
	}
})json";

void check_json_map()
{
#ifdef JS_UNORDERED_MAP_HANDLER
    JS::ParseContext context(jsonMapTest);
    JsonMapTest obj;
    context.parseTo(obj);
    JS_ASSERT(context.error == JS::Error::NoError);
#endif
}

struct TypeHandlerTypes
{
    double doubleN;
    float floatN;
    int intN;
    unsigned int uintN;
    int64_t int64N;
    uint64_t uint64N;
    int16_t int16N;
    uint16_t uint16N;
    bool boolN;


    JS_OBJECT(JS_MEMBER(doubleN),
              JS_MEMBER(floatN),
              JS_MEMBER(intN),
              JS_MEMBER(uintN),
              JS_MEMBER(int64N),
              JS_MEMBER(uint64N),
              JS_MEMBER(int16N),
              JS_MEMBER(uint16N),
              JS_MEMBER(boolN)
              );
};

const char jsonTypeHandlerTypes[] = R"json({
    "doubleN" : 44.50,
    "floatN" : 33.40,
    "intN" : -345,
    "uintN" : 567,
    "int64N" : -1234,
    "uint64N" : 987,
    "int16N" : -23,
    "uint16N" : 45,
    "boolN" : true
})json";

void check_json_type_handler_types()
{
    JS::ParseContext context(jsonTypeHandlerTypes);
    TypeHandlerTypes obj;
    context.parseTo(obj);
    JS_ASSERT(context.error == JS::Error::NoError);
}

struct ArrayTest 
{
    double data[3];

	JS_OBJECT(JS_MEMBER(data)
	);
};

const char arrayTestJson[] = R"json({
    "data" : [4, 5, 6]
})json";

void check_json_array_test()
{
    JS::ParseContext context(arrayTestJson);
    ArrayTest obj;
    context.parseTo(obj);
    JS_ASSERT(context.error == JS::Error::NoError);
}

struct SkipTestBase
{
    std::string name;
    int id;

    JS_OBJECT(JS_MEMBER(name),
              JS_MEMBER(id));
};

struct SkipTestInternalContainer
{
    std::vector<float> items;

    JS_OBJECT(JS_MEMBER(items));
};

struct SkipTestNameContainer
{
    std::string name;
    int id;
    SkipTestInternalContainer container;

    JS_OBJECT(JS_MEMBER(name),
              JS_MEMBER(id),
              JS_MEMBER(container));
};

struct SkipTestSubClass : public SkipTestBase
{
    float value;
    std::vector<SkipTestNameContainer> skip_test_list_01;
    std::vector<SkipTestNameContainer> skip_test_list_02;

    JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(JS_SUPER_CLASS(SkipTestBase)),
                         JS_MEMBER(value),
                         JS_MEMBER(skip_test_list_01),
                         JS_MEMBER(skip_test_list_02));
};

const char jsonSkipTest[] = R"json(
{
    "skip_test_list_01": [
        {
            "id" : 1,
            "container" : {
                "items" : []
            },
            "skip_me" : [],
            "name": "list01"
        },
        {
            "name": "list02",
            "skip_me" : [],
            "container" : {
                "items" : [1.1, 2.2, 3.3]
            },
            "id" : 2
        },
        {
            "skip_me" : [],
            "name": "list03",
            "id" : 3,
            "container" : {
                "items" : [0, 1, 2]
            }
        }
    ],
    "skip_test_list_02": [
        {
            "name": "list01",
            "id" : 1,
            "container" : {
                "items" : []
            },
            "skip_me" : []
        },
        {
            "name": "list02",
            "skip_me" : [],
            "container" : {
                "items" : []
            },
            "id" : 2
        },
        {
            "container" : {
                "items" : []
            },
            "skip_me" : [],
            "name": "list03",
            "id" : 3
        }
    ],
    "value" : 3.14,
    "name" : "base_name",
    "id" : 444
}
)json";

void check_json_skip_test()
{
    JS::ParseContext base_context(jsonSkipTest);
    SkipTestBase base;
    base_context.parseTo(base);
    JS_ASSERT(base_context.error == JS::Error::NoError);
    JS_ASSERT(base.name == "base_name");
    JS_ASSERT(base.id == 444);

    JS::ParseContext sub_context(jsonSkipTest);
    SkipTestSubClass sub;
    sub_context.parseTo(sub);
    JS_ASSERT(sub_context.error == JS::Error::NoError);
    JS_ASSERT(sub.skip_test_list_01[2].name == "list03");
    JS_ASSERT(sub.skip_test_list_02[1].name == "list02");
    JS_ASSERT(sub.name == "base_name");
    JS_ASSERT(sub.id == 444);
}

const char multi_top_level_json[] = R"json({ a: 1}{a: 2}{a:3})json";
struct MultiTopLevel
{
    int a;
    JS_OBJECT(JS_MEMBER(a));
};

void check_multi_top_level_json()
{
    JS::ParseContext pc(multi_top_level_json);
    pc.tokenizer.allowAsciiType(true);
    MultiTopLevel t;
    for (int i = 0; i < 3; i++)
    {
        JS_ASSERT(pc.tokenizer.currentPosition() < multi_top_level_json + sizeof(multi_top_level_json)-1);
        pc.parseTo(t);
        JS_ASSERT(t.a == i+1);
    }
    JS_ASSERT(pc.tokenizer.currentPosition() == multi_top_level_json + sizeof(multi_top_level_json)-1);
}

const char escapedJson[] = R"json({
    "some_text" : "more\"_te\\xt",
    "sub_object" : {
        "more_data" : "so\\me \"text",
        "a_float" : 1.2,
        "boolean_member" : false
    }
})json";

template<typename T>
struct EscapedOuterStruct
{
    std::string some_text;
    T sub_object;

    JS_OBJECT(JS_MEMBER(some_text),
              JS_MEMBER(sub_object));
};

struct EscapedSubObject
{
    std::string more_data;
    float a_float;
    bool boolean_member;

    JS_OBJECT(JS_MEMBER(more_data),
              JS_MEMBER(a_float),
              JS_MEMBER(boolean_member));
};

static int check_json_escaped()
{
    JS::ParseContext context(escapedJson);
    EscapedOuterStruct<EscapedSubObject> data;
    context.parseTo(data);
    JS_ASSERT(context.error == JS::Error::NoError);
    std::string equals("more\"_te\\xt");
    JS_ASSERT(data.some_text == equals);
    std::string json = JS::serializeStruct(data);
    fprintf(stderr, "%s\n", json.c_str());
    return 0;
};

struct OutsideMeta
{
    std::string data;
    float a;
};

JS_OBJECT_EXTERNAL(OutsideMeta,
    JS_MEMBER(data),
    JS_MEMBER(a)
);

const char outside_json[] = R"json({
    "data" : "this is some text",
    "a" : 44.5
})json";

void check_json_meta_outside()
{
    JS::ParseContext context(outside_json);
    OutsideMeta data;
    context.parseTo(data);
    JS_ASSERT(context.error == JS::Error::NoError);
    JS_ASSERT(data.data == "this is some text");
    JS_ASSERT(data.a == 44.5);
}

int main(int, char **)
{
    check_json_tree_nodes();
    check_json_tree_template();
    check_json_tree_subclass();
    check_json_tree_deep_tree();
    check_json_missing_object();
    check_json_error_in_sub();
    check_json_object();
    check_json_object_or_array_object();
    check_json_object_ref();
    check_json_object_or_array_object_ref();
    check_json_array();
    check_json_object_or_array_array();
    check_json_array_ref();
    check_json_object_or_array_array_ref();
    check_json_map();
    check_json_type_handler_types();
    	check_json_array_test();
    check_json_skip_test();
    check_multi_top_level_json();
    check_json_escaped();
    check_json_meta_outside();
    return 0;
}
