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

const char json_data1[] = R"json(
    {
        "TheAlias" : 55,
        "SomeOtherValue" : 44
    }
)json";

struct FirstAlias
{
    int ThePrimary = 0;
    int SomeOtherValue = 0;

    JT_STRUCT(
              JT_MEMBER_ALIASES(ThePrimary, "TheAlias"),
              JT_MEMBER(SomeOtherValue));
};

void checkPlain()
{
    JT::ParseContext context(json_data1);
    FirstAlias fa;
    context.parseTo(fa);

    JT_ASSERT(fa.ThePrimary == 55);
    JT_ASSERT(fa.SomeOtherValue == 44);
}

struct ShadowAlias
{
    int TheAlias = 0;
    int SomeOtherValue = 0; 

    JT_STRUCT(
              JT_MEMBER_ALIASES(TheAlias, "SomeOtherValue"),
              JT_MEMBER(SomeOtherValue));
};

void checkPlainShadow()
{
    JT::ParseContext context(json_data1);
    ShadowAlias sa;
    context.parseTo(sa);

    JT_ASSERT(sa.TheAlias == 55);
    JT_ASSERT(sa.SomeOtherValue == 44);
}

const char json_data2[] = R"json(
    {
        "SomeOtherValue" : 44,
        "TheAlias" : 55
    }
)json";

struct TheSuper
{
    int TheAlias = 0;
    JT_STRUCT(JT_MEMBER(TheAlias));
};

struct TheSub : public TheSuper
{
    int SomeOtherValue = 0;
    JT_STRUCT_WITH_SUPER(JT_SUPER_CLASSES(JT_SUPER_CLASS(TheSuper)),
                         JT_MEMBER_ALIASES(SomeOtherValue, "TheAlias"));
};

void checkSuperShadow()
{
    JT::ParseContext context(json_data1);
    TheSub sa;
    context.parseTo(sa);

    JT_ASSERT(sa.TheAlias == 55);
    JT_ASSERT(sa.SomeOtherValue == 44);

    context = JT::ParseContext(json_data2);
    sa = TheSub();
    context.parseTo(sa);

    JT_ASSERT(sa.TheAlias == 55);
    JT_ASSERT(sa.SomeOtherValue == 44);
}

int main()
{
    checkPlain();
    checkPlainShadow();
    checkSuperShadow();
    return 0;
}
