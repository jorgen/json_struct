/*
 * Copyright © 2013 Jorgen Lind
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

#include "json_tree.h"

#include "json-test-data.h"

#include <assert.h>

const char json_data_no_root[] = u8R"(
"foo" : "bar",
"testArray" : [
    3,
    1,
    4,
    1,
    5
],
"falseProp" : false
)";

const char json_data_no_root_fail[] = u8R"(
"foo" : "bar",
"testArray" : [
    3,
    1,
    4
    5
],
"falseProp" : false
)";
static int check_json_tree_nodes()
{
    JT::TreeBuilder tree_builder;
    auto created = tree_builder.build(json_data2,sizeof(json_data2));
    JT::Node *root = created.first;
    assert(created.second == JT::Error::NoError);
    assert(root);

    check_json_tree_from_json_data2(root);

    return 0;
}

static int check_json_tree_no_root()
{
    JT::TreeBuilder tree_builder;
    auto created = tree_builder.build(json_data_no_root, sizeof(json_data_no_root));
    JT::Node *root = created.first;
    assert(created.second != JT::Error::NoError);
    assert(!root);

    tree_builder.create_root_if_needed = true;
    created = tree_builder.build(json_data_no_root, sizeof(json_data_no_root));
    root = created.first;
    assert(created.second == JT::Error::NoError);
    assert(root);

    JT::StringNode *foo = root->stringNodeAt("foo");
    assert(foo);
    assert(foo->string() == "bar");

    JT::ArrayNode *array = root->arrayNodeAt("testArray");
    assert(array);

    JT::BooleanNode *falseNode = root->booleanNodeAt("falseProp");
    assert(falseNode);

    created = tree_builder.build(json_data_no_root_fail, sizeof(json_data_no_root_fail));
    root = created.first;
    assert(created.second != JT::Error::NoError);
    assert(!root);

    created = tree_builder.build(json_data2,sizeof(json_data2));
    root = created.first;
    assert(created.second == JT::Error::NoError);
    assert(root);

    check_json_tree_from_json_data2(root);

    return 0;
};

int main(int, char **)
{
    check_json_tree_nodes();
    check_json_tree_no_root();
    return 0;
}
