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

const char json_data1[] = "{\n"
    "\"StringNode\" : \"Some test data\",\n"
    "\"NumberNode\" : 4676\n"
    "}\n";

struct ContainsStringNode
{
    std::string StringNode;

    JT_STRUCT(JT_MEMBER(StringNode));
};

struct SubStruct : public ContainsStringNode
{
    int NumberNode;

    JT_STRUCT_WITH_SUPER(JT_SUPER_CLASSES(
                            JT_SUPER_CLASS(ContainsStringNode)),
                         JT_MEMBER(NumberNode));
};

void testSimpleOneMember()
{
    JT::ParseContext context(json_data1);
    SubStruct substruct;
    context.parseTo(substruct);

    JT_ASSERT(substruct.StringNode == "Some test data");
    JT_ASSERT(substruct.NumberNode == 4676);
}

const char json_data2[] = "{\n"
    "\"ThisWillBeUnassigned\" : \"Some data\",\n"
    "\"StringNode\" : \"Some test data\",\n"
    "}\n";

void testSimpleVerifyMissingMemberInStruct()
{
    JT::ParseContext context(json_data2);
    ContainsStringNode containsString;
    context.parseTo(containsString);

    JT_ASSERT(containsString.StringNode == "Some test data");
    JT_ASSERT(context.missing_members.size() == 1);
    JT_ASSERT(context.missing_members.front() == "ThisWillBeUnassigned");
}

const char json_data3[] = "{\n"
    "\"Field1\" : 1\n,"
    "\"Field3\" : 3\n"
    "}\n";

struct RequiredMemberStruct
{
    int Field1;
    int Field2;
    int Field3;

    JT_STRUCT(JT_MEMBER(Field1),
              JT_MEMBER(Field2),
              JT_MEMBER(Field3));
};

void testSimpleVerifyMissingRequiredMemberInStruct()
{
    JT::ParseContext context(json_data3);
    RequiredMemberStruct requiredMemberStruct;
    context.parseTo(requiredMemberStruct);

    JT_ASSERT(requiredMemberStruct.Field3 == 3);
    JT_ASSERT(context.unassigned_required_members.size() == 1);
    JT_ASSERT(context.unassigned_required_members.front() == "Field2");
}

const char json_data4[] = "{\n"
    "\"StringNode\" : \"Some test data\",\n"
    "\"NumberNode\" : 342,\n"
    "\"SubNode\" : \"This should be in subclass\"\n"
    "}\n";

struct SuperClass
{
    std::string StringNode;
    int NumberNode;
    JT_STRUCT(JT_MEMBER(StringNode),
              JT_MEMBER(NumberNode));
};

struct SubClass : public SuperClass
{
    std::string SubNode;
    int SubNode2;
    JT_STRUCT_WITH_SUPER(JT_SUPER_CLASSES(
                            JT_SUPER_CLASS(SuperClass)),
                         JT_MEMBER(SubNode),
                         JT_MEMBER(SubNode2));
};

void testClassHirarchyVerifyMissingMemberInStruct()
{
    JT::ParseContext context(json_data4);
    SubClass subClass;
    context.parseTo(subClass);

    JT_ASSERT(subClass.NumberNode == 342);
    JT_ASSERT(subClass.SubNode == "This should be in subclass");
    JT_ASSERT(context.unassigned_required_members.size() == 1);
    JT_ASSERT(context.unassigned_required_members.front() == "SubNode2");
}

struct SuperSuperClass {
    int SuperSuper;
    JT_STRUCT(JT_MEMBER(SuperSuper));
};

struct SuperClass2 : public SuperSuperClass
{
    std::string Super;
    JT_STRUCT_WITH_SUPER(JT_SUPER_CLASSES(
                            JT_SUPER_CLASS(SuperSuperClass)),
                         JT_MEMBER(Super));
};

struct RegularClass : public SuperClass2
{
    int Regular;
    JT_STRUCT_WITH_SUPER(JT_SUPER_CLASSES(
                            JT_SUPER_CLASS(SuperClass2)),
                         JT_MEMBER(Regular));
};

const char json_data5[] = "{\n"
    "\"SuperSuper\" : 5,\n"
    "\"Regular\": 42\n"
    "}\n";

void testClassHIrarchyVerifyMissingDataForStruct()
{
    JT::ParseContext context(json_data5);
    RegularClass regular;
    context.parseTo(regular);

    JT_ASSERT(context.unassigned_required_members.size() == 1);
    JT_ASSERT(context.unassigned_required_members.front() == "SuperClass2::Super");
}

const char json_data6[] = "{\n"
    "\"SuperSuper\" : 5,\n"
    "\"Super\" : \"This is super\",\n"
    "\"SuperSuperSuper\" : 42,\n"
    "\"Regular\": 42\n"
    "}\n";

void testClassHirarchyVerifyMissingMemberInStruct2()
{
    JT::ParseContext context(json_data6);
    RegularClass regular;
    context.parseTo(regular);

    JT_ASSERT(context.missing_members.size() == 1);
    JT_ASSERT(context.missing_members.front() == "SuperSuperSuper");
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

const char json_data7[] = "{\n"
    "\"a\" : 4,\n"
    "\"b\" : 5.5,\n"
    "\"d\" : 127,\n"
    "\"f\" : 345,\n"
    "\"g\" : \"a\",\n"
    "\"h\" : 987\n"
    "}\n";

void testClassHirarchyVerifyMissingDataForStructDeep()
{
    JT::ParseContext context(json_data7);
    Subclass subclass;
    context.parseTo(subclass);

    JT_ASSERT(context.unassigned_required_members.size() == 1);
    JT_ASSERT(context.unassigned_required_members.front() == "E::e");
}

int main()
{
    testSimpleOneMember();
    testSimpleVerifyMissingMemberInStruct();
#if JT_HAVE_CONSTEXPR
    testSimpleVerifyMissingRequiredMemberInStruct();
    testClassHIrarchyVerifyMissingDataForStruct();
    testClassHirarchyVerifyMissingDataForStructDeep();
    testClassHirarchyVerifyMissingMemberInStruct();
#endif
    testClassHirarchyVerifyMissingMemberInStruct2();
    return 0;
}
