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

#if JT_HAVE_CONSTEXPR
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
    unsigned char d;
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
    short f;
    JT_STRUCT_WITH_SUPER(JT_SUPER_CLASSES(
                           JT_SUPER_CLASS(E)),
                         JT_MEMBER(f));
};
struct G
{
    char g;
    JT_STRUCT(JT_MEMBER(g));
};

struct Subclass : public B, public F, public G
{
    unsigned int h;
    JT_STRUCT_WITH_SUPER(JT_SUPER_CLASSES(
                           JT_SUPER_CLASS(B),
                           JT_SUPER_CLASS(F),
                           JT_SUPER_CLASS(G)),
                         JT_MEMBER(h));
};

#endif

int main()
{
#if JT_HAVE_CONSTEXPR
    size_t member_count = JT::Internal::memberCount<Subclass, 0>();
    JT_ASSERT( member_count == 7);
    int array[JT::Internal::memberCount<Subclass,0 >()];
    for (size_t i = 0; i < JT::Internal::memberCount<Subclass, 0>(); i++) {
        array[i] = i;
    }

    for (size_t i = 0; i < JT::Internal::memberCount<Subclass, 0>(); i++)
        fprintf(stderr, "array %d\n", array[i]);
#endif
    return 0;
}
