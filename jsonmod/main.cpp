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
#include "arg.h"
#include "configuration.h"
#include "json_streamer.h"

#include <iostream>
#include <vector>

enum optionIndex {
    UNKNOWN,
    HELP,
    FILE_SRC,
    INLINE,
    PROPERTY,
    COMPACT,
    PRETTY,
    VALUE,
    CREATE_OBJ,
    ONLY_NAME,
    DELIMITER,
    STRICT,
    SILENT,
};

const option::Descriptor usage[] =
 {
  {UNKNOWN,       0,  "", "",                 Arg::unknown,              "USAGE: jsonmod [options][file]\n\n"
                                                                         "If file is omitted, then input is read from stdin\n"
                                                                         "Output is allways going to stdout if not the inline option is specified\n\n"
                                                                         "Options:" },
  {HELP,          0, "h", "help",             option::Arg::None,         "  --help, -?\tPrint usage and exit." },
  {INLINE,        0, "i", "inline",           option::Arg::None,         "  --inline, -i  \tInline modifies the input file." },
  {PROPERTY,      0, "p", "property",         Arg::requiresValue,        "  --property, -p \tProperty to retrun/modify." },
  {VALUE,         0, "v", "value",            Arg::requiresValue,        "  --value, -v\tValue to update property with."},
  {COMPACT,       0, "c", "compact",          option::Arg::None,         "  --compact, -h\tOutput in compact format."},
  {PRETTY,        0, "" , "pretty",           option::Arg::None,         "  --pretty,    \tForce pretty printing."},
  {CREATE_OBJ,    0, "o", "create-object",    Arg::requiresValue,        "  --create-object, -o\t Create object with path."},
  {ONLY_NAME,     0, "n", "only-name",        option::Arg::None,         "  --only-name, -n\t Only print the name of the object."},
  {DELIMITER,     0, "d", "delimiter",        Arg::requiresValue,        "  --delimiter, -d\t Delimiter to use between objects nodes."},
  {STRICT,        0, "s", "strict",           option::Arg::None,         "  --strict    \t Use strict json parsing"},
  {SILENT,        0, "" , "silent",           option::Arg::None,         "  --silent\t Dont print out anything except error messages"},
  {UNKNOWN, 0,"" ,  ""   ,                    option::Arg::None,         "\nExamples:\n"
                                                                         "  jsonmod -i -p \"foo\" -v 43, /some/file \n"},
  {0,0,0,0,0,0}
 };

int main(int argc, char **argv)
{
    argc-=(argc>0); argv+=(argc>0);
    option::Stats  stats(true, usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parser(true, usage, argc, argv, options.data(), buffer.data());

    if (parser.error()) {
        return 1;
    }

    if (options[HELP]) {
        option::printUsage(std::cout, usage);
        return 0;
    }

    Configuration configuration;

    if (parser.nonOptionsCount() > 1) {
        fprintf(stderr, "Its not leagal to specify more than one input file\n");
        return 1;
    }

    if (parser.nonOptionsCount() == 1) {
        configuration.setInputFile(parser.nonOption(0));
    }

    for (int i = 0; i < parser.optionsCount(); ++i)
    {
        option::Option& opt = buffer[i];
        switch (opt.index())
        {
            case HELP:
                // not possible, because handled further above and exits the program
            case FILE_SRC:
                configuration.setInputFile(opt.arg);
                break;
            case INLINE:
                configuration.setInline(true);
                break;
            case PROPERTY:
                if (configuration.createObject()) {
                    fprintf(stderr, "Create object and setting the property field is mutually exclusive\n");
                    return -1;
                }
                configuration.setProperty(opt.arg);
                break;
            case COMPACT:
                configuration.setCompactPrint(true);
                break;
            case PRETTY:
                configuration.setPrettyPrint(true);
                break;
            case VALUE:
                configuration.setValue(opt.arg);
                break;
            case CREATE_OBJ:
                if (configuration.hasProperty()) {
                    fprintf(stderr, "Create object and setting the property field is mutually exclusive\n");
                    return -1;
                }
                configuration.setProperty(opt.arg);
                configuration.setCreateObject(true);
                break;
            case ONLY_NAME:
                configuration.setPrintOnlyName(true);
                break;
            case DELIMITER:
                configuration.setDelimiter(opt.arg);
                break;
            case STRICT:
                configuration.setStrict(true);
                break;
            case SILENT:
                configuration.setSilent(true);
                break;
            case UNKNOWN:
                fprintf(stderr, "UNKNOWN!");
                // not possible because Arg::Unknown returns ARG_ILLEGAL
                // which aborts the parse with an error
                break;
        }
    }

    if (configuration.sane()) {
        JsonStreamer streamer(configuration);
        if (streamer.error())
            return 2;
        streamer.stream();
        if (streamer.error())
            return 3;
    } else {
        option::printUsage(std::cerr,usage);
        return 1;
    }

    return 0;
}
