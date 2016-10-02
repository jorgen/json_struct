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
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>

class Configuration
{
public:
    explicit Configuration();

    void setInputFile(const char *input);
    bool hasInputFile() const;
    const std::string &inputFile() const;

    void setProperty(const char *property);
    bool hasProperty() const;
    const std::string &property() const;

    void setValue(const char *value);
    bool hasValue() const;
    const std::string &value()const;

    void setInline(bool shouldInline);
    bool hasInlineSet() const;

    void setCompactPrint(bool compactPrint);
    bool compactPrint() const;

    void setPrettyPrint(bool pretty);
    bool prettyPrint() const;

    void setCreateObject(bool create);
    bool createObject() const;

    void setPrintOnlyName(bool onlyName);
    bool printOnlyName() const;

    void setDelimiter(const std::string &delimiter);
    const std::string &delimiter() const;

    void setStrict(bool strict);
    bool strict() const;

    void setSilent(bool silent);
    bool silent() const;

    bool sane() const;
private:
    std::string m_input_file;
    std::string m_property;
    std::string m_value;
    std::string m_delimiter;

    bool m_inline;
    bool m_compact;
    bool m_pretty_print;
    bool m_create_object;
    bool m_print_only_name;
    bool m_strict;
    bool m_silent;
};

#endif //CONFIGURATION_H
