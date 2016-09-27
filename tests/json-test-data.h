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

//#include "json_tree.h"

//int check_json_tree_from_json_data2(JT::Node *root);

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

const char json_data2[] = u8R"({
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
})";

#endif //JSON_TEST_DATA_H
