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

#ifdef NDEBUG
#error "These tests uses assert. Please remove define NDEBUG"
#endif

#include "json_tokenizer.h"
#include <assert.h>
#include <string>

static int assert_token(const JsonToken &token, JsonType::Type name_type, std::string property, JsonType::Type data_type, std::string data)
{
    if (token.name_type != name_type) {
        fprintf(stderr, "token.name_type is: %d, expected %d\n", token.name_type, name_type);
        return -1;
    }

    if (token.name_length != property.size()) {
        fprintf(stderr, "token.name_length is: %lu, expected: %lu\n", token.name_length, property.size());
        return -1;
    }
    std::string token_property(token.name, token.name_length);
    if (property.compare(token_property) != 0) {
        std::string name(token.name, token.name_length);
        fprintf(stderr, "token.property: %s is unequal to %s\n", name.c_str(), property.c_str());
        return -1;
    }

    if (token.data_type != data_type) {
        fprintf(stderr, "token.data_type is: %d, expected %d\n", token.data_type, data_type);
        return -1;
    }

    std::string token_data(token.data, token.data_length);
    if (data.compare(token_data) != 0) {
        std::string data_name(token.data, token.data_length);
        fprintf(stderr, "token.data: %s is unequal to %s\n", data_name.c_str(), data.c_str());
        return -1;
    }

    if (token.data_length != data.size()) {
        fprintf(stderr, "token.data_length is: %lu, expected: %lu\n", token.data_length, data.size());
        return -1;
    }
return 0;
}
