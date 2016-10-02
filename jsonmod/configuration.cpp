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
#include "configuration.h"

#include <limits.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>

#include <iostream>

#include <errno.h>

Configuration::Configuration()
    : m_delimiter(".")
    , m_inline(false)
    , m_compact(false)
    , m_pretty_print(false)
    , m_create_object(false)
    , m_print_only_name(false)
    , m_strict(false)
    , m_silent(false)
{

}

void Configuration::setInputFile(const char *input)
{
    m_input_file = input;
}

bool Configuration::hasInputFile() const
{
    return m_input_file.size() != 0;
}

const std::string &Configuration::inputFile() const
{
    return m_input_file;
}

void Configuration::setProperty(const char *property)
{
    m_property = property;
}

bool Configuration::hasProperty() const
{
    return m_property.size() != 0;
}

const std::string &Configuration::property() const
{
    return m_property;
}

void Configuration::setValue(const char *value)
{
    m_value = value;
}

bool Configuration::hasValue() const
{
    return m_value.size() != 0;
}

const std::string &Configuration::value() const
{
    return m_value;
}

void Configuration::setInline(bool shouldInline)
{
    m_inline = shouldInline;
}

bool Configuration::hasInlineSet() const
{
    return m_inline;
}

void Configuration::setCompactPrint(bool compactPrint)
{
    m_compact = compactPrint;
}

bool Configuration::compactPrint() const
{
    return m_compact;
}

void Configuration::setPrettyPrint(bool pretty)
{
    m_pretty_print = pretty;
}

bool Configuration::prettyPrint() const
{
    return m_pretty_print;
}

void Configuration::setCreateObject(bool create)
{
    m_create_object = create;
}
bool Configuration::createObject() const
{
    return m_create_object;
}

void Configuration::setPrintOnlyName(bool onlyName)
{
    m_print_only_name = onlyName;
}

bool Configuration::printOnlyName() const
{
    return m_print_only_name;
}

void Configuration::setDelimiter(const std::string &delimiter)
{
    m_delimiter = delimiter;
}

const std::string &Configuration::delimiter() const
{
    return m_delimiter;
}

void Configuration::setStrict(bool strict)
{
    m_strict = strict;
}

bool Configuration::strict() const
{
    return m_strict;
}

void Configuration::setSilent(bool silent)
{
    m_silent = silent;
}

bool Configuration::silent() const
{
    return m_silent;
}

bool Configuration::sane() const
{
    if (m_input_file.size()) {
        int input_file_access = m_inline ? W_OK : 0;
        input_file_access |= (R_OK | F_OK);

        if (access(m_input_file.c_str(),input_file_access)) {
            fprintf(stderr, "Error accessing input file\n%s\n\n", strerror(errno));
            return false;
        }
    }

    return true;
}
