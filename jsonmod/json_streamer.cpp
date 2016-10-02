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
#include "json_streamer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <string.h>

JsonStreamer::JsonStreamer(const Configuration &config)
    : m_config(config)
    , m_error(false)
    , m_print_subtree(false)
    , m_current_depth(-1)
    , m_last_matching_depth(-1)
{
    if (config.hasInputFile()) {
        m_input_file = open(m_config.inputFile().c_str(), O_RDONLY|O_CLOEXEC);

        if (m_input_file < 0) {
            fprintf(stderr, "%s\n", strerror(errno));
            m_error = true;
            return;
        }
        if (lseek(m_input_file, 0,SEEK_SET) < 0) {
            m_error = true;
            return;
        }
    } else {
        m_input_file = STDIN_FILENO;
    }

    if (config.hasInlineSet() && config.hasInputFile()) {
        m_tmp_output = config.inputFile();
        m_tmp_output.append("XXXXXX");
        m_output_file = mkstemp(&m_tmp_output[0]);
        if (m_output_file == -1) {
            fprintf(stderr, "%s\n", strerror(errno));
            m_error = true;
            return;
        }
    } else {
        m_output_file = STDOUT_FILENO;
    }

    if (!m_config.hasProperty()) {
        m_print_subtree = true;
    }

    createPropertyVector();

    std::function<void(JT::Serializer *)> callback=
        std::bind(&JsonStreamer::requestFlushOutBuffer, this, std::placeholders::_1);

    m_tokenizer.allowNewLineAsTokenDelimiter(!m_config.strict());
    m_tokenizer.allowSuperfluousComma(!m_config.strict());
    m_serializer.addRequestBufferCallback(callback);

    setStreamerOptions(m_config.compactPrint());
}

JsonStreamer::~JsonStreamer()
{
    if (m_config.hasInputFile()) {
        close(m_input_file);
    }
    if (m_config.hasInlineSet() && m_config.hasInputFile()) {
        fsync(m_output_file);
        close(m_output_file);
        if (!m_error) {
            rename(m_tmp_output.c_str(), m_config.inputFile().c_str());
        } else if (m_tmp_output.length()) {
            if (unlink(m_tmp_output.c_str())) {
                fprintf(stderr, "%s\n", strerror(errno));
            }
        }
    }
}

void JsonStreamer::requestFlushOutBuffer(JT::Serializer *serializer)
{
    JT::SerializerBuffer buffer = serializer->buffers().front();
    serializer->clearBuffers();

    if (!(m_config.silent() && m_output_file == STDOUT_FILENO))
        writeOutBuffer(buffer);
    serializer->appendBuffer(buffer.buffer,buffer.size);
}

