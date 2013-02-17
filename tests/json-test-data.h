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

#ifndef JSON_TEST_DATA_H
#define JSON_TEST_DATA_H

#include <assert.h>

#include "json_tree.h"

const char json_data1[] = u8R"(
{
   "foo": "bar",
   "color" : "red"
   weather: "clear"
   weather1 : "clear1"
   ToBeTrue: true,
   HeresANull : null
   ThisIsFalse: false,

   EscapedString: "contains \"",
   ThisIsANumber: 3.14
   ThisIsAnObject: {
       ThisIsASubType: "red"
   },
   AnotherProp: "prop"
   ThisIsAnotherObject: {
       ThisIsAnotherASubType: "blue"
   },
   ThisIsAnArray: [
       12.4,
       3,
       43.2
   ]
   ThisIsAnObjectArray: [
       { Test1: "Test2", Test3: "Test4" },
       { Test5: true, Test7: false }
   ]
}
)";

const char json_data2[] = u8R"(
{
    "StringNode" : "Some test data",
    "NumberNode" : 4676.4,
    "NullNode" : null,
    "BooleanTrue" : true,
    "BooleanFalse" : false,
    "Object" : {
        "SomeSubObjectProp": "RED"
    },
    "Array" : [
        "String",
        null,
        true,
        {
            "SomeOtherObjectProp" : "GREEN"
        }
    ],
    "LastStringNode" : "More test data"
}
)";

static int check_json_tree_from_json_data2(JsonNode *root)
{
    StringNode *string_node = root->stringNodeAt("StringNode");
    assert(string_node);
    assert(string_node->string() == "Some test data");

    NumberNode *number_node = root->numberNodeAt("NumberNode");
    assert(number_node);
    assert(number_node->number() < 4676.41 && number_node->number() > 4676.39);

    NullNode *null_node = root->nullNodeAt("NullNode");
    assert(null_node);

    BooleanNode *boolean_true_node = root->booleanNodeAt("BooleanTrue");
    assert(boolean_true_node);
    assert(boolean_true_node->boolean());

    BooleanNode *boolean_false_node = root->booleanNodeAt("BooleanFalse");
    assert(boolean_false_node);
    assert(!boolean_false_node->boolean());

    ObjectNode *sub_object = root->objectNodeAt("Object");
    assert(sub_object);
    StringNode *sub_prop1 = sub_object->stringNodeAt("SomeSubObjectProp");
    assert(sub_prop1);
    assert(sub_prop1->string() == "RED");

    StringNode *some_sub_prop2 = root->stringNodeAt("Object.SomeSubObjectProp");
    assert(some_sub_prop2);
    assert(some_sub_prop2 == sub_prop1);

    ArrayNode *array_node = root->arrayNodeAt("Array");
    assert(array_node);

    StringNode *string_in_array = array_node->index(0)->asStringNode();
    assert(string_in_array);
    assert(string_in_array->string() == "String");

    NullNode *null_in_array = array_node->index(1)->asNullNode();
    assert(null_in_array);

    BooleanNode *true_in_array = array_node->index(2)->asBooleanNode();
    assert(true_in_array);
    assert(true_in_array->boolean());

    ObjectNode *object_in_array = array_node->index(3)->asObjectNode();
    assert(object_in_array);

    StringNode *string_in_object_in_array = object_in_array->stringNodeAt("SomeOtherObjectProp");
    assert(string_in_object_in_array);
    assert(string_in_object_in_array->string() == "GREEN");

    StringNode *last_string_node = root->stringNodeAt("LastStringNode");
    assert(last_string_node);
    assert(last_string_node->string() == "More test data");

    StringNode *non_existing_string_node = root->stringNodeAt("NON EXISTING STRING NODE");
    assert(!non_existing_string_node);

    NullNode *null_null_node = root->nullNodeAt("NoNullNodeHere");
    assert(!null_null_node);

    return 0;
}

#endif //JSON_TEST_DATA_H
