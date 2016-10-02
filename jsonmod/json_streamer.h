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
#ifndef JSON_STREAMER_H
#define JSON_STREAMER_H

#include <json_tools.h>
#include "configuration.h"

#include <string>
#include <vector>

class JsonStreamer
{
public:
    JsonStreamer(const Configuration &config);
    ~JsonStreamer();

    bool error() const { return m_error; }

    void stream();

    void requestFlushOutBuffer(JT::Serializer *);
private:
    enum MatchingState {
        LookingForMatch,
        Matching,
        NoMatch
    };
    void createPropertyVector();
    bool matchAtDepth(const JT::DataRef &data) const;
    void writeOutBuffer(const JT::SerializerBuffer &buffer);

    void setStreamerOptions(bool compact);

    const Configuration &m_config;

    JT::Tokenizer m_tokenizer;
    JT::Serializer m_serializer;

    int m_input_file;
    int m_output_file;
    std::string m_tmp_output;

    std::vector<std::string> m_property;

    bool m_error;
    bool m_print_subtree;

    size_t m_current_depth;
    size_t m_last_matching_depth;
};

#endif //JSON_STREAMER_H
