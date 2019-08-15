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

#if JS_HAVE_CONSTEXPR
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
    unsigned char d;
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
    short f;
    JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(
                           JS_SUPER_CLASS(E)),
                         JS_MEMBER(f));
};
struct G
{
    char g;
    JS_OBJECT(JS_MEMBER(g));
};

struct Subclass : public B, public F, public G
{
    unsigned int h;
    JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(
                           JS_SUPER_CLASS(B),
                           JS_SUPER_CLASS(F),
                           JS_SUPER_CLASS(G)),
                         JS_MEMBER(h));
};

#endif

int main()
{
#if JS_HAVE_CONSTEXPR
    size_t member_count = JS::Internal::memberCount<Subclass, 0>();
    JS_ASSERT( member_count == 7);
    int array[JS::Internal::memberCount<Subclass,0 >()];
    for (size_t i = 0; i < JS::Internal::memberCount<Subclass, 0>(); i++) {
        array[i] = i;
    }

    for (size_t i = 0; i < JS::Internal::memberCount<Subclass, 0>(); i++)
        fprintf(stderr, "array %d\n", array[i]);
#endif
    return 0;
}
