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

#include "json_tools.h"
#include "assert.h"
#include <string>

static int assert_token(const JT::Token &token, JT::Type name_type, std::string property, JT::Type value_type, std::string value)
{
    if (token.name_type != name_type) {
        fprintf(stderr, "token.name_type is: %hhu, expected %hhu\n", token.name_type, name_type);
        return -1;
    }

    if (token.name.size != property.size()) {
        fprintf(stderr, "token.name_length is: %lu, expected: %lu\n", token.name.size, property.size());
        return -1;
    }
    std::string token_property(token.name.data, token.name.size);
    if (property.compare(token_property) != 0) {
        std::string name(token.name.data, token.name.size);
        fprintf(stderr, "token.property: %s is unequal to %s\n", name.c_str(), property.c_str());
        return -1;
    }

    if (token.value_type != value_type) {
        fprintf(stderr, "token.value_type is: %hhu, expected %hhu\n", token.value_type, value_type);
        return -1;
    }

    std::string token_value(token.value.data, token.value.size);
    if (value.compare(token_value) != 0) {
        std::string value_name(token.value.data, token.value.size);
        fprintf(stderr, "token.value: %s is unequal to %s\n", value_name.c_str(), value.c_str());
        return -1;
    }

    if (token.value.size != value.size()) {
        fprintf(stderr, "token.value_length is: %lu, expected: %lu\n", token.value.size, value.size());
        return -1;
    }
return 0;
}
