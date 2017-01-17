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

#include "json_tools.h"

#include "assert.h"

struct Simple
{
    std::string A;
    bool b;
    int some_longer_name;
    JT_STRUCT(JT_MEMBER(A),
              JT_MEMBER(b),
              JT_MEMBER(some_longer_name));
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

    std::string output = JT::serializeStruct(simple);
    JT_ASSERT(output == expected1);
}

struct A
{
    int a;
    JT_STRUCT(JT_MEMBER(a));
};

struct B : public A
{
    float b;
    JT_STRUCT_WITH_SUPER(JT_SUPER_CLASSES(
                                          JT_SUPER_CLASS(A)),
                         JT_MEMBER(b));
};

struct D
{
    int d;
    JT_STRUCT(JT_MEMBER(d));
};

struct E : public D
{
    double e;
    JT_STRUCT_WITH_SUPER(JT_SUPER_CLASSES(
                                          JT_SUPER_CLASS(D)),
                         JT_MEMBER(e));
};

struct F : public E
{
    int f;
    JT_STRUCT_WITH_SUPER(JT_SUPER_CLASSES(
                                          JT_SUPER_CLASS(E)),
                         JT_MEMBER(f));
};
struct G
{
    std::string g;
    JT_STRUCT(JT_MEMBER(g));
};

struct Subclass : public B, public F, public G
{
    int h;
    JT_STRUCT_WITH_SUPER(JT_SUPER_CLASSES(
                                          JT_SUPER_CLASS(B),
                                          JT_SUPER_CLASS(F),
                                          JT_SUPER_CLASS(G)),
                         JT_MEMBER(h));
};

const char expected2[] = R"json({
    "h" : 7,
    "g" : "OutputString",
    "f" : 6,
    "e" : 5.500000,
    "d" : 5,
    "b" : 4.500000,
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

    std::string output = JT::serializeStruct(subclass);
    JT_ASSERT(output == expected2);
}

int main()
{
    test_serialize_simple();
    test_serialize_deep();
    return 0;
}
