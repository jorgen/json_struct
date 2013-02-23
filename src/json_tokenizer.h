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
#include <functional>

namespace JT {

class TokenizerPrivate;

struct Token
{
    enum Type {
        Error,
        String,
        Ascii,
        Number,
        ObjectStart,
        ObjectEnd,
        ArrayStart,
        ArrayEnd,
        Bool,
        Null
    };

    Type name_type;
    const char *name;
    size_t name_length;
    Type data_type;
    const char *data;
    size_t data_length;
};

enum class Error {
        NoError,
        InvalidToken,
        NeedMoreData,
        ExpectedPropertyName,
        ExpectedDelimiter,
        ExpectedDataToken,
        IlligalPropertyName,
        IlligalPropertyType,
        IlligalDataValue,
        EncounteredIlligalChar,
        CouldNotCreateNode,
        NodeNotFound,
        UnknownError
};

class Tokenizer
{
public:
    Tokenizer();
    ~Tokenizer();

    void allowAsciiType(bool allow);
    void allowNewLineAsTokenDelimiter(bool allow);

    void addData(const char *data, size_t size);
    size_t registered_buffers() const;
    void registerRelaseCallback(std::function<void(const char *)> callback);

    Error nextToken(Token *next_token);

    Tokenizer(const Tokenizer &other);
    Tokenizer(Tokenizer &&other);
    Tokenizer &operator=(const Tokenizer &rhs);
    Tokenizer &operator=(Tokenizer &&rhs);
private:
    TokenizerPrivate *m_private;
};

} //Namespace
#endif //JSON_TOKENIZER_H