void JsonStreamer::stream()
{
    char in_buffer[4096];
    char out_buffer[4096];
    m_serializer.appendBuffer(out_buffer, sizeof out_buffer);
    ssize_t bytes_read;
    std::vector<bool> m_found_on_depth;
    while((bytes_read = read(m_input_file, in_buffer, 4096)) > 0) {
        m_tokenizer.addData(in_buffer,bytes_read);
        JT::Token token;
        JT::Error tokenizer_error;
        while ((tokenizer_error = m_tokenizer.nextToken(token)) == JT::Error::NoError) {
            bool print_token = false;
            bool finished_printing_subtree = false;
            if (!m_print_subtree && m_current_depth - 1 == m_last_matching_depth) {
                switch (token.name_type) {
                    case JT::Token::String:
                    case JT::Token::Ascii:
                        if (matchAtDepth(token.name)) {
                            m_last_matching_depth++;
                            if (m_last_matching_depth == m_property.size() - 1) {
                                print_token = true;
                                if (m_config.hasValue()) {
                                    token.value.data = m_config.value().c_str();
                                    token.value.size = m_config.value().size();
                                } else if (!m_config.createObject() && !m_config.printOnlyName()) {
                                    token.name.data = "";
                                    token.name.size = 0;
                                    token.name_type = JT::Token::Ascii;
                                    if (token.value_type == JT::Token::String) {
                                        token.value_type = JT::Token::Ascii;
                                        if (*token.value.data == '"') {
                                            token.value.data++;
                                            token.value.size -= 2;
                                        }
                                    }
                                }
                            }
                            m_found_on_depth.back() = true;
                        }
                        break;
                    default:
                        fprintf(stderr, "found invalid unrecognized type\n");
                        tokenizer_error = JT::Error::InvalidToken;
                        break;

                }
            }
            switch(token.value_type) {
                case JT::Token::ObjectStart:
                case JT::Token::ArrayStart:
                    m_current_depth++;
                    m_found_on_depth.push_back(false);
                    if (print_token && !m_config.createObject() && !m_config.printOnlyName()) {
                        if (m_config.hasValue()) {
                            fprintf(stderr, "Its not possible to change the value of and object or array\n");
                            m_error = true;
                            return;
                            break;
                        }
                        m_print_subtree = true;
                        setStreamerOptions(!m_config.prettyPrint());
                    }
                    break;
                case JT::Token::ObjectEnd:
                case JT::Token::ArrayEnd:
                    if (m_last_matching_depth == m_current_depth - 1
                            && m_found_on_depth.size() && !m_found_on_depth.back()) {
                        if (m_property.size() -1 == m_current_depth && m_config.hasValue()) {
                            JT::Token new_token;
                            new_token.name_type = JT::Token::String;
                            new_token.name.data = m_property.back().c_str();
                            new_token.name.size = m_property.back().size();
                            new_token.value_type = JT::Token::String;
                            new_token.value.data = m_config.value().c_str();
                            new_token.value.size = m_config.value().size();
                            m_serializer.write(new_token);
                        } else if (m_config.createObject()) {
                            for (size_t i = m_current_depth; i < m_property.size(); i++) {
                                JT::Token new_token;
                                new_token.name_type = JT::Token::String;
                                new_token.name.data = m_property[i].c_str();
                                new_token.name.size = m_property[i].size();
                                new_token.value_type = JT::Token::ObjectStart;
                                new_token.value.data = "{";
                                new_token.value.size = 1;
                                m_serializer.write(new_token);
                            }
                            for (size_t i = m_property.size(); i > m_current_depth; i--) {
                                JT::Token new_token;
                                new_token.name_type = JT::Token::Ascii;
                                new_token.name.data = "";
                                new_token.name.size = 0;
                                new_token.value_type = JT::Token::ObjectEnd;
                                new_token.value.data = "}";
                                new_token.value.size = 1;
                                m_serializer.write(new_token);
                            }
                        }
                    }

                    if (m_print_subtree && m_last_matching_depth == m_current_depth - 1) {
                        finished_printing_subtree = true;
                    }
                    if (m_current_depth - 1 == m_last_matching_depth) {
                        m_last_matching_depth--;
                    } else if (m_current_depth == m_last_matching_depth && m_found_on_depth.back()) {
                        m_last_matching_depth-=2;
                    }

                    m_found_on_depth.pop_back();
                    m_current_depth--;
                    break;
                default:
                    break;
            }

            if (m_config.printOnlyName() && print_token) {
                size_t written = write(m_output_file, token.name.data, token.name.size);
                written += write(m_output_file, " ", 1);
                if (written < token.name.size + 1) {
                    fprintf(stderr, "Error while writing to outbuffer :%s\n", strerror(errno));
                    m_error = true;
                }
            } else if (print_token || m_print_subtree || m_config.hasValue() || m_config.createObject()) {
                m_serializer.write(token);
            }

            if (finished_printing_subtree) {
                m_print_subtree = false;
                print_token = true;
                setStreamerOptions(m_config.compactPrint());
            }


        }
        if (tokenizer_error != JT::Error::NeedMoreData
                && tokenizer_error != JT::Error::NoError) {
            requestFlushOutBuffer(&m_serializer);
            char new_line[] = "\n";
            write(m_output_file, new_line, sizeof new_line - 1);
            fprintf(stderr, "Error while parsing json. \n%s\n", m_tokenizer.makeErrorString(tokenizer_error, m_tokenizer.currentErrorStringContext()).c_str());
            break;
        }
    }
    requestFlushOutBuffer(&m_serializer);
    char new_line[] = "\n";
    write(m_output_file, new_line, sizeof new_line - 1);
    if (bytes_read < 0) {
        fprintf(stderr, "Error while reading input %s\n", strerror(errno));
        m_error = true;
    }
}

void JsonStreamer::createPropertyVector()
{
    const std::string &property = m_config.property();
    size_t pos = 0;
    while (pos < property.size()) {
        size_t new_pos = property.find(m_config.delimiter(), pos);
        m_property.push_back(property.substr(pos, new_pos - pos));
        if (new_pos < std::string::npos - m_config.delimiter().size()) {
            pos = new_pos + m_config.delimiter().size();
        } else {
            pos = new_pos;
        }
    }
}

bool JsonStreamer::matchAtDepth(const JT::DataRef &data) const
{
    if (m_current_depth >= m_property.size())
        return false;
    const std::string &property = m_property[m_current_depth];
    if (property == "%{*}")
        return true;
    if (data.size != property.size())
        return false;
    if (memcmp(data.data, property.c_str(), data.size))
        return false;

    return true;
}

void JsonStreamer::writeOutBuffer(const JT::SerializerBuffer &buffer)
{
    size_t written = write(m_output_file,buffer.buffer,buffer.used);
    if (written < buffer.used) {
        fprintf(stderr, "Error while writing to outbuffer: %s\n", strerror(errno));
        m_error = true;
    }
}

void JsonStreamer::setStreamerOptions(bool compact)
{
    JT::SerializerOptions options = m_serializer.options();
    options.setPretty(!compact);
    options.skipDelimiter((m_config.hasProperty() && !m_config.hasValue() && !m_config.createObject()) && !compact);
    options.setAscii(m_config.hasProperty() && !compact);
    m_serializer.setOptions(options);
}

