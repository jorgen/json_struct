/*
 * Copyright © 2012 Jørgen Lind

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

#ifndef JSON_TOKENIZER_H
#define JSON_TOKENIZER_H

#include <stddef.h>

class JsonTokenizerPrivate;

struct JsonToken
{
    enum Type {
        Error = 0,
        String = 1,
        Ascii = 2,
        Number = 3,
        ObjectStart = 4,
        ObjectEnd = 5,
        ArrayStart = 6,
        ArrayEnd = 7,
        Bool = 8,
        Null = 9
    };

    Type name_type;
    const char *name;
    size_t name_length;
    Type data_type;
    const char *data;
    size_t data_length;
};

typedef void (*ReleaseDataCallback)(const char *data, void *user_handle);

class JsonTokenizer
{
public:

    enum Error {
        NoError = 0,
        InvalidToken = -1,
        NeedMoreData = -2,
        ExpectedPropertyName = -3,
        IlligalPropertyName = -4,
        IlligalDataValue = -5,
        ExpectedDataToken = -6,
        EncounteredIlligalChar = -7
    };

    JsonTokenizer(ReleaseDataCallback release_data_callback = 0);
    ~JsonTokenizer();

    void addData(const char *data, size_t size, void *user_handle);
    void allowAsciiType(bool allow);
    void allowNewLineAsTokenDelimiter(bool allow);

    JsonTokenizer::Error nextToken(JsonToken *next_token);

private:
    JsonTokenizerPrivate *m_private;
};

#endif //JSON_TOKENIZER_H
