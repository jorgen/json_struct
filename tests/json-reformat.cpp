/*
 * Copyright © 209 Jorgen Lind
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

int main(int argc, char **argv)
{
    auto fs = cmrc::external_json::get_filesystem();
    auto generated = fs.open("generated.json");
    std::string pretty;
    JT::Error error = JT::reformat(generated.begin(), generated.size(), pretty);
    JT_ASSERT(error == JT::Error::NoError);
    fprintf(stderr, "PRETTY:\n%s\n", pretty.c_str());

    std::string compact;
    error = JT::reformat(generated.begin(), generated.size(), compact, JT::SerializerOptions(JT::SerializerOptions::Compact));
    JT_ASSERT(error == JT::Error::NoError);
    fprintf(stderr, "COMPACT:\n%s\n", compact.c_str());

    return 0;
}
