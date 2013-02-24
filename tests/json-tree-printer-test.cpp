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

#include <string.h>

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
    size_t reported_used = buffer_handler.printBuffers().front().used;

    assert(printed_size == actual_size);
    assert(actual_size == reported_used);

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
    size_t reported_used = buffer_handler.printBuffers().front().used;

    assert(printed_size == actual_size);
    assert(actual_size == reported_used);

    delete root;

    tokenizer = JT::Tokenizer();
    tokenizer.addData(buffer, actual_size);
    created = JT::Node::create(&tokenizer);
    root = created.first;
    check_json_tree_from_json_data2(root);

    return 0;
}

static int check_multiple_print_buffers()
{
    JT::Tokenizer tokenizer;
    tokenizer.addData(json_data2, sizeof(json_data2));
    auto created = JT::Node::create(&tokenizer);
    JT::Node *root = created.first;

    JT::PrinterOption printerOption(true);
    size_t printed_size = root->printSize(printerOption);

    JT::PrintHandler printHandler;
    char buffer1[printed_size/2];
    printHandler.appendBuffer(buffer1, printed_size/2);
    char buffer2[2];
    printHandler.appendBuffer(buffer2,2);
    char buffer3[printed_size];
    printHandler.appendBuffer(buffer3,printed_size);

    assert(root->print(printHandler, printerOption));

    size_t complete_size = 0;
    char target_buffer[4096];
    memset(target_buffer,'\0', 4096);
    auto buffers = printHandler.printBuffers();
    for (auto it = buffers.begin(); it != buffers.end(); ++it) {
        if ((*it).used > 0) {
            memcpy(target_buffer + complete_size, (*it).buffer, (*it).used);
            complete_size += (*it).used;
        }
        assert(complete_size <= printed_size);
    }

    assert(complete_size == printed_size);

    char valid_buffer[4096];
    memset(valid_buffer,'\0', 4096);
    printHandler = JT::PrintHandler();
    printHandler.appendBuffer(valid_buffer,4096);

    root->print(printHandler,printerOption);

    assert(memcmp(valid_buffer, target_buffer, printed_size) == 0);

    return 0;
}

static void add_buffer_func(JT::PrintHandler *printHandler, size_t atleast)
{
    char *buffer = new char[atleast];
    printHandler->appendBuffer(buffer,atleast);
}

static int check_callback_print_buffers()
{
    JT::Tokenizer tokenizer;
    tokenizer.addData(json_data2, sizeof(json_data2));
    auto created = JT::Node::create(&tokenizer);
    JT::Node *root = created.first;

    JT::PrinterOption printerOption(true);
    size_t printed_size = root->printSize(printerOption);

    JT::PrintHandler printHandler;
    printHandler.addRequestBufferCallback(add_buffer_func);

    assert(root->print(printHandler, printerOption));

    size_t complete_size = 0;
    char target_buffer[4096];
    memset(target_buffer,'\0', 4096);
    auto buffers = printHandler.printBuffers();
    for (auto it = buffers.begin(); it != buffers.end(); ++it) {
        if ((*it).used > 0) {
            memcpy(target_buffer + complete_size, (*it).buffer, (*it).used);
            complete_size += (*it).used;
            delete[] (*it).buffer;
        }
        assert(complete_size <= printed_size);
    }

    assert(complete_size == printed_size);

    char valid_buffer[4096];
    memset(valid_buffer,'\0', 4096);
    printHandler = JT::PrintHandler();
    printHandler.appendBuffer(valid_buffer,4096);

    root->print(printHandler,printerOption);

    assert(memcmp(valid_buffer, target_buffer, printed_size) == 0);

    return 0;
}

int main(int argc ,char **argv)
{
    check_json_tree_printer();
    check_json_tree_printer_pretty();
    check_multiple_print_buffers();
    check_callback_print_buffers();
    return 0;
}
