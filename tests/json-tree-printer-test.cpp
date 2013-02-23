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

static int check_json_tree_printer()
{
    JT::Tokenizer tokenizer;
    tokenizer.addData(json_data2, sizeof(json_data2));
    auto created = JT::Node::create(&tokenizer);

    JT::Node *root = created.first;

    check_json_tree_from_json_data2(root);

    JT::PrinterOption printerOption(false);
    char buffer[4096];
    memset(buffer,'\0', 4096);
    JT::PrintHandler buffer_handler(buffer,4096);
    assert(root->print(buffer_handler, printerOption));

    size_t printed_size = root->printSize(printerOption);
    size_t actual_size = strlen(buffer);

    assert(printed_size == actual_size);

    delete root;

    tokenizer = JT::Tokenizer();
    tokenizer.addData(buffer, actual_size);
    created = JT::Node::create(&tokenizer);
    root = created.first;
    check_json_tree_from_json_data2(root);

    return 0;
}

static int check_json_tree_printer_pretty()
{
    JT::Tokenizer tokenizer;
    tokenizer.addData(json_data2, sizeof(json_data2));
    auto created = JT::Node::create(&tokenizer);

    JT::Node *root = created.first;

    check_json_tree_from_json_data2(root);

    JT::PrinterOption printerOption(true);
    char buffer[4096];
    memset(buffer,'\0', 4096);
    JT::PrintHandler buffer_handler(buffer,4096);
    assert(root->print(buffer_handler, printerOption));

    size_t printed_size = root->printSize(printerOption);
    size_t actual_size = strlen(buffer);

    assert(printed_size == actual_size);

    delete root;

    tokenizer = JT::Tokenizer();
    tokenizer.addData(buffer, actual_size);
    created = JT::Node::create(&tokenizer);
    root = created.first;
    check_json_tree_from_json_data2(root);

    return 0;
}

int main(int argc ,char **argv)
{
    check_json_tree_printer();
    check_json_tree_printer_pretty();
    return 0;
}
