/*
 * Copyright © 2017 Jorgen Lind
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

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(external_json);

struct Simple
{
    std::string A;
    bool b;
    int some_longer_name;
    JS_OBJECT(JS_MEMBER(A),
              JS_MEMBER(b),
              JS_MEMBER(some_longer_name));
};

const char expected1[] = R"json({
    "A" : "TestString",
    "b" : false,
    "some_longer_name" : 456
})json";

void test_serialize_simple()
{
    Simple simple;
    simple.A = "TestString";
    simple.b = false;
    simple.some_longer_name = 456;

    std::string output = JS::serializeStruct(simple);
    JS_ASSERT(output == expected1);
}

struct A
{
    int a;
    JS_OBJECT(JS_MEMBER(a));
};

struct B : public A
{
    float b;
    JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(
                                          JS_SUPER_CLASS(A)),
                         JS_MEMBER(b));
};

struct D
{
    int d;
    JS_OBJECT(JS_MEMBER(d));
};

struct E : public D
{
    double e;
    JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(
                                          JS_SUPER_CLASS(D)),
                         JS_MEMBER(e));
};

struct F : public E
{
    int f;
    JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(
                                          JS_SUPER_CLASS(E)),
                         JS_MEMBER(f));
};
struct G
{
    std::string g;
    JS_OBJECT(JS_MEMBER(g));
};

struct Subclass : public B, public F, public G
{
    int h;
    JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(
                                          JS_SUPER_CLASS(B),
                                          JS_SUPER_CLASS(F),
                                          JS_SUPER_CLASS(G)),
                         JS_MEMBER(h));
};

const char expected2[] = R"json({
    "h" : 7,
    "g" : "OutputString",
    "f" : 6,
    "e" : 5.5000000000000000e+00,
    "d" : 5,
    "b" : 4.50000000e+00,
    "a" : 4
})json";

void test_serialize_deep()
{
    Subclass subclass;
    subclass.a = 4;
    subclass.b = 4.5;
    subclass.d = 5;
    subclass.e = 5.5;
    subclass.f = 6;
    subclass.g = "OutputString";
    subclass.h = 7;

    std::string output = JS::serializeStruct(subclass);
    JS_ASSERT(output == expected2);
}

struct WithEscapedData
{
    std::string data;
    JS_OBJECT(JS_MEMBER(data));
};

const char escaped_expected[] = R"json({
    "data" : "escaped \n \" \t string"
})json";

void test_escaped_data()
{
    WithEscapedData escaped;
    escaped.data = "escaped \n \" \t string";
    std::string output = JS::serializeStruct(escaped);
    fprintf(stderr, "%s\n%s\n", escaped_expected, output.c_str());
    JS_ASSERT(output == escaped_expected);
}

const char expected3[]=R"json({"h":7,"g":"OutputString","f":6,"e":5.5000000000000000e+00,"d":5,"b":4.50000000e+00,"a":4})json";
void test_compact()
{
    Subclass subclass;
    subclass.a = 4;
    subclass.b = 4.5;
    subclass.d = 5;
    subclass.e = 5.5;
    subclass.f = 6;
    subclass.g = "OutputString";
    subclass.h = 7;

    std::string output = JS::serializeStruct(subclass, JS::SerializerOptions(JS::SerializerOptions::Compact));
    JS_ASSERT(output == expected3);
}

void test_serialize_big()
{
    auto fs = cmrc::external_json::get_filesystem();
    auto generated = fs.open("generated.json");

    JS::JsonObjectOrArrayRef objOrArr;
    {
        JS::ParseContext pc(generated.begin(), generated.size());
        pc.parseTo(objOrArr);
        JS_ASSERT(pc.error == JS::Error::NoError);
    }

    std::string serialized_json = JS::serializeStruct(objOrArr);

    {
        JS::ParseContext pc(serialized_json.data(), serialized_json.size());
        pc.parseTo(objOrArr);
        JS_ASSERT(pc.error == JS::Error::NoError);
    }
}

int main()
{
    test_serialize_simple();
    test_serialize_deep();
    test_escaped_data();
    test_compact();
    test_serialize_big();
    return 0;
}
