/*
 * Copyright © 2013 Jørgen Lind

 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.

 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
*/
#include <malloc.h>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include "io.h"
#include "arg.h"

#include <iostream>


//#include <errno.h>

void Arg::printError(const char* msg1, const option::Option& opt, const char* msg2)
{
  fprintf(stderr, "%s", msg1);
  fwrite(opt.name, opt.namelen, sizeof(char), stderr);
  fprintf(stderr, "%s", msg2);

}

option::ArgStatus Arg::requiresValue(const option::Option &option, bool msg)
{
    if (option.arg != 0)
        return option::ARG_OK;

    if (msg)
        printError("Option '", option, "' requires a valid directory path\n");

    return option::ARG_ILLEGAL;
}

option::ArgStatus Arg::requiresExistingFile(const option::Option &option, bool msg)
{
    struct stat stat_buf;
    stat(option.arg, &stat_buf);

    if (S_ISREG(stat_buf.st_mode)) {
        return option::ARG_OK;
    }

    if (msg)
        printError("Option '", option, "' requires an existing path\n");

    return option::ARG_ILLEGAL;
}

option::ArgStatus Arg::requiresNonExistingFile(const option::Option &option, bool )
{
    struct stat stat_buf;
    int success = stat(option.arg, &stat_buf);
    if (success == ENOENT)
        return option::ARG_OK;

    return option::ARG_ILLEGAL;
}

option::ArgStatus Arg::unknown(const option::Option &option, bool msg)
{
    if (msg)
        printError("Unknown option '", option, "'\n");
    return option::ARG_ILLEGAL;
}
