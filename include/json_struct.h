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

/*! \file */

/*! \mainpage json_struct
 *
 * json_struct is a set of classes meant for simple and efficient parse,
 * tokenize and validate json.
 *
 * json_struct support parsing json into a stream of tokens using the \ref
 * tokenizer "JS::Tokenizer" api, or parsing json into c++ structures using the
 * \ref js_struct "JS_OBJECT" api.
 */

/*! \page tokenizer Parsing json using JS::Tokenizer
 *
 * Tokenizing json JS::Tokenizer can be used to extract tokens
 * from a json stream.  Tokens does not describe a full object, but only
 * key-value pairs. So a object would be: "some key" and object start. Then the
 * next token would be the first key value pair of that object. Then at some
 * point the object is finished, and an object end value with no key would be
 * the token.
 *
 * Arrays would be expressed in a similar fashion, but the tokens would have no
 * key, and each element in the array would be tokens with only a value
 * specified.
 *
 * The root object would be a token with no key data, but only object or array
 * start
 *
 * A crude example of this is viewed in \ref simple_tokenize.cpp here:
 * \include simple_tokenize.cpp
 *
 * Tokenizing json in this way allows you parse arbitrary large json data.
 * Also the tokenizer has mechanisms for asking for more data, making it easy
 * to stream json data. Using this interface to parse json is a bit verbose and
 * requires the application code to keep some extra state. json_struct also has
 * functionality for parsing json data directly into c++ structures. This is
 * done by adding some metadata to the structure, or by adding a template
 * specialisation of a class.  \ref js_struct "describes this" in more detail.
 */

/*! \example simple_tokenize.cpp
 *
 * This example show very basic usage of how JS::Tokenizer can be used
 */

/*! \example simple_struct.cpp
 *
 * This example shows basic usage of parsing Json directly into structs
 */

/*! \page js_struct Parsing json into C++ structs
 *
 * json_struct makes it very easy to put your json data into c++ structures or
 * take data from c++ structures and generate json.
 *
 * This is best shown with an example: \include simple_struct.cpp
 *
 * The two interesting sections here are the lines are the:
 * \code{.cpp}
 *    JS_OBJECT(JS_MEMBER(key),
 *              JS_MEMBER(number),
 *              JS_MEMBER(boolean));
 * \endcode
 *
 * and
 *
 * \code{.cpp}
 *    JS::ParseContext parseContext(json);
 *    JsonData dataStruct;
 *    parseContext.parseTo(dataStruct);
 * \endcode
 *
 * The JS_OBJECT call inside the JsonData struct will create a nested struct
 * declaration inside the JsonData struct. This nested struct will expose some
 * meta data about the struct, exposing the names of the members at runtime.
 * json_struct can then use this runtime information to populate the struct.
 *
 * Populating the struct is done by first creating a JS::ParseContext. The
 * JS::ParseContext contains a JS::Tokenizer. This tokenizer is what the actual
 * state holder for the parsing. If allowing using '\n' instead of ',' to
 * seperate object and array elements, then this should be set on the
 * JS::Tokenizer.
 *
 * Since types will dictate the schema of the input json, the JS::ParseContext
 * will expose a list containing what members where not populated by the input
 * json, and what member in the input json that did not have any member to
 * populate.
*/

#ifndef JSON_STRUCT_H
#define JSON_STRUCT_H

#include <stddef.h>
#include <functional>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <stdlib.h>
#include <memory>
#include <assert.h>
#include <atomic>

#if __cplusplus > 199711L || (defined(__MSC_VER) && __MSC_VER > 1800)
#define JS_STD_UNORDERED_MAP 1
#endif
#ifdef JS_STD_UNORDERED_MAP
#include <unordered_map>
#endif

#ifndef JS_STD_OPTIONAL
#if defined(__APPLE__)
#if __clang_major__ > 9 && __cplusplus >= 201703L
#define JS_STD_OPTIONAL 1
#endif
#elif __cplusplus >= 201703L
#define JS_STD_OPTIONAL 1
#endif
#endif

#ifdef JS_STD_OPTIONAL
#include <optional>
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4503)
#if _MSC_VER > 1800
#define JS_CONSTEXPR constexpr
#define JS_HAVE_CONSTEXPR 1
#else
#define JS_CONSTEXPR
#define JS_HAVE_CONSTEXPR 0
#endif
#else
#define JS_CONSTEXPR constexpr
#define JS_HAVE_CONSTEXPR 1
#endif

#if defined(min) || defined(max)

#error min or max macro is defined. Make sure these are not defined before including json_struct.h.\
 Use "#define NOMINMAX 1" before including Windows.h
#endif

#define JS_UNUSED(x) (void)(x)

#ifndef JS
#define JS JS
#endif

namespace JS {
/*!
 *  \brief Pointer to data
 *
 *  DataRef is used to refere to some data inside a json string. It holds the
 *  start posisition of the data, and its size.
 */
struct DataRef
{
    /*!
     * Constructs a null Dataref pointing to "" with size 0.
     */
    JS_CONSTEXPR explicit DataRef()
        : data("")
        , size(0)
    {}

    /*!
     * Constructs a DataRef pointing to data and size.
     * \param data points to start of data.
     * \param size size of data.
     */
    JS_CONSTEXPR explicit DataRef(const char *data, size_t size)
        : data(data)
        , size(size)
    {}

    /*!  Cobstructs a DataRef pointing to an array. This will \b NOT look for
     * the null terminator, but just initialize the DataRef to the size of the
     * array - 1. This function is intended to be used with string literals.
     * \param data  start of the data.
     */
    template <size_t N>
    JS_CONSTEXPR explicit DataRef(const char (&data)[N])
        : data(data)
        , size(N -1)
    {}

    explicit DataRef(const std::string &str)
        : data(&str[0])
        , size(str.size())
    {
    }

    explicit DataRef(const char *data)
        : data(data)
        , size(strlen(data))
    {
    }

    const char *data;
    size_t size;
};

enum class Type : unsigned char
{
    Error,
    String,
    Ascii,
    Number,
    ObjectStart,
    ObjectEnd,
    ArrayStart,
    ArrayEnd,
    Bool,
    Null,
    Verbatim
};

struct Token
{
    Token();

    DataRef name;
    DataRef value;
    Type name_type;
    Type value_type;
};

namespace Internal {
    struct IntermediateToken
    {
        IntermediateToken()
            : active(false)
            , name_type_set(false)
            , data_type_set(false)
        { }

        void clear() {
            if (!active)
                return;
            active = false;
            name_type_set = false;
            data_type_set = false;
            name_type = Type::Error;
            data_type = Type::Error;
            name.clear();
            data.clear();
        }

        bool active : 1;
        bool name_type_set : 1;
        bool data_type_set : 1;
        Type name_type = Type::Error;
        Type data_type = Type::Error;
        std::string name;
        std::string data;
    };
    enum Lookup
    {
        StrEndOrBackSlash = 1,
        AsciiLetters = 2,
        WhiteSpaceOrNull = 4,
        PlusOrMinus = 8,
        Digits = 16,
        HatUnderscoreAprostoph = 32,
        NumberEnd = 64
    };

    static inline const unsigned char * lookup()
    {
        static const unsigned char tmp[] =
        {
        /*0*/    4,      0,      0,      0,      0,      0,      0,      0,
        /*8*/    0,      4,      4,      0,      0,      4,      0,      0,
        /*16*/   0,      0,      0,      0,      0,      0,      0,      0,
        /*24*/   0,      0,      0,      0,      0,      0,      0,      0,
        /*32*/   4,      0,      1,      0,      0,      0,      0,      0,
        /*40*/   0,      0,      0,      8|64,   0,      8|64,   64,      0,
        /*48*/   16|64,  16|64,  16|64,  16|64,  16|64,  16|64,  16|64,  16|64,
        /*56*/   16|64,  16|64,  0,      0,      0,      0,      0,      0,
        /*64*/   0,      2,      2,      2,      2,      2|64,   2,      2,
        /*72*/   2,      2,      2,      2,      2,      2,      2,      2,
        /*80*/   2,      2,      2,      2,      2,      2,      2,      2,
        /*88*/   2,      2,      2,      0,      1,      32,     32,     32,
        /*96*/   0,      2,      2,      2,      2,      2|64,   2,      2,
        /*104*/  2,      2,      2,      2,      2,      2,      2,      2,
        /*112*/  2,      2,      2,      2,      2,      2,      2,      2,
        /*120*/  2,      2,      2,      0,      0,      0,      0,      0,
        /*128*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*136*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*144*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*152*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*160*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*168*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*176*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*184*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*192*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*200*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*208*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*216*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*224*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*232*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*240*/  0,      0,      0,      0,      0,      0,      0,      0,
        /*248*/  0,      0,      0,      0,      0,      0,      0,      0
        };
        return tmp;
    }
}

enum class Error : unsigned char
{
    NoError,
    NeedMoreData,
    InvalidToken,
    ExpectedPropertyName,
    ExpectedDelimiter,
    ExpectedDataToken,
    ExpectedObjectStart,
    ExpectedObjectEnd,
    ExpectedArrayStart,
    ExpectedArrayEnd,
    IllegalPropertyName,
    IllegalPropertyType,
    IllegalDataValue,
    EncounteredIllegalChar,
    NodeNotFound,
    MissingPropertyMember,
    MissingFunction,
    FailedToParseBoolean,
    FailedToParseDouble,
    FailedToParseFloat,
    FailedToParseInt,
    UnassignedRequiredMember,
    NonContigiousMemory,
    ScopeHasEnded,
    UnknownError,
    UserDefinedErrors
};

namespace Internal {
class ErrorContext
{
public:
    size_t line = 0;
    size_t character = 0;
    Error error = Error::NoError;
    std::string custom_message;
    std::vector<std::string> lines;

    void clear()
    {
        line = 0;
        character = 0;
        error = Error::NoError;
        lines.clear();
    }
};

    template<typename T>
    struct CallbackContainer;
}

template<typename T>
class RefCounter
{
public:
    RefCounter()
        : callbackContainer(nullptr)
        , index(0)
    {}

    RefCounter(size_t index, Internal::CallbackContainer<T> *callbackContainer)
        : callbackContainer(callbackContainer)
        , index(index)
    {
        inc();
    }
    RefCounter(const RefCounter<T> &other)
        : callbackContainer(other.callbackContainer)
    {
        inc();
    }

    RefCounter<T> &operator=(const RefCounter<T> &other)
    {
        dec();
        callbackContainer = other.callbackContainer;
        index = other.index;
        inc();
        return *this;
    }

    ~RefCounter()
    {
        dec();
    }

private:
    void inc();
    void dec();
    Internal::CallbackContainer<T> *callbackContainer;
    size_t index;
};

template<typename T>
class Callback
{
public:
    Callback()
        : ref(0)
    {}

    Callback(std::function<T> &callback)
        : ref(0)
        , callback(callback)
    {}
    Callback(const Callback<T> &other)
        : ref(other.ref.load())
        , callback(other.callback)
    {}
    Callback &operator=(const Callback<T> &other)
    {
        ref.store(other.ref.load());
        callback = other.callback;
        return *this;
    }

    void inc() { ++ref; }
    void dec() { --ref; }

    std::atomic<int> ref;
    std::function<T> callback;
};

namespace Internal {
    template<typename T>
    struct CallbackContainer
    {
    public:
        const RefCounter<T> addCallback(std::function<T> &callback)
        {
            for (size_t i = 0; i < vec.size(); i++)
            {
                if (vec[i].ref.load() == 0) {
                    vec[i].callback = callback;
                    return RefCounter<T>(i, this);
                }
            }
            vec.push_back(Callback<T>(callback));
            return RefCounter<T>(vec.size() - 1, this);
        }

        template<typename ...Ts>
        void invokeCallbacks(Ts&...args)
        {
            for (auto &callbackHandler : vec)
            {
                if (callbackHandler.ref.load())
                {
                    callbackHandler.callback(args...);
                }
            }
        }
        void inc(size_t index)
        {
            assert(index < vec.size());
            ++vec[index].ref;
        }
        void dec(size_t index)
        {
            assert(index < vec.size());
            assert(vec[index].ref.load() != 0);
            --vec[index].ref;
        }
    private:
        std::vector<Callback<T>> vec;
    };
    
    struct ScopeCounter
    {
        JS::Type type;
        uint16_t depth;
        inline void handleType(JS::Type in_type)
        {
            if (type == JS::Type::ArrayStart || type == JS::Type::ObjectStart)
            {
                if (in_type == type)
                    depth++;
                else if (in_type == JS::Type(static_cast<int>(type)+1))
                    depth--;
            } else {
                depth--;
            }
        }
    };
}

template<typename T>
inline void RefCounter<T>::inc()
{
    if(callbackContainer)
        callbackContainer->inc(index);
}

template<typename T>
inline void RefCounter<T>::dec()
{
    if(callbackContainer)
        callbackContainer->dec(index);
}

class Tokenizer;
typedef RefCounter<void(const char *)> ReleaseCBRef;
typedef RefCounter<void(Tokenizer &)> NeedMoreDataCBRef;

class Tokenizer
{
public:
    Tokenizer();

    void allowAsciiType(bool allow);
    void allowNewLineAsTokenDelimiter(bool allow);
    void allowSuperfluousComma(bool allow);

    void addData(const char *data, size_t size);
    template<size_t N>
    void addData(const char (&data)[N]);
    void addData(const std::vector<Token> *parsedData);
    size_t registeredBuffers() const;

    NeedMoreDataCBRef registerNeedMoreDataCallback(std::function<void(Tokenizer &)> callback);
    ReleaseCBRef registerReleaseCallback(std::function<void(const char *)> &callback);
    Error nextToken(Token &next_token);
    const char *currentPosition() const;

    void copyFromValue(const Token &token, std::string &to_buffer);
    void copyIncludingValue(const Token &token, std::string &to_buffer);

    void pushScope(JS::Type type);
    void popScope();
    JS::Error goToEndOfScope(JS::Token &token);

    std::string makeErrorString() const;
    void setErrorContextConfig(size_t lineContext, size_t rangeContext);
    Error updateErrorContext(Error error, const std::string &custom_message = std::string());
    const Internal::ErrorContext &errorContext() const { return error_context; }
private:
    enum class InTokenState : unsigned char
    {
        FindingName,
        FindingDelimiter,
        FindingData,
        FindingTokenEnd
    };

    enum class InPropertyState : unsigned char
    {
        NoStartFound,
        FindingEnd,
        FoundEnd
    };

    void resetForNewToken();
    void resetForNewValue();
    Error findStringEnd(const DataRef &json_data, size_t *chars_ahead);
    Error findAsciiEnd(const DataRef &json_data, size_t *chars_ahead);
    Error findNumberEnd(const DataRef &json_data, size_t *chars_ahead);
    Error findStartOfNextValue(Type *type,
                               const DataRef &json_data,
                               size_t *chars_ahead);
    Error findDelimiter(const DataRef &json_data, size_t *chars_ahead);
    Error findTokenEnd(const DataRef &json_data, size_t *chars_ahead);
    void requestMoreData();
    void releaseFirstDataRef();
    Error populateFromDataRef(DataRef &data, Type &type, const DataRef &json_data);
    static void populate_annonymous_token(const DataRef &data, Type type, Token &token);
    Error populateNextTokenFromDataRef(Token &next_token, const DataRef &json_data);

    InTokenState token_state = InTokenState::FindingName;
    InPropertyState property_state = InPropertyState::NoStartFound;
    Type property_type = Type::Error;
    bool is_escaped : 1;
    bool allow_ascii_properties : 1;
    bool allow_new_lines : 1;
    bool allow_superfluous_comma : 1;
    bool expecting_prop_or_annonymous_data : 1;
    bool continue_after_need_more_data : 1;
    size_t cursor_index;
    size_t current_data_start;
    size_t line_context;
    size_t line_range_context;
    size_t range_context;
    Internal::IntermediateToken intermediate_token;
    std::vector<DataRef> data_list;
    std::vector<Internal::ScopeCounter> scope_counter;
    std::vector<Type> container_stack;
    Internal::CallbackContainer<void(const char *)> release_callbacks;
    Internal::CallbackContainer<void(Tokenizer &)> need_more_data_callbacks;
    std::vector<std::pair<size_t, std::string *>> copy_buffers;
    const std::vector<Token> *parsed_data_vector;
    Internal::ErrorContext error_context;
};

class SerializerOptions
{
public:
    enum Style : unsigned char
    {
        Pretty,
        Compact
    };

    SerializerOptions(Style style = Style::Pretty);

    int shiftSize() const;

    Style style() const;
    void setStyle(Style style);

    bool convertAsciiToString() const;
    void setConvertAsciiToString(bool set);

    unsigned char depth() const;
    void setDepth(unsigned char depth);

    void skipDelimiter(bool skip);

    const std::string &prefix() const;
    const std::string &tokenDelimiter() const;
    const std::string &valueDelimiter() const;
    const std::string &postfix() const;

private:
    unsigned char m_shift_size;
    unsigned char m_depth;
    Style m_style;
    bool m_convert_ascii_to_string;

    std::string m_prefix;
    std::string m_token_delimiter;
    std::string m_value_delimiter;
    std::string m_postfix;
};

class SerializerBuffer
{
public:
    bool free() const { return size - used != 0; }
    bool append(const char *data, size_t size);
    char *buffer;
    size_t size;
    size_t used;
};

class Serializer;
typedef RefCounter<void(Serializer &)> BufferRequestCBRef;
class Serializer
{
public:
    Serializer();
    Serializer(char *buffer, size_t size);

    void appendBuffer(char *buffer, size_t size);
    void setOptions(const SerializerOptions &option);
    SerializerOptions options() const { return m_option; }

    bool write(const Token &token);
    bool write(const char *data, size_t size);
    bool write(const std::string &str) { return write(str.c_str(), str.size()); }

    const BufferRequestCBRef addRequestBufferCallback(std::function<void(Serializer &)> callback);
    const std::vector<SerializerBuffer> &buffers() const;
    void clearBuffers();
private:
    void askForMoreBuffers();
    void markCurrentSerializerBufferFull();
    bool writeAsString(const DataRef &data);
    bool write(Type type, const DataRef &data);

    Internal::CallbackContainer<void(Serializer &)> m_request_buffer_callbacks;
    std::vector <SerializerBuffer *> m_unused_buffers;
    std::vector <SerializerBuffer> m_all_buffers;

    bool m_first : 1;
    bool m_token_start : 1;
    SerializerOptions m_option;
};

// IMPLEMENTATION

inline Token::Token()
    : name()
    , value()
    , name_type(Type::String)
    , value_type(Type::String)
{

}

inline Tokenizer::Tokenizer()
    : is_escaped(false)
    , allow_ascii_properties(false)
    , allow_new_lines(false)
    , allow_superfluous_comma(false)
    , expecting_prop_or_annonymous_data(false)
    , continue_after_need_more_data(false)
    , cursor_index(0)
    , current_data_start(0)
    , line_context(4)
    , line_range_context(256)
    , range_context(38)
    , parsed_data_vector(nullptr)
{
    container_stack.reserve(16);
}

inline void Tokenizer::allowAsciiType(bool allow)
{
    allow_ascii_properties = allow;
}

inline void Tokenizer::allowNewLineAsTokenDelimiter(bool allow)
{
    allow_new_lines = allow;
}

inline void Tokenizer::allowSuperfluousComma(bool allow)
{
    allow_superfluous_comma = allow;
}
inline void Tokenizer::addData(const char *data, size_t data_size)
{
    data_list.push_back(DataRef(data, data_size));
}

template<size_t N>
inline void Tokenizer::addData(const char (&data)[N])
{
    data_list.push_back(DataRef(data));
}

inline void Tokenizer::addData(const std::vector<Token> *parsedData)
{
    assert(parsed_data_vector == 0);
    parsed_data_vector = parsedData;
    cursor_index = 0;
}

inline size_t Tokenizer::registeredBuffers() const
{
    return data_list.size();
}

inline NeedMoreDataCBRef Tokenizer::registerNeedMoreDataCallback(std::function<void(Tokenizer &)> callback)
{
    return need_more_data_callbacks.addCallback(callback);
}

inline ReleaseCBRef Tokenizer::registerReleaseCallback(std::function<void(const char *)> &callback)
{
    return release_callbacks.addCallback(callback);
}

inline Error Tokenizer::nextToken(Token &next_token)
{
    assert(!scope_counter.size() ||
            (scope_counter.back().type != JS::Type::ArrayEnd && scope_counter.back().type != JS::Type::ObjectEnd));
    if (scope_counter.size() && scope_counter.back().depth == 0)
    {
        return Error::ScopeHasEnded;
    }
    if (parsed_data_vector) {
        next_token = (*parsed_data_vector)[cursor_index];
        cursor_index++;
        if (cursor_index == parsed_data_vector->size()) {
            cursor_index = 0;
            parsed_data_vector = nullptr;
        }
        if (scope_counter.size())
            scope_counter.back().handleType(next_token.value_type);
        return Error::NoError;
    }
    if (data_list.empty()) {
        requestMoreData();
    }

    error_context.clear();

    if (data_list.empty()) {
        return Error::NeedMoreData;
    }

    if (!continue_after_need_more_data)
        resetForNewToken();

    Error error = Error::NeedMoreData;
    while (error == Error::NeedMoreData && data_list.size()) {
        const DataRef &json_data = data_list.front();
        error = populateNextTokenFromDataRef(next_token, json_data);

        if (error != Error::NoError && error != Error::NeedMoreData)
            updateErrorContext(error);

        if (error == Error::NeedMoreData) {
            releaseFirstDataRef();
            requestMoreData();
        }
    }

    continue_after_need_more_data = error == Error::NeedMoreData;
    if (error == JS::Error::NoError)
    {
        if (next_token.value_type == Type::ArrayStart || next_token.value_type == Type::ObjectStart)
            container_stack.push_back(next_token.value_type);
        if (next_token.value_type == Type::ArrayEnd)
        {
            assert(container_stack.size() && container_stack.back() == JS::Type::ArrayStart);
            container_stack.pop_back();
        }
        if (next_token.value_type == Type::ObjectEnd)
        {
            assert(container_stack.size() && container_stack.back() == JS::Type::ObjectStart);
            container_stack.pop_back();
        }
        if (scope_counter.size())
            scope_counter.back().handleType(next_token.value_type);
    }
    return error;
}

inline const char *Tokenizer::currentPosition() const
{
    if (parsed_data_vector)
        return reinterpret_cast<const char *>(cursor_index);

    if (data_list.empty())
        return nullptr;

    return data_list.front().data + cursor_index;
}

static bool isValueInIntermediateToken(const Token &token, const Internal::IntermediateToken &intermediate)
{
    if (intermediate.data.size())
        return token.value.data >= &intermediate.data[0] && token.value.data < &intermediate.data[0] + intermediate.data.size();
    return false;
}

inline void Tokenizer::copyFromValue(const Token &token, std::string &to_buffer)
{
    if (isValueInIntermediateToken(token, intermediate_token)) {
        std::string data(token.value.data, token.value.size);
        to_buffer += data;
        auto pair = std::make_pair(cursor_index, &to_buffer);
        copy_buffers.push_back(pair);
    } else {
        assert(token.value.data >= data_list.front().data && token.value.data < data_list.front().data + data_list.front().size);
        ptrdiff_t index = token.value.data - data_list.front().data;
        auto pair = std::make_pair(index, &to_buffer);
        copy_buffers.push_back(pair);
    }
}

inline void Tokenizer::copyIncludingValue(const Token &, std::string &to_buffer)
{
    auto it = std::find_if(copy_buffers.begin(), copy_buffers.end(), [&to_buffer] (const std::pair<size_t, std::string *> &pair) { return &to_buffer == pair.second; });
    assert(it != copy_buffers.end());
    assert(it->first <= cursor_index);
    if (cursor_index - it->first != 0)
        to_buffer.append(data_list.front().data + it->first, cursor_index - it->first);
    copy_buffers.erase(it);
}

inline void Tokenizer::pushScope(JS::Type type)
{
    scope_counter.push_back({type, 1});
    if (type != Type::ArrayStart && type != Type::ObjectStart)
        scope_counter.back().depth--;
}

inline void Tokenizer::popScope()
{
    assert(scope_counter.size() && scope_counter.back().depth == 0);
    scope_counter.pop_back(); 
}

inline JS::Error Tokenizer::goToEndOfScope(JS::Token &token)
{
    JS::Error error = JS::Error::NoError;
    while (scope_counter.back().depth && error == JS::Error::NoError)
    {
        error = nextToken(token);
    }
    return error;
}

namespace Internal
{
    static const char *error_strings[] = {
        "NoError",
        "NeedMoreData",
        "InvalidToken",
        "ExpectedPropertyName",
        "ExpectedDelimiter",
        "ExpectedDataToken",
        "ExpectedObjectStart",
        "ExpectedObjectEnd",
        "ExpectedArrayStart",
        "ExpectedArrayEnd",
        "IllegalPropertyName",
        "IllegalPropertyType",
        "IllegalDataValue",
        "EncounteredIllegalChar",
        "NodeNotFound",
        "MissingPropertyMember",
        "MissingFunction",
        "FailedToParseBoolean",
        "FailedToParseDouble",
        "FailedToParseFloat",
        "FailedToParseInt",
        "UnassignedRequiredMember",
        "NonContigiousMemory",
        "ScopeHasEnded",
        "UnknownError",
    };
}

inline std::string Tokenizer::makeErrorString() const
{
    static_assert(sizeof(Internal::error_strings) / sizeof*Internal::error_strings == size_t(Error::UserDefinedErrors) , "Please add missing error message");

    std::string retString("Error");
    if (error_context.error < Error::UserDefinedErrors)
        retString += std::string(" ") + Internal::error_strings[int(error_context.error)];
    if (error_context.custom_message.size())
        retString += " " + error_context.custom_message;
    retString += std::string(":\n");
    for (size_t i = 0; i < error_context.lines.size(); i++)
    {
        retString += error_context.lines[i] + "\n";
        if (i == error_context.line) {
            std::string pointing(error_context.character + 1, ' ');
            pointing[error_context.character - 1] = '^';
            pointing[error_context.character] = '\n';
            retString += pointing;
        }
    }
    return retString;
}
    
inline void Tokenizer::setErrorContextConfig(size_t lineContext, size_t rangeContext)
{
    line_context = lineContext;
    range_context = rangeContext;
}

inline void Tokenizer::resetForNewToken()
{
    intermediate_token.clear();
    resetForNewValue();
}

inline void Tokenizer::resetForNewValue()
{
    property_state = InPropertyState::NoStartFound;
    property_type = Type::Error;
    current_data_start = 0;
}

inline Error Tokenizer::findStringEnd(const DataRef &json_data, size_t *chars_ahead)
{
    size_t end = cursor_index;
    while (end < json_data.size) {
        if (is_escaped) {
            is_escaped = false;
            end++;
            continue;
        }
        while (end + 4 < json_data.size)
        {
            unsigned char lc = Internal::lookup()[(unsigned char)json_data.data[end]];
            if (lc == Internal::StrEndOrBackSlash)
                break;
            lc = Internal::lookup()[(unsigned char)json_data.data[++end]];
            if (lc == Internal::StrEndOrBackSlash)
                break;
            lc = Internal::lookup()[(unsigned char)json_data.data[++end]];
            if (lc == Internal::StrEndOrBackSlash)
                break;
            lc = Internal::lookup()[(unsigned char)json_data.data[++end]];
            if (lc  == Internal::StrEndOrBackSlash)
                break;
            end++;
        }
        if (end >= json_data.size)
            break;
        char c = json_data.data[end];
        if (c == '\\') {
            is_escaped = true;
        } else if (c == '"') {
            *chars_ahead = end + 1 - cursor_index;
            return Error::NoError;
        }
        end++;
    }
    return Error::NeedMoreData;
}

inline Error Tokenizer::findAsciiEnd(const DataRef &json_data, size_t *chars_ahead)
{
    assert(property_type == Type::Ascii);
    size_t end = cursor_index;
    while (end < json_data.size)
    {
        while (end + 4 < json_data.size)
        {
            unsigned char lc = Internal::lookup()[(unsigned char)json_data.data[end]];
            if (!(lc & (Internal::AsciiLetters | Internal::Digits | Internal::HatUnderscoreAprostoph)))
                break;
            lc = Internal::lookup()[(unsigned char)json_data.data[++end]];
            if (!(lc & (Internal::AsciiLetters | Internal::Digits | Internal::HatUnderscoreAprostoph)))
                break;
            lc = Internal::lookup()[(unsigned char)json_data.data[++end]];
            if (!(lc & (Internal::AsciiLetters | Internal::Digits | Internal::HatUnderscoreAprostoph)))
                break;
            lc = Internal::lookup()[(unsigned char)json_data.data[++end]];
            if (!(lc & (Internal::AsciiLetters | Internal::Digits | Internal::HatUnderscoreAprostoph)))
                break;
            end++;
        }

        char ascii_code = json_data.data[end];
        if ((ascii_code >= 'A' && ascii_code <= 'Z') ||
            (ascii_code >= '^' && ascii_code <= 'z') ||
            (ascii_code >= '0' && ascii_code <= '9')) {
            end++;
            continue;
        } else if (ascii_code == '\0') {
            *chars_ahead = end - cursor_index;
            return Error::NeedMoreData;
        } else {
            *chars_ahead = end - cursor_index;
            return Error::NoError;
        }
    }
    return Error::NeedMoreData;
}

inline Error Tokenizer::findNumberEnd(const DataRef &json_data, size_t *chars_ahead)
{
    size_t end = cursor_index;
    while(end + 4 < json_data.size) {
        unsigned char lc = Internal::lookup()[(unsigned char)json_data.data[end]];
        if (!(lc & (Internal::NumberEnd)))
            break;
        lc = Internal::lookup()[(unsigned char)json_data.data[++end]];
        if (!(lc & (Internal::NumberEnd)))
            break;
        lc = Internal::lookup()[(unsigned char)json_data.data[++end]];
        if (!(lc & (Internal::NumberEnd)))
            break;
        lc = Internal::lookup()[(unsigned char)json_data.data[++end]];
        if (!(lc & (Internal::NumberEnd)))
            break;
        end++;
    }
    while (end < json_data.size) {
        unsigned char lc = Internal::lookup()[(unsigned char)json_data.data[end]];
        if (lc & (Internal::NumberEnd)) {
            end++;
        } else {
            *chars_ahead = end - cursor_index;
            return Error::NoError;
        }
    }
    return Error::NeedMoreData;
}

inline Error Tokenizer::findStartOfNextValue(Type *type,
                                      const DataRef &json_data,
                                      size_t *chars_ahead)
{

    assert(property_state == InPropertyState::NoStartFound);

    for (size_t current_pos  = cursor_index; current_pos < json_data.size; current_pos++) {
        const char c = json_data.data[current_pos];
        unsigned char lc = Internal::lookup()[(unsigned char)c];
        if (c == '"') {
            *type = Type::String;
            *chars_ahead = current_pos - cursor_index;
            return Error::NoError;
        } else if (c == '{') {
            *type = Type::ObjectStart;
            *chars_ahead = current_pos - cursor_index;
            return Error::NoError;
        } else if (c == '}') {
            *type = Type::ObjectEnd;
            *chars_ahead = current_pos - cursor_index;
            return Error::NoError;
        } else if (c == '[') {
            *type = Type::ArrayStart;
            *chars_ahead = current_pos - cursor_index;
            return Error::NoError;
        } else if (c == ']') {
            *type = Type::ArrayEnd;
            *chars_ahead = current_pos - cursor_index;
            return Error::NoError;
        } else if (lc & (Internal::PlusOrMinus | Internal::Digits)) {
            *type = Type::Number;
            *chars_ahead = current_pos - cursor_index;
            return Error::NoError;
        } else if (lc & Internal::AsciiLetters) {
                *type = Type::Ascii;
                *chars_ahead = current_pos - cursor_index;;
                return Error::NoError;
        } else if (lc == 0) {
            *chars_ahead = current_pos - cursor_index;
            return Error::EncounteredIllegalChar;
        }
    }
    return Error::NeedMoreData;
}

inline Error Tokenizer::findDelimiter(const DataRef &json_data, size_t *chars_ahead)
{
    if (container_stack.empty())
        return Error::IllegalPropertyType;
    for (size_t end = cursor_index; end < json_data.size; end++) {
        const char c = json_data.data[end];
        if (c == ':') {
            if (container_stack.back() != Type::ObjectStart)
                return Error::ExpectedDelimiter;
            token_state = InTokenState::FindingData;
            *chars_ahead = end + 1 - cursor_index;
            return Error::NoError;
        } else if (c == ',') {
            if (container_stack.back() != Type::ArrayStart)
                return Error::ExpectedDelimiter;
            token_state = InTokenState::FindingName;
            *chars_ahead = end + 1 - cursor_index;
            return Error::NoError;
        } else if (c == ']') {
            if (container_stack.back() != Type::ArrayStart)
                return Error::ExpectedDelimiter;
            token_state = InTokenState::FindingName;
            *chars_ahead = end - cursor_index;
            return Error::NoError;
        } else if (!(Internal::lookup()[(unsigned char)c] & Internal::WhiteSpaceOrNull)) {
            return Error::ExpectedDelimiter;
        }
    }
    return Error::NeedMoreData;
}

inline Error Tokenizer::findTokenEnd(const DataRef &json_data, size_t *chars_ahead)
{
    if (container_stack.empty())
        return Error::NoError;
    for (size_t end = cursor_index; end < json_data.size; end++) {
        const char c = json_data.data[end];
        if (c == ',') {
            expecting_prop_or_annonymous_data = true;
            *chars_ahead = end + 1 - cursor_index;
            return Error::NoError;
        } else if (c == ']' || c == '}') {
            *chars_ahead = end - cursor_index;
            return Error::NoError;
        } else if (c == '\n') {
            if (allow_new_lines) {
                *chars_ahead = end + 1 - cursor_index;
                return Error::NoError;
            }
        } else if (Internal::lookup()[(unsigned char)c] & Internal::WhiteSpaceOrNull) {
            continue;
        
        } else {
            *chars_ahead = end + 1 - cursor_index;
            return Error::InvalidToken;
        }
    }
    return Error::NeedMoreData;
}

inline void Tokenizer::requestMoreData()
{
    need_more_data_callbacks.invokeCallbacks(*this);
}

inline void Tokenizer::releaseFirstDataRef()
{
    if (data_list.empty())
        return;

    const DataRef &json_data = data_list.front();

    for (auto &copy_pair : copy_buffers) {
        std::string data(json_data.data + copy_pair.first, json_data.size - copy_pair.first);
        *copy_pair.second += data;
        copy_pair.first = 0;
    }

    cursor_index = 0;
    current_data_start = 0;

    const char *data_to_release = json_data.data;
    data_list.erase(data_list.begin());
    release_callbacks.invokeCallbacks(data_to_release);
}

inline Error Tokenizer::populateFromDataRef(DataRef &data, Type &type, const DataRef &json_data)
{
    size_t diff = 0;
    Error error = Error::NoError;
    data.size = 0;
    data.data = json_data.data + cursor_index;
    if (property_state == InPropertyState::NoStartFound) {
        error = findStartOfNextValue(&type, json_data, &diff);
        if (error != Error::NoError) {
            type = Type::Error;
            return error;
        }

        data.data = json_data.data + cursor_index + diff;
        current_data_start = cursor_index + diff;
        if (type == Type::String) {
            data.data++;
            current_data_start++;
        }
        cursor_index += diff + 1;
        property_type = type;


        if (type == Type::ObjectStart || type == Type::ObjectEnd
            || type == Type::ArrayStart || type == Type::ArrayEnd) {
            data.size = 1;
            property_state = InPropertyState::FoundEnd;
        } else {
            property_state = InPropertyState::FindingEnd;
        }
    }

    size_t negative_size_adjustment = 0;
    if (property_state == InPropertyState::FindingEnd) {
        switch (type) {
        case Type::String:
            error = findStringEnd(json_data, &diff);
            negative_size_adjustment = 1;
            break;
        case Type::Ascii:
            error = findAsciiEnd(json_data, &diff);
            break;
        case Type::Number:
            error = findNumberEnd(json_data, &diff);
            break;
        default:
            return Error::InvalidToken;
        }

        if (error != Error::NoError) {
            return error;
        }

        cursor_index += diff;
        data.size =  cursor_index - current_data_start - negative_size_adjustment;
        property_state = InPropertyState::FoundEnd;
    }

    return Error::NoError;
}

inline void Tokenizer::populate_annonymous_token(const DataRef &data, Type type, Token &token)
{
    token.name = DataRef();
    token.name_type = Type::Ascii;
    token.value = data;
    token.value_type = type;
}

namespace Internal {
    static Type getType(Type type, const char *data, size_t length)
    {
        static const char m_null[] = "null";
        static const char m_true[] = "true";
        static const char m_false[] = "false";
        if (type != Type::Ascii)
            return type;
        if (sizeof(m_null) - 1 == length) {
            if (memcmp(m_null, data, length) == 0) {
                return Type::Null;
            }
            else if (memcmp(m_true, data, length) == 0) {
                return Type::Bool;
            }
        }
        if (sizeof(m_false) - 1 == length) {
            if (memcmp(m_false, data, length) == 0)
                return Type::Bool;
        }
        return Type::Ascii;
    }
}

inline Error Tokenizer::populateNextTokenFromDataRef(Token &next_token, const DataRef &json_data)
{
    Token tmp_token;
    while (cursor_index < json_data.size) {
        size_t diff = 0;
        DataRef data;
        Type type;
        Error error;
        switch (token_state) {
        case InTokenState::FindingName:
            type = intermediate_token.name_type;
            error = populateFromDataRef(data, type, json_data);
            if (error == Error::NeedMoreData) {
                if (property_state > InPropertyState::NoStartFound) {
                    intermediate_token.active = true;
                    size_t to_null = strnlen(data.data , json_data.size - current_data_start);
                    intermediate_token.name.append(data.data , to_null);
                    if (!intermediate_token.name_type_set) {
                        intermediate_token.name_type = type;
                        intermediate_token.name_type_set = true;
                    }
                }
                return error;
            } else if (error != Error::NoError) {
                return error;
            }

            if (intermediate_token.active) {
                intermediate_token.name.append(data.data, data.size);
                data = DataRef(intermediate_token.name);
                type = intermediate_token.name_type;
            }

            if (type == Type::ObjectEnd || type == Type::ArrayEnd
                || type == Type::ArrayStart || type == Type::ObjectStart) {
                switch (type) {
                case Type::ObjectEnd:
                case Type::ArrayEnd:
                    if (expecting_prop_or_annonymous_data && !allow_superfluous_comma) {
                        return Error::ExpectedDataToken;
                    }
                    populate_annonymous_token(data,type,next_token);
                    token_state = InTokenState::FindingTokenEnd;
                    return Error::NoError;

                case Type::ObjectStart:
                case Type::ArrayStart:
                    populate_annonymous_token(data,type,next_token);
                    expecting_prop_or_annonymous_data = false;
                    token_state = InTokenState::FindingName;
                    return Error::NoError;
                default:
                    return Error::UnknownError;
                }
            } else {
                tmp_token.name = data;
            }

            tmp_token.name_type = Internal::getType(type, tmp_token.name.data,
                                          tmp_token.name.size);
            token_state = InTokenState::FindingDelimiter;
            resetForNewValue();
            break;

        case InTokenState::FindingDelimiter:
            error = findDelimiter(json_data, &diff);
            if (error != Error::NoError) {
                if (intermediate_token.active == false) {
                    intermediate_token.name.append(tmp_token.name.data, tmp_token.name.size);
                    intermediate_token.name_type = tmp_token.name_type;
                    intermediate_token.active = true;
                }
                return error;
            }
            cursor_index += diff;
            resetForNewValue();
            expecting_prop_or_annonymous_data = false;
            if (token_state == InTokenState::FindingName) {
                populate_annonymous_token(tmp_token.name, tmp_token.name_type, next_token);
                return Error::NoError;
            } else {
                if (tmp_token.name_type != Type::String) {
                    if (!allow_ascii_properties || tmp_token.name_type != Type::Ascii) {
                        return Error::IllegalPropertyName;
                    }
                }
            }
            break;

        case InTokenState::FindingData:
            type = intermediate_token.data_type;
            error = populateFromDataRef(data, type, json_data);
            if (error == Error::NeedMoreData) {
                if (intermediate_token.active == false) {
                    intermediate_token.name.append(tmp_token.name.data, tmp_token.name.size);
                    intermediate_token.name_type = tmp_token.name_type;
                    intermediate_token.active = true;
                }
                if (property_state > InPropertyState::NoStartFound) {
                    size_t data_length = strnlen(data.data , json_data.size - current_data_start);
                    intermediate_token.data.append(data.data, data_length);
                    if (!intermediate_token.data_type_set) {
                        intermediate_token.data_type = type;
                        intermediate_token.data_type_set = true;
                    }
                }
                return error;
            } else if (error != Error::NoError) {
                return error;
            }

            if (intermediate_token.active) {
                intermediate_token.data.append(data.data, data.size);
                if (!intermediate_token.data_type_set) {
                    intermediate_token.data_type = type;
                    intermediate_token.data_type_set = true;
                }
                tmp_token.name = DataRef(intermediate_token.name);
                tmp_token.name_type = intermediate_token.name_type;
                data = DataRef(intermediate_token.data);
                type = intermediate_token.data_type;
            }

            tmp_token.value = data;
            tmp_token.value_type = Internal::getType(type, tmp_token.value.data, tmp_token.value.size);

            if (tmp_token.value_type  == Type::Ascii && !allow_ascii_properties)
                return Error::IllegalDataValue;

            if (type == Type::ObjectStart || type == Type::ArrayStart) {
                token_state = InTokenState::FindingName;
            } else {
                token_state = InTokenState::FindingTokenEnd;
            }
            next_token = tmp_token;
            return Error::NoError;
        case InTokenState::FindingTokenEnd:
            error = findTokenEnd(json_data, &diff);
            if (error != Error::NoError) {
                return error;
            }
            cursor_index += diff;
            token_state = InTokenState::FindingName;
            break;
        }
    }
    return Error::NeedMoreData;
}

namespace Internal {
    struct Lines
    {
        size_t start;
        size_t end;
    };
}

inline Error Tokenizer::updateErrorContext(Error error, const std::string &custom_message)
{
    error_context.error = error;
    error_context.custom_message = custom_message;
    if ((!parsed_data_vector || parsed_data_vector->empty()) && data_list.empty())
        return error;
    
    const DataRef json_data = parsed_data_vector && parsed_data_vector->size() ?
        DataRef(parsed_data_vector->front().value.data, size_t(parsed_data_vector->back().value.data - parsed_data_vector->front().value.data)) : data_list.front();
    size_t real_cursor_index = parsed_data_vector && parsed_data_vector->size() ?
        size_t(parsed_data_vector->at(cursor_index).value.data - json_data.data) : cursor_index;
    const size_t stop_back = real_cursor_index - std::min(real_cursor_index, line_range_context);
    const size_t stop_forward = std::min(real_cursor_index + line_range_context, json_data.size);
    std::vector<Internal::Lines> lines;
    lines.push_back({0, real_cursor_index});
    assert(real_cursor_index <= json_data.size);
    size_t lines_back = 0;
    size_t lines_forward = 0;
    size_t cursor_back;
    size_t cursor_forward;
    for (cursor_back = real_cursor_index - 1; cursor_back > stop_back; cursor_back--)
    {
        if (*(json_data.data + cursor_back) == '\n') {
            lines.front().start = cursor_back + 1;
            lines_back++;
            if (lines_back == 1)
                error_context.character = real_cursor_index - cursor_back;
            if (lines_back == line_context) {
                lines_back--;
                break;
            }

            lines.insert(lines.begin(), {0, cursor_back});
        }
    }
    if (lines.front().start == 0)
        lines.front().start = cursor_back;
    bool add_new_line = false;
    for (cursor_forward = real_cursor_index; cursor_forward < stop_forward; cursor_forward++)
    {
        if (add_new_line) {
            lines.push_back({cursor_forward, 0});
            add_new_line = false;
        }
        if (*(json_data.data + cursor_forward) == '\n') {
            lines.back().end = cursor_forward;
            lines_forward++;
            if (lines_forward == line_context)
                break;
            add_new_line = true;
        }
    }
    if (lines.back().end == 0)
        lines.back().end = cursor_forward - 1;

    if (lines.size() > 1) {
        error_context.lines.reserve(lines.size());
        for (auto &line : lines)
        {
            error_context.lines.push_back(std::string(json_data.data + line.start, line.end - line.start));
        }
        error_context.line = lines_back;
    } else {
        error_context.line = 0;

        size_t left = real_cursor_index > range_context ? real_cursor_index - range_context : 0;
        size_t right = real_cursor_index + range_context > json_data.size ? json_data.size : real_cursor_index + range_context;
        error_context.character = real_cursor_index - left;
        error_context.lines.push_back(std::string(json_data.data + left, right - left));
    }
    return error;
}

inline SerializerOptions::SerializerOptions(Style style)
    
    : m_shift_size(4)
    , m_depth(0)
    , m_style(style)
    , m_convert_ascii_to_string(true)
    , m_token_delimiter(",")
{
    m_value_delimiter = m_style == Pretty ? std::string(" : ") : std::string(":");
    m_postfix = m_style == Pretty ? std::string("\n") : std::string("");
}

inline int SerializerOptions::shiftSize() const { return m_shift_size; }

inline unsigned char SerializerOptions::depth() const { return m_depth; }

inline SerializerOptions::Style SerializerOptions::style() const { return m_style; }

inline bool SerializerOptions::convertAsciiToString() const
{
    return m_convert_ascii_to_string;
}

inline void SerializerOptions::setConvertAsciiToString(bool set)
{
    m_convert_ascii_to_string = set;
}

inline void SerializerOptions::setStyle(Style style)
{
    m_style = style;
    m_postfix = m_style == Pretty ? std::string("\n") : std::string("");
    m_value_delimiter = m_style == Pretty ? std::string(" : ") : std::string(":");
    setDepth(m_depth);
}

inline void SerializerOptions::skipDelimiter(bool skip)
{
    if (skip)
        m_token_delimiter = "";
    else
        m_token_delimiter = ",";
}

inline void SerializerOptions::setDepth(unsigned char depth)
{
    m_depth = depth;
    m_prefix = m_style == Pretty ? std::string(depth * m_shift_size, ' ') : std::string();
}

inline const std::string &SerializerOptions::prefix() const { return m_prefix; }
inline const std::string &SerializerOptions::tokenDelimiter() const { return m_token_delimiter; }
inline const std::string &SerializerOptions::valueDelimiter() const { return m_value_delimiter; }
inline const std::string &SerializerOptions::postfix() const { return m_postfix; }

inline bool SerializerBuffer::append(const char *data, size_t data_size)
{
    if (used + data_size > size)
        return false;

    memcpy(buffer + used, data, data_size);
    used += data_size;
    return true;
}

inline Serializer::Serializer()
    : m_first(true)
    , m_token_start(true)
{
}

inline Serializer::Serializer(char *buffer, size_t size)
    : m_first(true)
    , m_token_start(true)

{
    appendBuffer(buffer,size);
}

inline void Serializer::appendBuffer(char *buffer, size_t size)
{
    m_all_buffers.push_back({buffer,size,0});
    m_unused_buffers.push_back(&m_all_buffers.back());
}

inline void Serializer::setOptions(const SerializerOptions &option)
{
    m_option = option;
}

inline bool Serializer::write(const Token &in_token)
{
    const Token &token = in_token;
    if (!m_token_start) {
        if (token.value_type != Type::ObjectEnd
                && token.value_type != Type::ArrayEnd) {
            if (!write(m_option.tokenDelimiter()))
                return false;
        }
    }

    if (m_first) {
        m_first = false;
    } else {
        if (!write(m_option.postfix()))
            return false;
    }

    if (token.value_type == Type::ObjectEnd
            || token.value_type == Type::ArrayEnd) {
        m_option.setDepth(m_option.depth() - 1);
    }

    if (!write(m_option.prefix()))
        return false;

    if (token.name.size) {
        if (!write(token.name_type, token.name))
            return false;

        if (!write(m_option.valueDelimiter()))
            return false;
    }

    if (!write(token.value_type, token.value))
        return false;

    m_token_start = (token.value_type == Type::ObjectStart
            || token.value_type == Type::ArrayStart);
    if (m_token_start) {
        m_option.setDepth(m_option.depth() + 1);
    }
    return true;
}

inline const BufferRequestCBRef Serializer::addRequestBufferCallback(std::function<void(Serializer &)> callback)
{
    return m_request_buffer_callbacks.addCallback(callback);
}

inline const std::vector<SerializerBuffer> &Serializer::buffers() const
{
    return m_all_buffers;
}

inline void Serializer::clearBuffers()
{
    m_all_buffers.clear();
    m_unused_buffers.clear();
}

inline void Serializer::askForMoreBuffers()
{
    m_request_buffer_callbacks.invokeCallbacks(*this);
}

inline void Serializer::markCurrentSerializerBufferFull()
{
    m_unused_buffers.erase(m_unused_buffers.begin());
    if (m_unused_buffers.size() == 0)
        askForMoreBuffers();
}

inline bool Serializer::writeAsString(const DataRef &data)
{
    bool written;
    if (*data.data != '"') {
        written = write("\"",1);
        if (!written)
            return false;
    }

    written = write(data.data, data.size);
    if (!written)
        return false;

    if (*data.data != '"') {
        if (!written)
            return false;
        written = write("\"",1);
    }
    return written;
}

inline bool Serializer::write(Type type, const DataRef &data)
{
    bool written;
    switch (type) {
        case Type::String:
            written = writeAsString(data);
            break;
        case Type::Ascii:
            if (m_option.convertAsciiToString())
                written = writeAsString(data);
            else
                written = write(data.data,data.size);
            break;
        case Type::Null:
            written = write("null", 4);
            break;
        default:
            written = write(data.data,data.size);
            break;
    }
    return written;
}

inline bool Serializer::write(const char *data, size_t size)
{
    if(!size)
        return true;
    if (m_unused_buffers.size() == 0)
        askForMoreBuffers();
    size_t written = 0;
    while(m_unused_buffers.size() && written < size) {
        SerializerBuffer *first = m_unused_buffers.front();
        size_t free = first->free();
        if (!free) {
            markCurrentSerializerBufferFull();
            continue;
        }
        size_t to_write = std::min(size, free);
        first->append(data + written, to_write);
        written += to_write;
    }
    return written == size;
}

static JS::Error reformat(const char *data, size_t size, std::string &out, const SerializerOptions &options = SerializerOptions())
{
    Token token;
    Tokenizer tokenizer;
    tokenizer.addData(data,size);
    Error error = Error::NoError;

    Serializer serializer;
    serializer.setOptions(options);
    size_t last_pos = 0;
    auto cbref = serializer.addRequestBufferCallback([&out, &last_pos](Serializer &serializer_p)
                                          {
                                          size_t end = out.size();
                                          out.resize(end * 2);
                                          serializer_p.appendBuffer(&out[0] + end, end);
                                          last_pos = end;
                                          });
    if (out.empty())
        out.resize(4096);
    serializer.appendBuffer(&out[0], out.size());

    while (true)
    {
        error = tokenizer.nextToken(token);
        if (error != Error::NoError)
            break;
        serializer.write(token);
    }
    out.resize(last_pos + serializer.buffers().back().used);
    if (error == Error::NeedMoreData)
        return Error::NoError;

    return error;
}
static JS::Error reformat(const std::string &in, std::string &out, const SerializerOptions &options = SerializerOptions())
{
    return reformat(in.c_str(), in.size(), out, options);
}

//Tuple start
namespace Internal
{
    template<size_t...> struct Sequence { using type = Sequence; };

    template<typename A, typename B> struct Merge;
    template<size_t ... Is1, size_t ... Is2>
    struct Merge<Sequence<Is1...>, Sequence<Is2...>>
    {
        using type = Sequence<Is1..., (sizeof...(Is1)+Is2)...>;
    };

    template<size_t size> struct GenSequence;
    template<> struct GenSequence<0> { using type = Sequence<>; };
    template<> struct GenSequence<1> { using type = Sequence<0>; };
    template<size_t size>
    struct GenSequence
    {
        using type = typename Merge<typename GenSequence<size / size_t(2)>::type, typename GenSequence<size - size / size_t(2)>::type>::type;
    };

    template<size_t index, typename T>
    struct Element
    {
        Element()
            : data()
        {

        }

        Element(const T &t)
            : data(t)
        {}
        using type = T;
        T data;
    };

    template<typename A, typename ...Bs> struct TupleImpl;

    template<size_t ...indices, typename ...Ts>
    struct TupleImpl<Sequence<indices...>, Ts...> : public Element<indices, Ts>...
    {
        TupleImpl()
            : Element<indices, Ts>()...
        {}

        TupleImpl(Ts ...args)
            : Element<indices, Ts>(args)...
        {}
    };
}

template<size_t I, typename ...Ts>
struct TypeAt
{
        template<typename T>
        static Internal::Element<I, T> deduce(Internal::Element<I, T>);

        using tuple_impl = Internal::TupleImpl<typename Internal::GenSequence<sizeof...(Ts)>::type, Ts...>;
        using element = decltype(deduce(tuple_impl()));
        using type = typename element::type;
};

template<typename ...Ts>
struct Tuple
{
        Tuple()
                : impl()
        {}

        Tuple(Ts ...args)
                : impl(args...)
        {}

        using Seq = typename Internal::GenSequence<sizeof...(Ts)>::type;
        Internal::TupleImpl<Seq, Ts...> impl;
        static JS_CONSTEXPR const size_t size = sizeof...(Ts);

        template<size_t Index>
        const typename TypeAt<Index, Ts...>::type &get() const
        {
                return static_cast<const typename TypeAt<Index, Ts...>::element&>(impl).data;
        }

        template<size_t Index>
        typename TypeAt<Index, Ts...>::type &get()
        {
                return static_cast<typename TypeAt<Index, Ts...>::element&>(impl).data;
        }
};

/// \private
template<size_t I, typename ...Ts>
struct TypeAt<I, const Tuple<Ts...>>
{
        template<typename T>
        static Internal::Element<I, T> deduce(Internal::Element<I, T>);

        using tuple_impl = Internal::TupleImpl<typename Internal::GenSequence<sizeof...(Ts)>::type, Ts...>;
        using element = decltype(deduce(tuple_impl()));
        using type = typename element::type;
};

/// \private
template<size_t I, typename ...Ts>
struct TypeAt<I, Tuple<Ts...>>
{
        template<typename T>
        static Internal::Element<I, T> deduce(Internal::Element<I, T>);

        using tuple_impl = Internal::TupleImpl<typename Internal::GenSequence<sizeof...(Ts)>::type, Ts...>;
        using element = decltype(deduce(tuple_impl()));
        using type = typename element::type;
};

/*!  \private
 */
template<>
struct Tuple<>
{
        static JS_CONSTEXPR const size_t size = 0;
};

template<typename ...Ts>
Tuple<Ts...> makeTuple(Ts ...args)
{
        return Tuple<Ts...>(args...);
}
//Tuple end

template<typename T>
struct Nullable
{
    Nullable()
        : data()
    {
    }
    Nullable(const T &t)
        : data(t)
    {
    }

    Nullable(const Nullable<T> &t)
        : data(t.data)
    {
    }

    Nullable<T> &operator= (const T &other)
    {
        data = other;
        return *this;
    }

    T data;
    T &operator()() { return data; }
    const T &operator()() const { return data; }
};

template<typename T>
struct NullableChecked
{
    NullableChecked()
        : data()
        , null(true)
    { }
    NullableChecked(const T &t)
        : data(t)
        , null(false)
    { }
    NullableChecked(const NullableChecked<T> &t)
        : data(t.data)
        , null(t.null)
    { }
    NullableChecked<T> &operator= (const T &other)
    {
        data = other;
        null = false;
        return *this;
    }

    T &operator()() { return data; }
    const T &operator()() const { return data; }
    T data;
    bool null;
};

template<typename T>
struct Optional
{
    Optional()
        : data()
    {
    }
    Optional(const T &t)
        : data(t)
    {
    }

    Optional(const Optional<T> &t)
        : data(t.data)
    {
    }
    Optional<T> &operator= (const T &other)
    {
        data = other;
        return *this;
    }

    T data;
    T &operator()() { return data; }
    const T &operator()() const { return data; }
    typedef bool IsOptionalType;
};

template<typename T>
struct OptionalChecked
{
    OptionalChecked()
        : data()
        , assigned(false)
    { }
    OptionalChecked(const T &t)
        : data(t)
        , assigned(true)
    { }
    OptionalChecked(const OptionalChecked<T> &t)
        : data(t.data)
        , assigned(t.assigned)
    { }
    OptionalChecked<T> &operator= (const T &other)
    {
        data = other;
        assigned = true;
        return *this;
    }

    T &operator()() { return data; }
    const T &operator()() const { return data; }
    T data;
    bool assigned;
    typedef bool IsOptionalType;
};

struct SilentString
{
    std::string data;
    typedef bool IsOptionalType;
};

template<typename T, typename A = std::allocator<T>>
struct SilentVector
{
    std::vector<T, A> data;
    typedef bool IsOptionalType;
};

template<typename T, typename Deleter = std::default_delete<T>>
struct SilentUniquePtr
{
    std::unique_ptr<T, Deleter> data;
    typedef bool IsOptionalType;
};

struct JsonObjectRef
{
    DataRef ref;
};

struct JsonObject
{
    std::string data;
};

struct JsonArrayRef
{
    DataRef ref;
};

struct JsonArray
{
    std::string data;
};

struct JsonObjectOrArrayRef
{
    DataRef ref;
};

struct JsonObjectOrArray
{
    std::string data;
};

struct JsonTokens
{
    std::vector<JS::Token> data;
};

struct JsonMeta
{
    JsonMeta(size_t pos, bool is_array)
        : position(pos)
          , size(1)
          , skip(1)
          , children(0)
          , complex_children(0)
          , is_array(is_array)
          , has_data(false)
    {}

    size_t position;
    unsigned int size;
    unsigned int skip;
    unsigned int children;
    unsigned int complex_children;
    bool is_array : 1;
    bool has_data : 1;
};

static inline std::vector<JsonMeta> metaForTokens(const JsonTokens &tokens)
{
    std::vector<JsonMeta> meta;
    meta.reserve(tokens.data.size() / 4);
    std::vector<size_t> parent;
    for (size_t i = 0; i < tokens.data.size(); i++)
    {
        for (size_t parent_index : parent) {
            meta[parent_index].size++;
        }
        const JS::Token &token = tokens.data.at(i);
        if (token.value_type == Type::ArrayEnd
            || token.value_type == Type::ObjectEnd)
        {
            assert(parent.size());
            assert(meta[parent.back()].is_array == (token.value_type == Type::ArrayEnd));
            parent.pop_back();
        }
        else {
            if (parent.size())
                meta[parent.back()].children++;
        }

        if (token.value_type == Type::ArrayStart
            || token.value_type == Type::ObjectStart)
        {
            if (parent.size())
                meta[parent.back()].complex_children++;
             for (size_t parent_index: parent) {
                meta[parent_index].skip++;
            }
            meta.push_back(JsonMeta(i, token.value_type == Type::ArrayStart));
            parent.push_back(meta.size()-1);
        }
        else if (token.value_type != JS::Type::ArrayEnd && token.value_type != JS::Type::ObjectEnd)
        {
            for (size_t parent_index : parent) {
                meta[parent_index].has_data = true;
            }
        }
    }
    assert(!parent.size()); // This assert may be triggered when JSON is invalid (e.g. when creating a DiffContext).
    return meta;
}

namespace Internal
{
    static size_t findFirstChildWithData(const std::vector<JsonMeta> &meta_vec, size_t start_index)
    {
        const JsonMeta &meta = meta_vec[start_index];
        if (!meta.has_data)
            return size_t(-1);

        size_t skip_size = 0;
        for (unsigned int i = 0; i < meta.complex_children; i++)
        {
            auto &current_child = meta_vec[start_index + skip_size + 1];
            skip_size += current_child.skip;
            if (current_child.has_data)
                return i;
        }
        return -1;
    }
}

template <typename T>
struct IsOptionalType {
    typedef char yes[1];
    typedef char no[2];

    template <typename C>
    static JS_CONSTEXPR yes& test_in_optional(typename C::IsOptionalType*);

    template <typename>
    static JS_CONSTEXPR no& test_in_optional(...);

    static JS_CONSTEXPR const bool value = sizeof(test_in_optional<T>(0)) == sizeof(yes);
};

/// \private
template <typename T>
struct IsOptionalType<std::unique_ptr<T>>
{
    static JS_CONSTEXPR const bool value = true;
};

#ifdef JS_STD_OPTIONAL
/// \private
template <typename T>
struct IsOptionalType<std::optional<T>>
{
    static JS_CONSTEXPR const bool value = true;
};
#endif

struct ParseContext
{
    ParseContext()
    {}
    explicit ParseContext(const char *data, size_t size)
    {
        tokenizer.addData(data, size);
    }

    explicit ParseContext(const char *data)
    {
        size_t size = strlen(data);
        tokenizer.addData(data, size);
    }

    explicit ParseContext(const std::string &data)
    {
        tokenizer.addData(&data[0], data.size());
    }

    template<typename T>
    explicit ParseContext(const char *data, size_t size, T &to_type)
    {
        tokenizer.addData(data, size);
        parseTo(to_type);

    }
    template<size_t SIZE>
    explicit ParseContext(const char (&data)[SIZE])
    {
        tokenizer.addData(data);
    }

    template<typename T>
    Error parseTo(T &to_type);

    Error nextToken()
    {
        error = tokenizer.nextToken(token);
        return error;
    }

    std::string makeErrorString() const
    {
        return tokenizer.makeErrorString();
    }

    Tokenizer tokenizer;
    Token token;
    Error error = Error::NoError;
    std::vector<std::string> missing_members;
    std::vector<std::string> unassigned_required_members;
    bool allow_missing_members = true;
    bool allow_unnasigned_required_members = true;
};

/*! \def JS_MEMBER
 *
 * Create meta information of the member with the same name as
 * the member.
 */
/*! \def JS_MEMBER_ALIASES
 *
 * Create meta information where the primary name is the same as the member and
 * the subsequent names are aliases.
 */
/*! \def JS_MEMBER_WITH_NAME
 *
 * Create meta information where the primary name is argument name, and the subsequent
 * names are aliases.
 */
/*! \def JS_MEMBER_WITH_NAME_AND_ALIASES
 *
 * Creates meta information where the primary name is argument name, a
 * and subsequent names are aliases
 */

/*! \def JS_SUPER_CLASS
 *
 * Creates superclass meta data which is used inside the JS_SUPER_CLASSES macro
 */

/*! \def JS_SUPER_CLASSES
 *
 * Macro to contain the super class definitions
 */

namespace Internal
{
template <typename T>
struct HasJsonStructBase {
    typedef char yes[1];
    typedef char no[2];

    template <typename C>
        static JS_CONSTEXPR yes& test_in_base(typename C::template JsonStructBase<C>*);

    template <typename>
        static JS_CONSTEXPR no& test_in_base(...);
};

template<typename JS_BASE_STRUCT_T, typename JS_OBJECT_T>
struct JsonStructBaseDummy
{
    static_assert(sizeof(HasJsonStructBase<JS_OBJECT_T>::template test_in_base<JS_OBJECT_T>(nullptr)) == sizeof(typename HasJsonStructBase<JS_OBJECT_T>::yes), "Missing JS_OBJECT JS_OBJECT_EXTERNAL or TypeHandler specialisation\n");
    using TT = typename std::remove_reference<decltype(JS_OBJECT_T::template JsonStructBase<JS_OBJECT_T>::js_static_meta_data_info())>::type;
    using ST = typename std::remove_reference<decltype(JS_OBJECT_T::template JsonStructBase<JS_OBJECT_T>::js_static_meta_super_info())>::type;
    static const TT &js_static_meta_data_info()
    {
        return JS_OBJECT_T::template JsonStructBase<JS_OBJECT_T>::js_static_meta_data_info();
    }

    static const ST &js_static_meta_super_info()
    {
        return JS_OBJECT_T::template JsonStructBase<JS_OBJECT_T>::js_static_meta_super_info();
    }
};
}

#define JS_MEMBER(member) JS::makeMemberInfo(#member, &JS_OBJECT_T::member)
#define JS_MEMBER_ALIASES(member, ...) JS::makeMemberInfo(#member, &JS_OBJECT_T::member, __VA_ARGS__)
#define JS_MEMBER_WITH_NAME(member, name) JS::makeMemberInfo(name, &JS_OBJECT_T::member)
#define JS_MEMBER_WITH_NAME_AND_ALIASES(member, name, ...) JS::makeMemberInfo(name, &JS_OBJECT_T::member, __VA_ARGS__)

#define JS_SUPER_CLASS(super) JS::Internal::SuperInfo<super>(JS::DataRef(#super))

#define JS_SUPER_CLASSES(...) JS::makeTuple(__VA_ARGS__)

#define JS_OBJECT(...) \
    template<typename JS_OBJECT_T> \
    struct JsonStructBase \
    { \
       using TT = decltype(JS::makeTuple(__VA_ARGS__)); \
       static const TT &js_static_meta_data_info() \
       { static auto ret = JS::makeTuple(__VA_ARGS__); return ret; } \
       static const decltype(JS::makeTuple()) &js_static_meta_super_info() \
       { static auto ret = JS::makeTuple(); return ret; } \
    }

#define JS_OBJECT_WITH_SUPER(super_list, ...) \
    template<typename JS_OBJECT_T> \
    struct JsonStructBase \
    { \
       using TT = decltype(JS::makeTuple(__VA_ARGS__)); \
       static const TT &js_static_meta_data_info() \
       { static auto ret = JS::makeTuple(__VA_ARGS__); return ret; } \
        static const decltype(super_list) &js_static_meta_super_info() \
        { static auto ret = super_list; return ret; } \
    }

#define JS_OBJECT_EXTERNAL(Type, ...) \
    namespace JS {\
    namespace Internal {\
    template<typename JS_OBJECT_T> \
    struct JsonStructBaseDummy<Type, JS_OBJECT_T> \
    { \
        using TT = decltype(JS::makeTuple(__VA_ARGS__)); \
        static const TT &js_static_meta_data_info() \
          { static auto ret = JS::makeTuple(__VA_ARGS__); return ret; } \
       static const decltype(JS::makeTuple()) &js_static_meta_super_info() \
       { static auto ret = JS::makeTuple(); return ret; } \
    };\
    }}

#define JS_OBJECT_EXTERNAL_WITH_SUPER(Type,super_list, ...) \
    namespace JS {\
    namespace Internal {\
    template<typename JS_OBJECT_T> \
    struct JsonStructBaseDummy<Type, JS_OBJECT_T> \
    { \
        using TT = decltype(JS::makeTuple(__VA_ARGS__)); \
        static const TT &js_static_meta_data_info() \
          { static auto ret = JS::makeTuple(__VA_ARGS__); return ret; } \
        static const decltype(super_list) &js_static_meta_super_info() \
        { static auto ret = super_list; return ret; } \
    };\
    }}

/*!
 * \private
 */
template<typename T, typename U, size_t NAME_COUNT>
struct MI
{
    DataRef name[NAME_COUNT];
    T U::* member;
    typedef T type;
};

namespace Internal {
    template<typename T, typename U, size_t NAME_COUNT>
    using MemberInfo = MI <T, U, NAME_COUNT>;

    template<typename T>
    struct SuperInfo
    {
        explicit
            SuperInfo(const DataRef &name)
            : name(name)
            {}
        const DataRef name;
        typedef T type;
    };
}

template<typename T, typename U, size_t NAME_SIZE, typename ...Aliases>
JS_CONSTEXPR const MI<T, U, sizeof...(Aliases) + 1> makeMemberInfo(const char(&name)[NAME_SIZE], T U::* member, Aliases ... aliases)
{
    return { {DataRef(name), DataRef(aliases)... }, member };
}

template<typename T>
struct TypeHandler
{
    static inline Error to(T &to_type, ParseContext &context);
    static inline void from(const T &from_type, Token &token, Serializer &serializer);
};

namespace Internal {
    template<typename T, typename MI_T, typename MI_M, size_t MI_NC>
    inline Error unpackMember(T &to_type, const MemberInfo<MI_T, MI_M, MI_NC> &memberInfo, ParseContext &context, size_t index, bool primary, bool *assigned_members)
    {
        if (primary)
        {
            if (memberInfo.name[0].size == context.token.name.size && memcmp(memberInfo.name[0].data, context.token.name.data, context.token.name.size) == 0)
            {
#if JS_HAVE_CONSTEXPR
                assigned_members[index] = true;
#endif
                return TypeHandler<MI_T>::to(to_type.*memberInfo.member, context);
            }
        } else {
            for (size_t start = 1; start < MI_NC; start++) {
                if (memberInfo.name[start].size == context.token.name.size && memcmp(memberInfo.name[start].data, context.token.name.data, context.token.name.size) == 0)
                {
#if JS_HAVE_CONSTEXPR
                    assigned_members[index] = true;
#endif
                    return TypeHandler<MI_T>::to(to_type.*memberInfo.member, context);
                }
            }
        }
        return Error::MissingPropertyMember;
    }

    template<typename MI_T, typename MI_M, size_t MI_NC>
    inline Error verifyMember(const MemberInfo<MI_T, MI_M, MI_NC> &memberInfo, size_t index, bool *assigned_members, std::vector<std::string> &missing_members, const char *super_name)
    {
#if JS_HAVE_CONSTEXPR
        if (assigned_members[index])
            return Error::NoError;
        if (IsOptionalType<MI_T>::value)
            return Error::NoError;

        std::string to_push = strlen(super_name) ? std::string(super_name) + "::" : std::string();
        to_push += std::string(memberInfo.name[0].data, memberInfo.name[0].size);
        missing_members.push_back(to_push);
        return Error::UnassignedRequiredMember;
#else
        return Error::NoError;
#endif
    }

    template<typename T, typename MI_T, typename MI_M, size_t MI_NC>
    inline void serializeMember(const T &from_type, const MemberInfo<MI_T, MI_M, MI_NC> &memberInfo, Token &token, Serializer &serializer, const char *super_name)
    {
        JS_UNUSED(super_name);
        token.name.data = memberInfo.name[0].data;
        token.name.size = memberInfo.name[0].size;
        token.name_type = Type::Ascii;

        TypeHandler<MI_T>::from(from_type.*memberInfo.member, token, serializer);
    }

    template<typename T, size_t PAGE, size_t INDEX>
    struct SuperClassHandler
    {
        static Error handleSuperClasses(T &to_type, ParseContext &context, bool primary, bool *assigned_members);
        static Error verifyMembers(bool *assigned_members, std::vector<std::string> &missing_members);
        static JS_CONSTEXPR size_t membersInSuperClasses();
        static void serializeMembers(const T &from_type, Token &token, Serializer &serializer);
    };

    template<typename T, size_t PAGE, size_t SIZE>
    struct StartSuperRecursion
    {
        static Error start(T &to_type, ParseContext &context, bool primary, bool *assigned)
        {
            return SuperClassHandler<T, PAGE, SIZE - 1>::handleSuperClasses(to_type, context, primary, assigned);
        }

        static Error verifyMembers(bool *assigned_members, std::vector<std::string> &missing_members)
        {
            return SuperClassHandler<T, PAGE, SIZE - 1>::verifyMembers(assigned_members, missing_members);
        }

        static JS_CONSTEXPR size_t membersInSuperClasses()
        {
            return SuperClassHandler<T, PAGE, SIZE - 1>::membersInSuperClasses();
        }

        static void serializeMembers(const T &from_type, Token &token, Serializer &serializer)
        {
            return SuperClassHandler<T, PAGE, SIZE - 1>::serializeMembers(from_type, token, serializer);
        }
    };

    template<typename T, size_t PAGE>
    JS_CONSTEXPR size_t memberCount()
    {
        using Members = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<T,T>::js_static_meta_data_info())>::type;
        using SuperMeta = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<T,T>::js_static_meta_super_info())>::type;
        return Members::size + StartSuperRecursion<T, PAGE + Members::size, SuperMeta::size>::membersInSuperClasses();
    }

    template<typename T, size_t PAGE>
    struct StartSuperRecursion<T, PAGE, 0>
    {
        static Error start(T &to_type, ParseContext &context, bool primary, bool *assigned)
        {
            JS_UNUSED(to_type);
            JS_UNUSED(context);
            JS_UNUSED(primary);
            JS_UNUSED(assigned);
            return Error::MissingPropertyMember;
        }

        static Error verifyMembers(bool *assigned_members, std::vector<std::string> &missing_members)
        {
            JS_UNUSED(assigned_members);
            JS_UNUSED(missing_members);
            return Error::NoError;
        }

        static JS_CONSTEXPR size_t membersInSuperClasses()
        {
            return 0;
        }

        static void serializeMembers(const T &from_type, Token &token, Serializer &serializer)
        {
            JS_UNUSED(from_type);
            JS_UNUSED(token);
            JS_UNUSED(serializer);
        }
    };

    template<typename T, typename Members, size_t PAGE, size_t INDEX>
    struct MemberChecker
    {
        static Error unpackMembers(T &to_type, const Members &members, ParseContext &context, bool primary, bool *assigned_members)
        {
            Error error = unpackMember(to_type, members.template get<INDEX>(), context, PAGE + INDEX, primary, assigned_members);
            if (error != Error::MissingPropertyMember)
                return error;

            return MemberChecker<T, Members, PAGE, INDEX - 1>::unpackMembers(to_type, members, context, primary, assigned_members);
        }

        static Error verifyMembers(const Members &members, bool *assigned_members, std::vector<std::string> &missing_members, const char *super_name)
        {
            Error memberError = verifyMember(members.template get<INDEX>(), PAGE + INDEX, assigned_members, missing_members, super_name);
            Error error = MemberChecker<T, Members, PAGE, INDEX - 1>::verifyMembers(members, assigned_members, missing_members, super_name);
            if (memberError != Error::NoError)
                return memberError;
            return error;
        }
        static void serializeMembers(const T &from_type, const Members &members, Token &token, Serializer &serializer, const char *super_name)
        {
            serializeMember(from_type, members.template get<Members::size - INDEX - 1>(), token, serializer, super_name);
            MemberChecker<T, Members, PAGE, INDEX - 1>::serializeMembers(from_type, members, token, serializer, super_name);
        }
    };

    template<typename T, typename Members, size_t PAGE>
    struct MemberChecker<T, Members, PAGE, 0>
    {
        static Error unpackMembers(T &to_type, const Members &members, ParseContext &context, bool primary, bool *assigned_members)
        {
            Error error = unpackMember(to_type, members.template get<0>(), context, PAGE, primary, assigned_members);
            if (error != Error::MissingPropertyMember)
                return error;

            using Super = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<T, T>::js_static_meta_super_info())>::type;
            return StartSuperRecursion<T, PAGE + Members::size, Super::size>::start(to_type, context, primary, assigned_members);
        }

        static Error verifyMembers(const Members &members, bool *assigned_members, std::vector<std::string> &missing_members, const char *super_name)
        {
            Error memberError = verifyMember(members.template get<0>(), PAGE, assigned_members, missing_members, super_name);
            using Super = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<T, T>::js_static_meta_super_info())>::type;
            Error superError = StartSuperRecursion<T, PAGE + Members::size, Super::size>::verifyMembers(assigned_members, missing_members);
            if (memberError != Error::NoError)
                return memberError;
            return superError;
        }

        static void serializeMembers(const T &from_type, const Members &members, Token &token, Serializer &serializer, const char *super_name)
        {
            serializeMember(from_type, members.template get<Members::size - 1>(), token, serializer, super_name);
            using Super = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<T, T>::js_static_meta_super_info())>::type;
            StartSuperRecursion<T, PAGE + Members::size, Super::size>::serializeMembers(from_type, token, serializer);

        }
    };

    template<typename T, size_t PAGE, size_t INDEX>
    Error SuperClassHandler<T, PAGE, INDEX>::handleSuperClasses(T &to_type, ParseContext &context, bool primary, bool *assigned_members)
    {
        using SuperMeta = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<T,T>::js_static_meta_super_info())>::type;
        using Super = typename JS::TypeAt<INDEX, SuperMeta>::type::type;
        using Members = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<Super,Super>::js_static_meta_data_info())>::type;
        auto &members = Internal::template JsonStructBaseDummy<Super,Super>::js_static_meta_data_info();
        Error error = MemberChecker<Super, Members, PAGE, Members::size - 1>::unpackMembers(static_cast<Super &>(to_type), members, context, primary, assigned_members);
        if (error != Error::MissingPropertyMember)
            return error;
#if JS_HAVE_CONSTEXPR
        return SuperClassHandler<T, PAGE + memberCount<Super, 0>(), INDEX - 1>::handleSuperClasses(to_type, context, primary, assigned_members);
#else
        return SuperClassHandler<T, PAGE, INDEX - 1>::handleSuperClasses(to_type, context, primary, assigned_members);
#endif
    }

    template<typename T, size_t PAGE, size_t INDEX>
    Error SuperClassHandler<T, PAGE, INDEX>::verifyMembers(bool *assigned_members, std::vector<std::string> &missing_members)
    {
        using SuperMeta = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<T,T>::js_static_meta_super_info())>::type;
        using Super = typename TypeAt<INDEX, SuperMeta>::type::type;
        using Members = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<Super,Super>::js_static_meta_data_info())>::type;
        auto &members = Internal::template JsonStructBaseDummy<Super,Super>::js_static_meta_data_info();
        const char *super_name = Internal::template JsonStructBaseDummy<T,T>::js_static_meta_super_info().template get<INDEX>().name.data;
        Error error = MemberChecker<Super, Members, PAGE, Members::size - 1>::verifyMembers(members, assigned_members, missing_members, super_name);
#if JS_HAVE_CONSTEXPR
        Error superError = SuperClassHandler<T, PAGE + memberCount<Super, 0>(), INDEX - 1>::verifyMembers(assigned_members, missing_members);
#else
        Error superError = SuperClassHandler<T, PAGE, INDEX - 1>::verifyMembers(assigned_members, missing_members);
#endif
        if (error != Error::NoError)
            return error;
        return superError;
    }

    template<typename T, size_t PAGE, size_t INDEX>
    size_t JS_CONSTEXPR SuperClassHandler<T, PAGE, INDEX>::membersInSuperClasses()
    {
        using SuperMeta = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<T,T>::js_static_meta_super_info())>::type;
        using Super = typename TypeAt<INDEX, SuperMeta>::type::type;
#if JS_HAVE_CONSTEXPR
        return memberCount<Super, PAGE>() + SuperClassHandler<T, PAGE + memberCount<Super, PAGE>(), INDEX - 1>::membersInSuperClasses();
#else
        return memberCount<Super, PAGE>() + SuperClassHandler<T, PAGE, INDEX - 1>::membersInSuperClasses();
#endif
    }

    template<typename T, size_t PAGE, size_t INDEX>
    void SuperClassHandler<T, PAGE, INDEX>::serializeMembers(const T &from_type, Token &token, Serializer &serializer)
    {
        using SuperMeta = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<T,T>::js_static_meta_super_info())>::type;
        using Super = typename TypeAt<INDEX, SuperMeta>::type::type;
        using Members = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<Super,Super>::js_static_meta_data_info())>::type;
        auto &members = Internal::template JsonStructBaseDummy<Super,Super>::js_static_meta_data_info();
        MemberChecker<Super, Members, PAGE, Members::size - 1>::serializeMembers(from_type, members, token, serializer, "");
#if JS_HAVE_CONSTEXPR
        SuperClassHandler<T, PAGE + memberCount<Super, 0>(), INDEX - 1>::serializeMembers(from_type, token, serializer);
#else
        SuperClassHandler<T, PAGE, INDEX - 1>::serializeMembers(from_type, token, serializer);
#endif
    }

    template<typename T, size_t PAGE>
    struct SuperClassHandler<T, PAGE, 0>
    {
        static Error handleSuperClasses(T &to_type, ParseContext &context, bool primary, bool *assigned_members)
        {
            using Meta = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<T,T>::js_static_meta_super_info())>::type;
            using Super = typename TypeAt<0, Meta>::type::type;
            using Members = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<Super,Super>::js_static_meta_data_info())>::type;
            auto &members = Internal::template JsonStructBaseDummy<Super, Super>::js_static_meta_data_info();
            return MemberChecker<Super, Members, PAGE, Members::size- 1>::unpackMembers(static_cast<Super &>(to_type), members, context, primary, assigned_members);
        }
        static Error verifyMembers(bool *assigned_members, std::vector<std::string> &missing_members)
        {
            using SuperMeta = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<T,T>::js_static_meta_super_info())>::type;
            using Super = typename TypeAt<0, SuperMeta>::type::type;
            using Members = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<Super,Super>::js_static_meta_data_info())>::type;
            auto &members = Internal::template JsonStructBaseDummy<Super, Super>::js_static_meta_data_info();
            const char *super_name = Internal::template JsonStructBaseDummy<T,T>::js_static_meta_super_info().template get<0>().name.data;
            return MemberChecker<Super, Members, PAGE, Members::size - 1>::verifyMembers(members, assigned_members, missing_members, super_name);
        }
        JS_CONSTEXPR static size_t membersInSuperClasses()
        {
            using SuperMeta = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<T,T>::js_static_meta_super_info())>::type;
            using Super = typename TypeAt<0, SuperMeta>::type::type;
            return memberCount<Super, PAGE>();
        }
        static void serializeMembers(const T &from_type, Token &token, Serializer &serializer)
        {
            using SuperMeta = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<T,T>::js_static_meta_super_info())>::type;
            using Super = typename TypeAt<0, SuperMeta>::type::type;
            using Members = typename std::remove_reference<decltype(Internal::template JsonStructBaseDummy<Super,Super>::js_static_meta_data_info())>::type;
			auto &members = Internal::JsonStructBaseDummy<Super, Super>::js_static_meta_data_info();
            MemberChecker<Super, Members, PAGE, Members::size - 1>::serializeMembers(from_type, members, token, serializer, "");
        }
    };

    static bool skipArrayOrObject(ParseContext &context)
    {
        assert(context.error == Error::NoError);
        Type end_type;
        if (context.token.value_type == Type::ObjectStart) {
            end_type = Type::ObjectEnd;
        }
        else if (context.token.value_type == Type::ArrayStart) {
            end_type = Type::ArrayEnd;
        }
        else {
            return false;
        }

        while ((context.error == Error::NoError && context.token.value_type != end_type))
        {
            context.nextToken();
            if (context.error != Error::NoError)
                return false;
            if (context.token.value_type == Type::ObjectStart
                || context.token.value_type == Type::ArrayStart) {
                if (skipArrayOrObject(context))
                    context.nextToken();
                if (context.error != Error::NoError)
                    return false;
            }
        }

        return true;
    }
}

namespace Internal {
    template<typename ...Ts>
    static int js_snprintf(char *dst, size_t max, const char * format, Ts ...args)
    {
#ifdef _MSC_VER
        return _snprintf_s(dst, max, max, format, args...);
#else
        return snprintf(dst, max, format, args...);
#endif
    }
}

template<typename T>
inline Error ParseContext::parseTo(T &to_type)
{
    error = tokenizer.nextToken(token);
    if (error != JS::Error::NoError)
        return error;
    error = TypeHandler<T>::to(to_type, *this);
    return error;
}

struct SerializerContext
{
    SerializerContext(std::string &json_out_p)
        : serializer()
        , cb_ref(serializer.addRequestBufferCallback([this](Serializer &serializer_p)
                                                    {
                size_t end = this->json_out.size();
                this->json_out.resize(end * 2);
                serializer_p.appendBuffer(&(this->json_out[0]) + end, end);
                this->last_pos = end;
                                                    }))
        , json_out(json_out_p)
        , last_pos(0)
    {
        if (json_out.empty())
            json_out.resize(4096);
        serializer.appendBuffer(&json_out[0], json_out.size());
    }

    ~SerializerContext()
    {
        flush();
    }

    template<typename T>
    void serialize(const T &type)
    {
        JS::Token token;
        JS::TypeHandler<T>::from(type, token, serializer);
        flush();
    }

    void flush()
    {
        if (serializer.buffers().empty() || !serializer.buffers().back().used)
            return;
        json_out.resize(last_pos + serializer.buffers().back().used);
        serializer.clearBuffers();
    }

    Serializer serializer;
    BufferRequestCBRef cb_ref;
    std::string &json_out;
    size_t last_pos;
};

template<typename T>
std::string serializeStruct(const T &from_type)
{
    std::string ret_string;
    SerializerContext serializeContext(ret_string);
    Token token;
    TypeHandler<T>::from(from_type, token, serializeContext.serializer);
    serializeContext.flush();
    return ret_string;
}

template<typename T>
std::string serializeStruct(const T &from_type, const SerializerOptions &options)
{
    std::string ret_string;
    SerializerContext serializeContext(ret_string);
       serializeContext.serializer.setOptions(options);
    Token token;
    TypeHandler<T>::from(from_type, token, serializeContext.serializer);
    serializeContext.flush();
    return ret_string;
}

template<>
struct TypeHandler<Error>
{
    static inline Error to(Error &to_type, ParseContext &context)
    {
		(void)to_type;
		(void)context;
//		if (context.token.value_type == JS::Type::Number) {
//			int x;
//			Error error = TypeHandler<int>::to(x, context);
//			for (int i = 0; i < )
//		}

//        size_t level = 1;
//        Error error = Error::NoError;
//        while (error == JS::Error::NoError && level) {
//            error = context.nextToken();
//            if (context.token.value_type == Type::ObjectStart)
//                level++;
//            else if (context.token.value_type == Type::ObjectEnd)
//                level--;
//        }

//        context.tokenizer.copyIncludingValue(context.token, to_type.data);

        return Error::NoError;
    }

    static inline void from(const Error &from_type, Token &token, Serializer &serializer)
    {
        token.value_type = JS::Type::String;
        if (from_type < JS::Error::UserDefinedErrors)
        {
            token.value = DataRef(Internal::error_strings[(int)from_type]);
        }
        else
        {
            token.value = DataRef("UserDefinedError");
        }
        serializer.write(token);
    }
};

struct CallFunctionExecutionState
{
    explicit CallFunctionExecutionState(const std::string &name)
        : name(name)
        , error(Error::NoError)
    {}
    std::string name;
    SilentString context;
    Error error;
    SilentString error_string;
    SilentVector<std::string> missing_members;
    SilentVector<std::string> unassigned_required_members;
    SilentVector<CallFunctionExecutionState> child_states;
    JS_OBJECT(
            JS_MEMBER(name),
            JS_MEMBER(context),
            JS_MEMBER(error),
            JS_MEMBER(error_string),
            JS_MEMBER(missing_members),
            JS_MEMBER(unassigned_required_members),
            JS_MEMBER(child_states)
            );
};

struct CallFunctionContext;

struct CallFunctionErrorContext
{
    CallFunctionErrorContext(CallFunctionContext &context)
        : context(context)
    {}

    Error setError(Error error, const std::string &error_string);
    Error setError(const std::string &error_string) { return setError(Error::UserDefinedErrors, error_string); }
    Error getLatestError() const;

private:
    CallFunctionContext &context;
};

struct CallFunctionContext
{
    CallFunctionContext(ParseContext &parser_context, Serializer &return_serializer)
        : parse_context(parser_context)
        , return_serializer(return_serializer)
        , error_context(*this)
    {}

    virtual ~CallFunctionContext() {}
    template<typename T>
    Error callFunctions(T &container);

    ParseContext &parse_context;
    Serializer &return_serializer;
    CallFunctionErrorContext error_context;
    std::vector<CallFunctionExecutionState> execution_list;
    std::string user_context;
    bool allow_missing = false;
    bool stop_execute_on_fail = false;
    void *user_handle = nullptr;

protected:
    virtual void beforeCallFunctions() {}
    virtual void afterCallFunctions() {}
};

inline Error CallFunctionErrorContext::setError(Error error, const std::string &errorString)
{
    context.parse_context.error = error;
    if (context.execution_list.size()) {
        context.execution_list.back().error = error;
        context.execution_list.back().error_string.data = context.parse_context.tokenizer.makeErrorString();
    }
    context.parse_context.tokenizer.updateErrorContext(error, errorString);
    return error;
}

inline Error CallFunctionErrorContext::getLatestError() const
{
    return context.parse_context.error;
}

template<typename T, typename Ret, typename Arg, size_t NAME_COUNT, size_t TAKES_CONTEXT>
struct FunctionInfo
{
    typedef Ret(T::*Function)(Arg);
    typedef Ret returnType;
    DataRef name[NAME_COUNT];
    Function function;
};

/// \private
template<typename T, typename Ret, typename Arg, size_t NAME_COUNT>
struct FunctionInfo<T, Ret, Arg, NAME_COUNT, 1>
{
    typedef Ret(T::*Function)(Arg, CallFunctionErrorContext &);
    typedef Ret returnType;
    DataRef name[NAME_COUNT];
    Function function;
};

/// \private
template<typename T, typename Ret, typename Arg, size_t NAME_COUNT>
struct FunctionInfo<T, Ret, Arg, NAME_COUNT, 2>
{
    typedef Ret(T::*Function)(Arg, CallFunctionContext &);
    typedef Ret returnType;
    DataRef name[NAME_COUNT];
    Function function;
};

/// \private
template<typename T, typename Ret, size_t NAME_COUNT, size_t TAKES_CONTEXT>
struct FunctionInfo<T, Ret, void, NAME_COUNT, TAKES_CONTEXT>
{
    typedef Ret(T::*Function)(void);
    typedef Ret returnType;
    DataRef name[NAME_COUNT];
    Function function;
};

/// \private
template<typename T, typename Ret, size_t NAME_COUNT>
struct FunctionInfo<T, Ret, void, NAME_COUNT, 1>
{
    typedef Ret(T::*Function)(CallFunctionErrorContext &);
    typedef Ret returnType;
    DataRef name[NAME_COUNT];
    Function function;
};

/// \private
template<typename T, typename Ret, size_t NAME_COUNT>
struct FunctionInfo<T, Ret, void, NAME_COUNT, 2>
{
    typedef Ret(T::*Function)(CallFunctionContext &);
    typedef Ret returnType;
    DataRef name[NAME_COUNT];
    Function function;
};

/// \private
template<typename T, typename Ret, typename Arg, size_t NAME_SIZE, typename ...Aliases>
JS_CONSTEXPR FunctionInfo<T, Ret, Arg, sizeof...(Aliases) + 1, 0> makeFunctionInfo(const char(&name)[NAME_SIZE], Ret(T::*function)(Arg), Aliases ...aliases)
{
    return { { DataRef(name), DataRef(aliases)...} , function };
}

/// \private
template<typename T, typename Ret, typename Arg, size_t NAME_SIZE, typename ...Aliases>
JS_CONSTEXPR FunctionInfo<T, Ret, Arg, sizeof...(Aliases) + 1, 1> makeFunctionInfo(const char(&name)[NAME_SIZE], Ret(T::*function)(Arg, CallFunctionErrorContext &), Aliases ...aliases)
{
    return { { DataRef(name), DataRef(aliases)...} , function };
}

/// \private
template<typename T, typename Ret, typename Arg, size_t NAME_SIZE, typename ...Aliases>
JS_CONSTEXPR FunctionInfo<T, Ret, Arg, sizeof...(Aliases) + 1, 2> makeFunctionInfo(const char(&name)[NAME_SIZE], Ret(T::*function)(Arg, CallFunctionContext &), Aliases ...aliases)
{
    return { {DataRef(name), DataRef(aliases)...},   function };
}

/// \private
template<typename T, typename Ret, size_t NAME_SIZE, typename ...Aliases>
JS_CONSTEXPR FunctionInfo<T, Ret, void, sizeof...(Aliases) + 1, 0> makeFunctionInfo(const char(&name)[NAME_SIZE], Ret(T::*function)(void), Aliases ...aliases)
{
    return { {DataRef(name), DataRef(aliases)...}, function };
}

/// \private
template<typename T, typename Ret, size_t NAME_SIZE, typename ...Aliases>
JS_CONSTEXPR FunctionInfo<T, Ret, void, sizeof...(Aliases) + 1, 1> makeFunctionInfo(const char(&name)[NAME_SIZE], Ret(T::*function)(CallFunctionErrorContext &), Aliases ...aliases)
{
    return { {DataRef(name), DataRef(aliases)...}, function };
}

/// \private
template<typename T, typename Ret, size_t NAME_SIZE, typename ...Aliases>
JS_CONSTEXPR FunctionInfo<T, Ret, void, sizeof...(Aliases) + 1, 2> makeFunctionInfo(const char(&name)[NAME_SIZE], Ret(T::*function)(CallFunctionContext &), Aliases ...aliases)
{
    return{ {DataRef(name), DataRef(aliases)...}, function };
}

namespace Internal {
template <typename T>
struct HasJsonStructFunctionContainer {
    typedef char yes[1];
    typedef char no[2];

    template <typename C>
        static JS_CONSTEXPR yes& test_in_base(typename C::template JsonStructFunctionContainer<C>*);

    template <typename>
        static JS_CONSTEXPR no& test_in_base(...);
};

template<typename JS_BASE_CONTAINER_STRUCT_T, typename JS_CONTAINER_STRUCT_T>
struct JsonStructFunctionContainerDummy
{
    using TT = typename std::remove_reference<decltype(JS_CONTAINER_STRUCT_T::template JsonStructFunctionContainer<JS_CONTAINER_STRUCT_T>::js_static_meta_functions_info())>::type;
    using ST = typename std::remove_reference<decltype(JS_CONTAINER_STRUCT_T::template JsonStructFunctionContainer<JS_CONTAINER_STRUCT_T>::js_static_meta_super_info())>::type;
    static const TT &js_static_meta_functions_info()
    {
        return JS_CONTAINER_STRUCT_T::template JsonStructFunctionContainer<JS_CONTAINER_STRUCT_T>::js_static_meta_functions_info();
    }

    static const ST &js_static_meta_super_info()
    {
        return JS_CONTAINER_STRUCT_T::template JsonStructFunctionContainer<JS_CONTAINER_STRUCT_T>::js_static_meta_super_info();
    }
};

}

#define JS_FUNCTION(name) JS::makeFunctionInfo(#name, &JS_CONTAINER_STRUCT_T::name)
#define JS_FUNCTION_ALIASES(name, ...) JS::makeFunctionInfo(#name, &JS_CONTAINER_STRUCT_T::name, __VA_ARGS__)
#define JS_FUNCTION_WITH_NAME(member, name) JS::makeFunctionInfo(name, &JS_CONTAINER_STRUCT_T::member)
#define JS_FUNCTION_WITH_NAME_ALIASES(member, name, ...) JS::makeFunctionInfo(name, &JS_CONTAINER_STRUCT_T::member, __VA_ARGS__)
#define JS_FUNCTION_CONTAINER(...) \
    template<typename JS_CONTAINER_STRUCT_T> \
    struct JsonStructFunctionContainer \
    { \
        using TT = decltype(JS::makeTuple(__VA_ARGS__)); \
        static const TT &js_static_meta_functions_info() \
        { static auto ret = JS::makeTuple(__VA_ARGS__); return ret; } \
       static const decltype(JS::makeTuple()) &js_static_meta_super_info() \
       { static auto ret = JS::makeTuple(); return ret; } \
    }

#define JS_FUNCTION_CONTAINER_WITH_SUPER(super_list, ...) \
    template<typename JS_CONTAINER_STRUCT_T> \
    struct JsonStructFunctionContainer \
    { \
       using TT = decltype(JS::makeTuple(__VA_ARGS__)); \
       static const TT &js_static_meta_functions_info() \
       { static auto ret = JS::makeTuple(__VA_ARGS__); return ret; } \
       static const decltype(super_list) &js_static_meta_super_info() \
       { static auto ret = super_list; return ret; } \
    }

#define JS_FUNCTION_CONTAINER_EXTERNAL(Type, ...) \
    namespace JS { \
    namespace Internal { \
    template<typename JS_CONTAINER_STRUCT_T> \
    struct JsonStructFunctionContainerDummy<Type, JS_CONTAINER_STRUCT_T> \
    { \
        using TT = decltype(JS::makeTuple(__VA_ARGS__)); \
        static const TT &js_static_meta_functions_info() \
        { static auto ret = JS::makeTuple(__VA_ARGS__); return ret; } \
       static const decltype(JS::makeTuple()) &js_static_meta_super_info() \
       { static auto ret = JS::makeTuple(); return ret; } \
    }; \
    }}

#define JS_FUNCTION_CONTAINER_EXTERNAL_WITH_SUPER(Type, super_list, ...) \
    namespace JS { \
    namespace Internal { \
    template<typename JS_CONTAINER_STRUCT_T> \
    struct JsonStructFunctionContainerDummy<Type, JS_CONTAINER_STRUCT_T> \
    { \
        using TT = decltype(JS::makeTuple(__VA_ARGS__)); \
        static const TT &js_static_meta_functions_info() \
        { static auto ret = JS::makeTuple(__VA_ARGS__); return ret; } \
        static const decltype(super_list) &js_static_meta_super_info() \
        { static auto ret = super_list; return ret; } \
    }; \
    }}

namespace Internal {
    template<typename T, typename U, typename Ret, typename Arg, size_t NAME_COUNT, size_t TAKES_CONTEXT>
    struct FunctionCaller
    {
        static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, Ret, Arg, NAME_COUNT, TAKES_CONTEXT> &functionInfo, CallFunctionContext &context)
        {
            typedef typename std::remove_reference<Arg>::type NonRefArg;
            typedef typename std::remove_cv<NonRefArg>::type PureArg;
            PureArg arg;
            context.parse_context.error = TypeHandler<PureArg>::to(arg, context.parse_context);
            if (context.parse_context.error != Error::NoError)
                return context.parse_context.error;

            Token token;
            TypeHandler<Ret>::from((container.*functionInfo.function)(arg), token, context.return_serializer);
            return Error::NoError;
        }
    };

    template<typename T, typename U, typename Ret, typename Arg, size_t NAME_COUNT>
    struct FunctionCaller<T, U, Ret, Arg, NAME_COUNT, 1>
    {
        static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, Ret, Arg, NAME_COUNT, 1> &functionInfo, CallFunctionContext &context)
        {
            typedef typename std::remove_reference<Arg>::type NonRefArg;
            typedef typename std::remove_cv<NonRefArg>::type PureArg;
            PureArg arg;
            context.parse_context.error = TypeHandler<PureArg>::to(arg, context.parse_context);
            if (context.parse_context.error != Error::NoError)
                return context.parse_context.error;

            Token token;
            Ret ret = (container.*functionInfo.function)(arg, context.error_context);
            if (context.execution_list.back().error == Error::NoError)
                TypeHandler<Ret>::from(ret, token, context.return_serializer);
            return context.execution_list.back().error;
        }
    };

    template<typename T, typename U, typename Ret, typename Arg, size_t NAME_COUNT>
    struct FunctionCaller<T, U, Ret, Arg, NAME_COUNT, 2>
    {
        static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, Ret, Arg, NAME_COUNT, 2> &functionInfo, CallFunctionContext &context)
        {
            typedef typename std::remove_reference<Arg>::type NonRefArg;
            typedef typename std::remove_cv<NonRefArg>::type PureArg;
            PureArg arg;
            context.parse_context.error = TypeHandler<PureArg>::to(arg, context.parse_context);
            if (context.parse_context.error != Error::NoError)
                return context.parse_context.error;

            Token token;
            Ret ret = (container.*functionInfo.function)(arg, context);
            if (context.execution_list.back().error == Error::NoError)
                    TypeHandler<Ret>::from(ret, token, context.return_serializer);
            return context.execution_list.back().error;
        }
    };

    template<typename T, typename U, typename Arg, size_t NAME_COUNT, size_t TAKES_CONTEXT>
    struct FunctionCaller<T, U, void, Arg, NAME_COUNT, TAKES_CONTEXT>
    {
        static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, void, Arg, NAME_COUNT, TAKES_CONTEXT> &functionInfo, CallFunctionContext &context)
        {
            typedef typename std::remove_reference<Arg>::type NonRefArg;
            typedef typename std::remove_cv<NonRefArg>::type PureArg;
            PureArg arg;
            context.parse_context.error = TypeHandler<PureArg>::to(arg, context.parse_context);
            if (context.parse_context.error != Error::NoError)
                return context.parse_context.error;

            (container.*functionInfo.function)(arg);
            return Error::NoError;
        }
    };

    template<typename T, typename U, typename Arg, size_t NAME_COUNT>
    struct FunctionCaller<T, U, void, Arg, NAME_COUNT, 1>
    {
        static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, void, Arg, NAME_COUNT, 1> &functionInfo, CallFunctionContext &context)
        {
            typedef typename std::remove_reference<Arg>::type NonRefArg;
            typedef typename std::remove_cv<NonRefArg>::type PureArg;
            PureArg arg;
            context.parse_context.error = TypeHandler<PureArg>::to(arg, context.parse_context);
            if (context.parse_context.error != Error::NoError)
                return context.parse_context.error;

            (container.*functionInfo.function)(arg, context.error_context);
            return context.execution_list.back().error;
        }
    };

    template<typename T, typename U, typename Arg, size_t NAME_COUNT>
    struct FunctionCaller<T, U, void, Arg, NAME_COUNT, 2>
    {
        static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, void, Arg, NAME_COUNT, 2> &functionInfo, CallFunctionContext &context)
        {
            typedef typename std::remove_reference<Arg>::type NonRefArg;
            typedef typename std::remove_cv<NonRefArg>::type PureArg;
            PureArg arg;
            context.parse_context.error = TypeHandler<PureArg>::to(arg, context.parse_context);
            if (context.parse_context.error != Error::NoError)
                return context.parse_context.error;

            (container.*functionInfo.function)(arg, context);
            return context.execution_list.back().error;
        }
    };

    static inline void checkValidVoidParameter(CallFunctionContext &context)
    {
        if (context.parse_context.token.value_type != Type::Null
            && context.parse_context.token.value_type != Type::ArrayStart
            && context.parse_context.token.value_type != Type::ObjectStart
            && context.parse_context.token.value_type != Type::Bool)
        {
            fprintf(stderr, "Passing data arguments to a void function\n");
        }
        skipArrayOrObject(context.parse_context);
    }

    template<typename T, typename U, typename Ret, size_t NAME_COUNT, size_t TAKES_CONTEXT>
    struct FunctionCaller<T, U, Ret, void, NAME_COUNT, TAKES_CONTEXT>
    {
        static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, Ret, void, NAME_COUNT, TAKES_CONTEXT> &functionInfo, CallFunctionContext &context)
        {
            checkValidVoidParameter(context);
            if (context.parse_context.error != Error::NoError)
                return context.parse_context.error;
            Token token;
            TypeHandler<Ret>::from((container.*functionInfo.function)(), token, context.return_serializer);
            return Error::NoError;
        }
    };

    template<typename T, typename U, typename Ret, size_t NAME_COUNT>
    struct FunctionCaller<T, U, Ret, void, NAME_COUNT, 1>
    {
        static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, Ret, void, NAME_COUNT, 1> &functionInfo, CallFunctionContext &context)
        {
            checkValidVoidParameter(context);
            if (context.parse_context.error != Error::NoError)
                return context.parse_context.error;

            Token token;
            Ret ret = (container.*functionInfo.function)(context.error_context);
            if (context.execution_list.back().error == Error::NoError)
                    TypeHandler<Ret>::from(ret, token, context.return_serializer);
            return context.execution_list.back().error;
        }
    };

    template<typename T, typename U, typename Ret, size_t NAME_COUNT>
    struct FunctionCaller<T, U, Ret, void, NAME_COUNT, 2>
    {
        static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, Ret, void, NAME_COUNT, 2> &functionInfo, CallFunctionContext &context)
        {
            checkValidVoidParameter(context);
            if (context.parse_context.error != Error::NoError)
                return context.parse_context.error;

            Token token;
            Ret ret = (container.*functionInfo.function)(context);
            if (context.execution_list.back().error == Error::NoError)
                    TypeHandler<Ret>::from(ret, token, context.return_serializer);
            return context.execution_list.back().error;
        }
    };

    template<typename T, typename U, size_t NAME_COUNT, size_t TAKES_CONTEXT>
    struct FunctionCaller<T, U, void, void, NAME_COUNT, TAKES_CONTEXT>
    {
        static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, void, void, NAME_COUNT, TAKES_CONTEXT> &functionInfo, CallFunctionContext &context)
       {
            checkValidVoidParameter(context);
            if (context.parse_context.error != Error::NoError)
                return context.parse_context.error;

            (container.*functionInfo.function)();
            return Error::NoError;
        }
    };

    template<typename T, typename U, size_t NAME_COUNT>
    struct FunctionCaller<T, U, void, void, NAME_COUNT, 1>
    {
        static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, void, void, NAME_COUNT, 1> &functionInfo, CallFunctionContext &context)
        {
            checkValidVoidParameter(context);
            if (context.parse_context.error != Error::NoError)
                return context.parse_context.error;

            (container.*functionInfo.function)(context.error_context);
            return context.execution_list.back().error;
        }
    };

    template<typename T, typename U, size_t NAME_COUNT>
    struct FunctionCaller<T, U, void, void, NAME_COUNT, 2>
    {
        static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, void, void, NAME_COUNT, 2> &functionInfo, CallFunctionContext &context)
        {
            checkValidVoidParameter(context);
            if (context.parse_context.error != Error::NoError)
                return context.parse_context.error;

            (container.*functionInfo.function)(context);
            return context.execution_list.back().error;
        }
    };
}
template<typename T, typename U, typename Ret, typename Arg, size_t NAME_COUNT, size_t TAKES_CONTEXT>
Error matchAndCallFunction(T &container, CallFunctionContext &context, FunctionInfo<U,Ret,Arg, NAME_COUNT, TAKES_CONTEXT> &functionInfo, bool primary)
{
    if (primary && context.parse_context.token.name.size == functionInfo.name[0].size && memcmp(functionInfo.name[0].data, context.parse_context.token.name.data, functionInfo.name[0].size) == 0)
    {
        return Internal::FunctionCaller<T, U, Ret, Arg, NAME_COUNT, TAKES_CONTEXT>::callFunctionAndSerializeReturn(container, functionInfo, context);
    } else if (!primary) {
        for (size_t i = 1; i < NAME_COUNT; i++)
        {
            if (context.parse_context.token.name.size == functionInfo.name[i].size && memcmp(functionInfo.name[i].data, context.parse_context.token.name.data, functionInfo.name[i].size) == 0)
            {
                return Internal::FunctionCaller<T, U, Ret, Arg, NAME_COUNT, TAKES_CONTEXT>::callFunctionAndSerializeReturn(container, functionInfo, context);
            }
        }
    }
    return Error::MissingFunction;
}

namespace Internal {
    template<typename T, size_t INDEX>
    struct FunctionalSuperRecursion
    {
        static Error callFunction(T &container, CallFunctionContext &context, bool primary);
    };

    template<typename T, size_t SIZE>
    struct StartFunctionalSuperRecursion
    {
        static Error callFunction(T &container, CallFunctionContext &context, bool primary)
        {
            return FunctionalSuperRecursion<T, SIZE - 1>::callFunction(container, context, primary);
        }
    };
    template<typename T>
    struct StartFunctionalSuperRecursion<T, 0>
    {
        static Error callFunction(T &container, CallFunctionContext &context, bool primary)
        {
            JS_UNUSED(container);
            JS_UNUSED(context);
            JS_UNUSED(primary);
            return Error::MissingFunction;
        }
    };

    template<typename T, typename Functions, size_t INDEX>
    struct FunctionObjectTraverser
    {
        static Error call(T &container, CallFunctionContext &context, Functions &functions, bool primary)
        {
            auto function = functions.template get<INDEX>();
            Error error = matchAndCallFunction(container, context, function, primary);
            if (error == Error::NoError)
                return Error::NoError;
            if (error != Error::MissingFunction)
                return context.parse_context.error;
            return FunctionObjectTraverser<T, Functions, INDEX - 1>::call(container, context, functions, primary);
        }
    };

    template<typename T, typename Functions>
    struct FunctionObjectTraverser<T, Functions, 0>
    {
        static Error call(T &container, CallFunctionContext &context, Functions &functions, bool primary)
        {
            auto function = functions.template get<0>();
            Error error = matchAndCallFunction(container, context, function, primary);
            if (error == Error::NoError)
                return Error::NoError;
            if (error != Error::MissingFunction)
                return error;
            using SuperMeta = typename std::remove_reference<decltype(Internal::template JsonStructFunctionContainerDummy<T,T>::js_static_meta_super_info())>::type;
            return StartFunctionalSuperRecursion<T, SuperMeta::size>::callFunction(container, context, primary);
        }
    };

    template<typename T, typename Functions>
    struct FunctionObjectTraverser<T, Functions, size_t(-1)>
    {
        static Error call(T &container, CallFunctionContext &context, Functions &functions, bool primary)
        {
            using SuperMeta = typename std::remove_reference<decltype(Internal::template JsonStructFunctionContainerDummy<T,T>::js_static_meta_super_info())>::type;
            return StartFunctionalSuperRecursion<T, SuperMeta::size>::callFunction(container, context, primary);
        }
    };

    static inline void add_error(CallFunctionExecutionState &executionState, ParseContext &context)
    {
        executionState.error = context.error;
        if (context.error != Error::NoError) {
            if (context.tokenizer.errorContext().custom_message.empty())
                context.tokenizer.updateErrorContext(context.error);
            executionState.error_string.data = context.tokenizer.makeErrorString();
        }
        if (context.missing_members.size())
            std::swap(executionState.missing_members.data, context.missing_members);
        if (context.unassigned_required_members.size())
            std::swap(executionState.unassigned_required_members.data, context.unassigned_required_members);
    }
}

namespace Internal {
    typedef void (CallFunctionContext::*AfterCallFunction)();

    struct RAICallFunctionOnExit
    {
        RAICallFunctionOnExit(CallFunctionContext &context, AfterCallFunction after)
            : context(context)
              , after(after)
        {}
        ~RAICallFunctionOnExit()
        {
            (context.*after)();
        }
        CallFunctionContext &context;
        AfterCallFunction after;
    };
}

namespace Internal {
    struct ArrayEndWriter
    {
        ArrayEndWriter(Serializer &serializer, Token &token)
            : serializer(serializer)
              , token(token)
        {}

        ~ArrayEndWriter()
        {
            token.value_type = Type::ArrayEnd;
            token.value = DataRef("]");
            serializer.write(token);
        }

        Serializer &serializer;
        Token &token;
    };
}

template<typename T>
inline Error CallFunctionContext::callFunctions(T &container)
{
    beforeCallFunctions();
    Internal::RAICallFunctionOnExit callOnExit(*this, &CallFunctionContext::afterCallFunctions);
    JS::Error error = parse_context.nextToken();
    if (error != JS::Error::NoError)
        return error;
    if (parse_context.token.value_type != JS::Type::ObjectStart) {
        return error_context.setError(Error::ExpectedObjectStart, "Can only call functions on objects with members");
    }
    error = parse_context.nextToken();
    if (error != JS::Error::NoError)
        return error;
    Token token;
    token.value_type = Type::ArrayStart;
    token.value = DataRef("[");
    Internal::ArrayEndWriter endWriter(return_serializer, token);
    return_serializer.write(token);
    auto &functions = Internal::JsonStructFunctionContainerDummy<T,T>::js_static_meta_functions_info();
    using FunctionsType	 = typename std::remove_reference<decltype(functions)>::type;
    while (parse_context.token.value_type != JS::Type::ObjectEnd)
    {
        parse_context.tokenizer.pushScope(parse_context.token.value_type);
        execution_list.push_back(CallFunctionExecutionState(std::string(parse_context.token.name.data, parse_context.token.name.size)));
        execution_list.back().context.data = user_context;
        error = Internal::FunctionObjectTraverser<T, FunctionsType, FunctionsType::size - 1>::call(container, *this,  functions, true);
        if (error == Error::MissingFunction)
            error = Internal::FunctionObjectTraverser<T, FunctionsType, FunctionsType::size - 1>::call(container, *this,  functions, false);
        if (error != Error::NoError) {
            assert(error == parse_context.error || parse_context.error == Error::NoError);
            parse_context.error = error;
        }
        Internal::add_error(execution_list.back(), parse_context);
        parse_context.tokenizer.goToEndOfScope(parse_context.token);
        parse_context.tokenizer.popScope();
        if (error == Error::MissingFunction && allow_missing)
            error = Error::NoError;
        if (stop_execute_on_fail && error != Error::NoError)
            return error;

        error = parse_context.nextToken();
        if (error != JS::Error::NoError)
            return error;
    }
    
    return Error::NoError;
}

struct DefaultCallFunctionContext : public CallFunctionContext
{
    DefaultCallFunctionContext(std::string &json_out)
        : CallFunctionContext(p_context, s_context.serializer)
        , s_context(json_out)
    {}

    DefaultCallFunctionContext(const char *data, size_t size, std::string &json_out)
        : CallFunctionContext(p_context, s_context.serializer)
        , p_context(data, size)
        , s_context(json_out)
    {}

    template<size_t SIZE>
    DefaultCallFunctionContext(const char(&data)[SIZE], std::string &json_out)
        : CallFunctionContext(p_context, s_context.serializer)
        , p_context(data)
        , s_context(json_out)
    {}


    ParseContext p_context;
    SerializerContext s_context;
protected:
    void afterCallFunctions() { s_context.flush(); }
};
namespace Internal {
    template<typename T, size_t INDEX>
    Error FunctionalSuperRecursion<T, INDEX>::callFunction(T &container, CallFunctionContext &context, bool primary)
    {
        using SuperMeta = typename std::remove_reference<decltype(Internal::template JsonStructFunctionContainerDummy<T,T>::js_static_meta_super_info())>::type;
        using Super = typename TypeAt<INDEX, SuperMeta>::type::type;
        auto &functions = Internal::template JsonStructFunctionContainerDummy<Super,Super>::js_static_meta_functions_info();
        using FunctionsType =typename std::remove_reference<decltype(functions)>::type;
        Error error = FunctionObjectTraverser<Super, FunctionsType, FunctionsType::size - 1>::call(container, context, functions, primary);
        if (error != Error::MissingFunction)
            return error;

        return FunctionalSuperRecursion<T, INDEX - 1>::callFunction(container, context, primary);
    }

    template<typename T>
    struct FunctionalSuperRecursion<T, 0>
    {
        static Error callFunction(T &container, CallFunctionContext &context, bool primary)
        {
            using SuperMeta = typename std::remove_reference<decltype(Internal::template JsonStructFunctionContainerDummy<T,T>::js_static_meta_super_info())>::type;
            using Super = typename TypeAt<0, SuperMeta>::type::type;
            auto &functions = Internal::template JsonStructFunctionContainerDummy<Super,Super>::js_static_meta_functions_info();
            using FunctionsType = typename std::remove_reference<decltype(functions)>::type;
            return FunctionObjectTraverser<Super, FunctionsType, FunctionsType::size - 1>::call(container, context, functions, primary);
        }
    };
}
namespace Internal {
    enum class ParseEnumStringState
    {
        FindingNameStart,
        FindingNameEnd,
        FindingSeperator
    };
    template <size_t N>
    void populateEnumNames(std::vector<DataRef> &names, const char(&data)[N])
    {
        size_t name_starts_at = 0;
        ParseEnumStringState state = ParseEnumStringState::FindingNameStart;
        for (size_t i = 0; i < N; i++)
        {
            char c = data[i];
            assert(c != '=');
            switch (state)
            {
                case ParseEnumStringState::FindingNameStart:
                    if ((c >= 'A' && c <= 'Z')
                            || (c >= 'a' && c <= 'z')) {
                        name_starts_at = i;
                        state = ParseEnumStringState::FindingNameEnd;
                    }
                    break;
                case ParseEnumStringState::FindingNameEnd:
                    if (c == '\0' || c == '\t' || c == '\n' || c == '\r' || c == ' ' || c == ',') {
                        names.push_back(DataRef(data + name_starts_at, i - name_starts_at));
                        state = c == ',' ? ParseEnumStringState::FindingNameStart : ParseEnumStringState::FindingSeperator;
                    }
                    break;
                case ParseEnumStringState::FindingSeperator:
                    if (c == ',')
                        state = ParseEnumStringState::FindingNameStart;
                    break;
            }
        }
    }
}
} //JS namespace

#define JS_ENUM(name, ...) \
enum class name \
{ \
    __VA_ARGS__ \
}; \
struct js_##name##_string_struct \
{ \
    template <size_t N> \
    explicit js_##name##_string_struct(const char(&data)[N]) \
    { \
        JS::Internal::populateEnumNames(_strings, data); \
    } \
    std::vector<JS::DataRef> _strings; \
    \
    static const std::vector<JS::DataRef> & strings() \
    { \
        static js_##name##_string_struct ret(#__VA_ARGS__); \
        return ret._strings; \
    } \
}; \

#define JS_ENUM_DECLARE_STRING_PARSER(name) \
namespace JS { \
template<> \
struct TypeHandler<name> \
{ \
    static inline Error to(name&to_type, ParseContext &context) \
    { \
        return Internal::EnumHandler<name, js_##name##_string_struct>::to(to_type, context); \
    } \
    static inline void from(const name &from_type, Token &token, Serializer &serializer) \
    { \
        return Internal::EnumHandler<name , js_##name##_string_struct>::from(from_type, token, serializer); \
    } \
}; \
}

#define JS_ENUM_NAMESPACE_DECLARE_STRING_PARSER(ns, name) \
namespace JS { \
template<> \
struct TypeHandler<ns::name> \
{ \
    static inline Error to(ns::name &to_type, ParseContext &context) \
    { \
        return Internal::EnumHandler<ns::name, ns::js_##name##_string_struct>::to(to_type, context); \
    } \
    static inline void from(const ns::name &from_type, Token &token, Serializer &serializer) \
    { \
        return Internal::EnumHandler<ns::name , ns::js_##name##_string_struct>::from(from_type, token, serializer); \
    } \
}; \
}

namespace JS
{
template <typename T>
inline Error TypeHandler<T>::to(T &to_type, ParseContext &context)
{
    if (context.token.value_type != JS::Type::ObjectStart)
        return Error::ExpectedObjectStart;
    Error error = context.tokenizer.nextToken(context.token);
    if (error != JS::Error::NoError)
        return error;
    auto &members = Internal::JsonStructBaseDummy<T,T>::js_static_meta_data_info();
    using MembersType = typename std::remove_reference<decltype(members)>::type;
#if JS_HAVE_CONSTEXPR
    bool assigned_members[Internal::memberCount<T, 0>()];
    memset(assigned_members, 0, sizeof(assigned_members));
#else
    bool *assigned_members = nullptr;
#endif
    while(context.token.value_type != JS::Type::ObjectEnd)
    {
        std::string token_name(context.token.name.data, context.token.name.size);
        error = Internal::MemberChecker<T, MembersType, 0 ,MembersType::size - 1>::unpackMembers(to_type, members, context, true, assigned_members);
        if (error == Error::MissingPropertyMember)
            error = Internal::MemberChecker<T, MembersType, 0 ,MembersType::size - 1>::unpackMembers(to_type, members, context, false, assigned_members);
        if (error == Error::MissingPropertyMember) {

            context.missing_members.push_back(token_name);
            if (context.allow_missing_members) {
                Internal::skipArrayOrObject(context);
                if (context.error != Error::NoError)
                    return context.error;
            }
            else {
                return error;
            }
        } else if (error != Error::NoError) {
            return error;
        }
        context.nextToken();
        if (context.error != Error::NoError)
            return context.error;

    }
    std::vector<std::string> unassigned_required_members;
    error = Internal::MemberChecker<T, MembersType, 0, MembersType::size - 1>::verifyMembers(members, assigned_members, unassigned_required_members, "");
    if (error == Error::UnassignedRequiredMember) {
        context.unassigned_required_members.insert(context.unassigned_required_members.end(),unassigned_required_members.begin(), unassigned_required_members.end());
        if (context.allow_unnasigned_required_members)
            error = Error::NoError;
    }
    return error;
}

template<typename T>
void TypeHandler<T>::from(const T &from_type, Token &token, Serializer &serializer)
{
    static const char objectStart[] = "{";
    static const char objectEnd[] = "}";
    token.value_type = Type::ObjectStart;
    token.value = DataRef(objectStart);
    serializer.write(token);
    auto &members = Internal::JsonStructBaseDummy<T,T>::js_static_meta_data_info();
    using MembersType = typename std::remove_reference<decltype(members)>::type;
    Internal::MemberChecker<T, MembersType, 0, MembersType::size - 1>::serializeMembers(from_type, members, token, serializer, "");
    token.name.size = 0;
    token.name.data = "";
    token.name_type = Type::String;
    token.value_type = Type::ObjectEnd;
    token.value = DataRef(objectEnd);
    serializer.write(token);
}

namespace Internal {
    template<typename T, typename F>
    struct EnumHandler
    {
        static inline Error to(T &to_type, ParseContext &context)
        {
            if (context.token.value_type == Type::String)
            {
                auto &strings = F::strings();
                for (size_t i = 0; i < strings.size(); i++)
                {
                    const DataRef &ref = strings[i];
                    if (ref.size == context.token.value.size) {
                        if (memcmp(ref.data, context.token.value.data, ref.size) == 0) {
                            to_type = static_cast<T>(i);
                            return Error::NoError;
                        }
                    }
                }
            }
            return Error::IllegalDataValue;
        }

        static inline void from(const T &from_type, Token &token, Serializer &serializer)
        {
            size_t i = static_cast<size_t>(from_type);
            token.value = F::strings()[i];
            token.value_type = Type::String;
            serializer.write(token);
        }
    };
}

namespace Internal
{
	static void handle_json_escapes_in(const DataRef &ref, std::string &to_type)
	{
            static const char escaped_table[7][2] =
            {
                { 'b', '\b' },
                { 'f', '\f' },
                { 'n', '\n' },
                { 'r', '\r' },
                { 't', '\t' },
                { '\"', '\"' },
                { '\\', '\\'}
            };
		to_type.reserve(ref.size);
		const char *start = ref.data;
		bool escaped = false;
		for (size_t i = 0; i < ref.size; i++)
    {
      if (escaped)
      {
        escaped = false;
        bool found = false;
        const char current_char = ref.data[i];
        for (int n = 0; n < sizeof(escaped_table) / sizeof(*escaped_table); n++)
        {
          if (current_char == escaped_table[n][0])
          {
            to_type.push_back(escaped_table[n][1]);
            found = true;
            break;
          }
        }
        if (!found)
        {
          to_type.push_back('\\');
          to_type.push_back(current_char);
        }
      }
      else if (ref.data[i] == '\\')
      {
        auto diff = &ref.data[i] - start;
        to_type.insert(to_type.end(), start, start + diff);
        start = &ref.data[i + 2];
        escaped = true;
      }
    }
    if (start < ref.data + ref.size) //This isn't strictly needed since the tokenizer will enforce a char after '\' 
    {
      auto diff = ref.data + ref.size - start;
      to_type.insert(to_type.end(), start, start + diff);
    }
	}

        static DataRef handle_json_escapes_out(const std::string &data, std::string &buffer)
        {
            int start_index = 0;
            for (size_t i = 0; i < data.size(); i++)
            {
                const char cur = data[i];
                if (static_cast<uint8_t>(cur) <= uint8_t('\r') || cur == '\"' || cur == '\\')
                {
                    if (buffer.empty())
                    {
                        buffer.reserve(data.size() + 10);
                    }
                    size_t diff = i - start_index;
                    if (diff > 0)
                    {
                        buffer.insert(buffer.end(), data.data() + start_index, data.data() + start_index + diff);
                    }
                    start_index = i + 1;

                    switch (cur)
                    {
                    case '\b':
                        buffer += std::string("\\b");
                        break;
                    case '\t':
                        buffer += std::string("\\t");
                        break;
                    case '\n':
                        buffer += std::string("\\n");
                        break;
                    case '\f':
                        buffer += std::string("\\f");
                        break;
                    case '\r':
                        buffer += std::string("\\r");
                        break;
                    case '\"':
                        buffer += std::string("\\\"");
                        break;
                    case '\\':
                        buffer += std::string("\\\\");
                        break;
                    default:
                        buffer.push_back(cur);
                        break;
                    }
                }
            }
            if (buffer.size())
            {
                size_t diff = data.size() - start_index;
                if (diff > 0)
                {
                    buffer.insert(buffer.end(), data.data() + start_index, data.data() + start_index + diff);
                }
                return DataRef(buffer.data(), buffer.size());
            }
            return DataRef(data.data(), data.size());
        }
}
/// \private
template<>
struct TypeHandler<std::string>
{
    static inline Error to(std::string &to_type, ParseContext &context)
    {
        to_type.clear();
        Internal::handle_json_escapes_in(context.token.value, to_type);
        return Error::NoError;
    }

    static inline void from(const std::string &str, Token &token, Serializer &serializer)
    {
        std::string buffer;
        DataRef ref = Internal::handle_json_escapes_out(str, buffer);
        token.value_type = Type::String;
        token.value.data = ref.data;
        token.value.size = ref.size;
        serializer.write(token);
    }
};

/// \private
template<>
struct TypeHandler<double>
{
    static inline Error to(double &to_type, ParseContext &context)
    {
        char *pointer;
        to_type = strtod(context.token.value.data, &pointer);
        if (context.token.value.data + context.token.value.size != pointer)
            return Error::FailedToParseDouble;
        return Error::NoError;
    }

    static inline void from(const double &d, Token &token, Serializer &serializer)
    {
        //char buf[1/*'-'*/ + (DBL_MAX_10_EXP+1)/*308+1 digits*/ + 1/*'.'*/ + 6/*Default? precision*/ + 1/*\0*/];
        char buf[32];
        int size;
        size = Internal::js_snprintf(buf, sizeof buf / sizeof *buf, "%1.16e", d);

        if (size < 0) {
            fprintf(stderr, "error serializing float token\n");
            return;
        }

        token.value_type = Type::Number;
        token.value.data = buf;
        token.value.size = size_t(size);
        serializer.write(token);
    }
};


/// \private
template<>
struct TypeHandler<float>
{
    static inline Error to(float &to_type, ParseContext &context)
    {
        char *pointer;
        to_type = strtof(context.token.value.data, &pointer);
        if (context.token.value.data + context.token.value.size != pointer)
            return Error::FailedToParseFloat;
        return Error::NoError;
    }

    static inline void from(const float &f, Token &token, Serializer &serializer)
    {
        char buf[16];
        int size;
        size = Internal::js_snprintf(buf, sizeof buf / sizeof *buf, "%1.8e", f);
        if (size < 0) {
            fprintf(stderr, "error serializing float token\n");
            return;
        }

        token.value_type = Type::Number;
        token.value.data = buf;
        token.value.size = size_t(size);
        serializer.write(token);
    }
};

/// \private
template<>
struct TypeHandler<int32_t>
{
    static inline Error to(int32_t &to_type, ParseContext &context)
    {
        char *pointer;
        long value = strtol(context.token.value.data, &pointer, 10);
        to_type = int32_t(value);
        if (context.token.value.data == pointer)
            return Error::FailedToParseInt;
        return Error::NoError;
    }

    static inline void from(const int32_t &d, Token &token, Serializer &serializer)
    {
        char buf[11];
        int size = Internal::js_snprintf(buf, sizeof buf / sizeof *buf, "%d", d);
        if (size < 0) {
            fprintf(stderr, "error serializing int token\n");
            return;
        }

        token.value_type = Type::Number;
        token.value.data = buf;
        token.value.size = size_t(size);
        serializer.write(token);
    }
};

/// \private
template<>
struct TypeHandler<uint32_t>
{
public:
    static inline Error to(uint32_t &to_type, ParseContext &context)
    {
        char *pointer;
        unsigned long value = strtoul(context.token.value.data, &pointer, 10);
        to_type = static_cast<uint32_t>(value);
        if (context.token.value.data == pointer)
            return Error::FailedToParseInt;
        return Error::NoError;
    }

    static void from(const uint32_t &from_type, Token &token, Serializer &serializer)
    {
        char buf[12];
        int size = Internal::js_snprintf(buf, sizeof buf / sizeof *buf, "%u", from_type);
        if (size < 0) {
            fprintf(stderr, "error serializing int token\n");
            return;
        }

        token.value_type = Type::Number;
        token.value.data = buf;
        token.value.size = size_t(size);
        serializer.write(token);
    }

};

/// \private
template<>
struct TypeHandler<int64_t>
{
public:
    static inline Error to(int64_t &to_type, ParseContext &context)
    {
        static_assert(sizeof(to_type) == sizeof(long long int), "sizeof int64_t != sizeof long long int");
        char *pointer;
        to_type = strtoll(context.token.value.data, &pointer, 10);
        if (context.token.value.data == pointer)
            return Error::FailedToParseInt;
        return Error::NoError;
    }

    static void from(const int64_t &from_type, Token &token, Serializer &serializer)
    {
        static_assert(sizeof(from_type) == sizeof(long long int), "sizeof int64_t != sizeof long long int");
        char buf[24];
        int size = Internal::js_snprintf(buf, sizeof buf / sizeof *buf, "%lld", from_type);
        if (size < 0) {
            fprintf(stderr, "error serializing int token\n");
            return;
        }

        token.value_type = Type::Number;
        token.value.data = buf;
        token.value.size = size_t(size);
        serializer.write(token);
    }

};

/// \private
template<>
struct TypeHandler<uint64_t>
{
public:
    static inline Error to(uint64_t &to_type, ParseContext &context)
    {
        static_assert(sizeof(to_type) == sizeof(long long unsigned int), "sizeof uint64_t != sizeof long long unsinged int");
        char *pointer;
        to_type = strtoull(context.token.value.data, &pointer, 10);
        if (context.token.value.data == pointer)
            return Error::FailedToParseInt;
        return Error::NoError;
    }

    static inline void from(const uint64_t &from_type, Token &token, Serializer &serializer)
    {
        static_assert(sizeof(from_type) == sizeof(long long unsigned int), "sizeof uint64_t != sizeof long long int");
        char buf[24];
        int size = Internal::js_snprintf(buf, sizeof buf / sizeof *buf, "%llu", from_type);
        if (size < 0) {
            fprintf(stderr, "error serializing int token\n");
            return;
        }

        token.value_type = Type::Number;
        token.value.data = buf;
        token.value.size = size_t(size);
        serializer.write(token);
    }

};

template<typename FromT, typename ToT>
Error boundsAssigner(FromT value, ToT &to_type)
{
    static_assert(sizeof(FromT) >= sizeof(ToT), "boundsAssigner with type missmatch");
    if (value < std::numeric_limits<ToT>::lowest())
    {
        fprintf(stderr, "input is lower than types range: %ld : %d\n",
                value,
                std::numeric_limits<ToT>::lowest());
    return Error::FailedToParseInt;
    }
    if (value > std::numeric_limits<ToT>::max())
    {
        fprintf(stderr, "input is higher than types range: %ld : %d\n",
                value,
                std::numeric_limits<ToT>::max());
        return Error::FailedToParseInt;
    }

    to_type = ToT(value);
    return Error::NoError;
}

/// \private
template<>
struct TypeHandler<int16_t>
{
public:
    static inline Error to(int16_t &to_type, ParseContext &context)
    {
        static_assert(sizeof(to_type) == sizeof(short int), "sizeof int16_t != sizeof long long int");
        char *pointer;
        long value = strtol(context.token.value.data, &pointer, 10);
        if (context.token.value.data == pointer)
            return Error::FailedToParseInt;
        return boundsAssigner(value, to_type);
    }

    static inline void from(const int16_t &from_type, Token &token, Serializer &serializer)
    {
        static_assert(sizeof(from_type) == sizeof(short int), "sizeof int16_t != sizeof long long int");
        char buf[24];
        int size = Internal::js_snprintf(buf, sizeof buf / sizeof *buf, "%hd", from_type);
        if (size < 0) {
            fprintf(stderr, "error serializing int token\n");
            return;
        }

        token.value_type = Type::Number;
        token.value.data = buf;
        token.value.size = size_t(size);
        serializer.write(token);
    }

};

/// \private
template<>
struct TypeHandler<uint16_t>
{
public:
    static inline Error to(uint16_t &to_type, ParseContext &context)
    {
        static_assert(sizeof(to_type) == sizeof(unsigned short int), "sizeof uint16_t != sizeof long long unsinged int");
        char *pointer;
        unsigned long value = strtoul(context.token.value.data, &pointer, 10);
        if (context.token.value.data == pointer)
            return Error::FailedToParseInt;
        return boundsAssigner(value, to_type);
    }

    static inline void from(const uint16_t &from_type, Token &token, Serializer &serializer)
    {
        static_assert(sizeof(from_type) == sizeof(unsigned short int), "sizeof uint16_t != sizeof long long int");
        char buf[24];
        int size = Internal::js_snprintf(buf, sizeof buf / sizeof *buf, "%hu", from_type);
        if (size < 0) {
            fprintf(stderr, "error serializing int token\n");
            return;
        }

        token.value_type = Type::Number;
        token.value.data = buf;
        token.value.size = size_t(size);
        serializer.write(token);
    }

};

template<>
struct TypeHandler<uint8_t>
{
public:
    static inline Error to(uint8_t &to_type, ParseContext &context)
    {
        char *pointer;
        unsigned long value = strtoul(context.token.value.data, &pointer, 10);
        if (context.token.value.data == pointer)
            return Error::FailedToParseInt;
        return boundsAssigner(value, to_type);
    }

    static inline void from(const uint8_t &from_type, Token &token, Serializer &serializer)
    {
        char buf[24];
        int size = Internal::js_snprintf(buf, sizeof buf / sizeof *buf, "%hu", from_type);
        if (size < 0) {
            fprintf(stderr, "error serializing int token\n");
            return;
        }

        token.value_type = Type::Number;
        token.value.data = buf;
        token.value.size = size_t(size);
        serializer.write(token);
    }

};

/// \private
template<typename T>
struct TypeHandler<Nullable<T>>
{
public:
    static inline Error to(Nullable<T> &to_type, ParseContext &context)
    {
        if (context.token.value_type == Type::Null)
            return Error::NoError;
        return TypeHandler<T>::to(to_type.data, context);
    }

    static inline void from(const Nullable<T> &opt, Token &token, Serializer &serializer)
    {
        TypeHandler<T>::from(opt(), token, serializer);
    }
};

/// \private
template<typename T>
struct TypeHandler<NullableChecked<T>>
{
public:
    static inline Error to(NullableChecked<T> &to_type, ParseContext &context)
    {
        if (context.token.value_type == Type::Null)
        {
            to_type.null = true;
            return Error::NoError;
        }
        to_type.null = false;
        return TypeHandler<T>::to(to_type.data, context);
    }

    static inline void from(const NullableChecked<T> &opt, Token &token, Serializer &serializer)
    {
        if (opt.null) {
            const char nullChar[] = "null";
            token.value_type = Type::Null;
            token.value = DataRef(nullChar);
            serializer.write(token);
        } else {
            TypeHandler<T>::from(opt(), token, serializer);
        }
    }
};

/// \private
template<typename T>
struct TypeHandler<Optional<T>>
{
public:
    static inline Error to(Optional<T> &to_type, ParseContext &context)
    {
        return TypeHandler<T>::to(to_type.data, context);
    }

    static inline void from(const Optional<T> &opt, Token &token, Serializer &serializer)
    {
        TypeHandler<T>::from(opt(), token, serializer);
    }
};

/// \private
template<typename T>
struct TypeHandler<OptionalChecked<T>>
{
public:
    static inline Error to(OptionalChecked<T> &to_type, ParseContext &context)
    {
        to_type.assigned = true;
        return TypeHandler<T>::to(to_type.data, context);
    }

    static inline void from(const OptionalChecked<T> &opt, Token &token, Serializer &serializer)
    {
        if (opt.assigned)
            TypeHandler<T>::from(opt(), token, serializer);
    }
};

#ifdef JS_STD_OPTIONAL
/// \private
template<typename T>
struct TypeHandler<std::optional<T>>
{
public:
    static inline Error to(std::optional<T> &to_type, ParseContext &context)
    {
        to_type = {};
        return TypeHandler<T>::to(to_type.value(), context);
    }

    static inline void from(const std::optional<T> &opt, Token &token, Serializer &serializer)
    {
        if (opt.has_value())
            TypeHandler<T>::from(opt.value(), token, serializer);
    }
};
#endif

/// \private
template<typename T>
struct TypeHandler<std::unique_ptr<T>>
{
public:
    static inline Error to(std::unique_ptr<T> &to_type, ParseContext &context)
    {
        if (context.token.value_type != Type::Null) {
            to_type.reset(new T());
            return TypeHandler<T>::to(*to_type.get(), context);
        }
        to_type.reset(nullptr);
        return Error::NoError;
    }

    static inline void from(const std::unique_ptr<T> &unique, Token &token, Serializer &serializer)
    {
        if (unique) {
            TypeHandler<T>::from(*unique.get(), token, serializer);
        } else {
            const char nullChar[] = "null";
            token.value_type = Type::Null;
            token.value = DataRef(nullChar);
            serializer.write(token);
        }
    }

};

/// \private
template<>
struct TypeHandler<bool>
{
    static inline Error to(bool &to_type, ParseContext &context)
    {
        if (context.token.value.size == sizeof("true") - 1 && memcmp("true", context.token.value.data, sizeof("true") - 1) == 0)
            to_type = true;
        else if (context.token.value.size == sizeof("false") - 1 && memcmp("false", context.token.value.data, sizeof("false") - 1) == 0)
            to_type = false;
        else
            return Error::FailedToParseBoolean;

        return Error::NoError;
    }

    static inline void from(const bool &b, Token &token, Serializer &serializer)
    {
        const char trueChar[] = "true";
        const char falseChar[] = "false";
        token.value_type = Type::Bool;
        if (b) {
            token.value = DataRef(trueChar);
        }
        else {
            token.value = DataRef(falseChar);
        }
        serializer.write(token);
    }
};

/// \private
template<typename T>
struct TypeHandler<std::vector<T>>
{
public:
    static inline Error to(std::vector<T> &to_type, ParseContext &context)
    {
        if (context.token.value_type != JS::Type::ArrayStart)
            return Error::ExpectedArrayStart;
        Error error = context.nextToken();
        if (error != JS::Error::NoError)
            return error;
        to_type.clear();
        to_type.reserve(10);
        while(context.token.value_type != JS::Type::ArrayEnd)
        {
            to_type.push_back(T());
            error = TypeHandler<T>::to(to_type.back(), context);
            if (error != JS::Error::NoError)
                break;
            error = context.nextToken();
            if (error != JS::Error::NoError)
                break;
        }

        return error;
    }

    static inline void from(const std::vector<T> &vec, Token &token, Serializer &serializer)
    {
        token.value_type = Type::ArrayStart;
        token.value = DataRef("[");
        serializer.write(token);

        token.name = DataRef("");

        for (auto &index : vec)
        {
            TypeHandler<T>::from(index, token, serializer);
        }

        token.name = DataRef("");

        token.value_type = Type::ArrayEnd;
        token.value = DataRef("]");
        serializer.write(token);
    }
};

    /// \private
    template<>
    struct TypeHandler<std::vector<bool>>
    {
    public:
        static inline Error to(std::vector<bool> &to_type, ParseContext &context)
        {
            if (context.token.value_type != JS::Type::ArrayStart)
                return Error::ExpectedArrayStart;
            Error error = context.nextToken();
            if (error != JS::Error::NoError)
                return error;
            to_type.clear();
            to_type.reserve(10);
            while(context.token.value_type != JS::Type::ArrayEnd)
            {
                
                bool toBool;
                error = TypeHandler<bool>::to(toBool, context);
                to_type.push_back(toBool);
                if (error != JS::Error::NoError)
                    break;
                error = context.nextToken();
                if (error != JS::Error::NoError)
                    break;
            }
            
            return error;
        }
        
        static inline void from(const std::vector<bool> &vec, Token &token, Serializer &serializer)
        {
            token.value_type = Type::ArrayStart;
            token.value = DataRef("[");
            serializer.write(token);
            
            token.name = DataRef("");
            
            for (bool index : vec)
            {
                TypeHandler<bool>::from(index, token, serializer);
            }
            
            token.name = DataRef("");
            
            token.value_type = Type::ArrayEnd;
            token.value = DataRef("]");
            serializer.write(token);
        }
    };

/// \private
template<>
struct TypeHandler<SilentString>
{
    static inline Error to(SilentString &to_type, ParseContext &context)
    {
        return TypeHandler<std::string>::to(to_type.data, context);
    }
    static inline void from(const SilentString &str, Token &token, Serializer &serializer)
    {
        if (str.data.size()) {
            TypeHandler<std::string>::from(str.data, token, serializer);
        }
    }
};

/// \private
template<typename T>
struct TypeHandler<SilentVector<T>>
{
public:
    static inline Error to(SilentVector<T> &to_type, ParseContext &context)
    {
        return TypeHandler<std::vector<T>>::to(to_type, context);
    }

    static inline void from(const SilentVector<T> &vec, Token &token, Serializer &serializer)
    {
        if (vec.data.size()) {
            TypeHandler<std::vector<T>>::from(vec.data, token, serializer);
        }
    }
};

/// \private
template<typename T>
struct TypeHandler<SilentUniquePtr<T>>
{
public:
    static inline Error to(SilentUniquePtr<T> &to_type, ParseContext &context)
    {
        return TypeHandler<std::unique_ptr<T>>::to(to_type.data, context);
    }

    static inline void from(const SilentUniquePtr<T> &ptr, Token &token, Serializer &serializer)
    {
        if (ptr.data) {
            TypeHandler<std::unique_ptr<T>>::from(ptr.data, token, serializer);
        }
    }
};

/// \private
template<>
struct TypeHandler<std::vector<Token>>
{
public:
    static inline Error to(std::vector<Token> &to_type, ParseContext &context)
    {
        if (context.token.value_type != JS::Type::ArrayStart &&
                context.token.value_type != JS::Type::ObjectStart)
        {
            to_type.push_back(context.token);
            return context.error;
        }
        to_type.clear();
        to_type.push_back(context.token);
        bool buffer_change = false;
        auto ref = context.tokenizer.registerNeedMoreDataCallback([&buffer_change](JS::Tokenizer &tokenizer)
        {
            JS_UNUSED(tokenizer);
            buffer_change = true;
        });

        size_t level = 1;
        Error error = Error::NoError;
        while (error == JS::Error::NoError && level && buffer_change == false) {
            error = context.nextToken();
            to_type.push_back(context.token);
            if (context.token.value_type == Type::ArrayStart || context.token.value_type == Type::ObjectStart)
                level++;
            else if (context.token.value_type == Type::ArrayEnd || context.token.value_type == Type::ObjectEnd)
                level--;
        }
        if (buffer_change)
            return Error::NonContigiousMemory;

        return error;
    }

    static inline void from(const std::vector<Token> &from_type, Token &token, Serializer &serializer)
    {
        for (auto &t : from_type) {
            token = t;
            serializer.write(token);
        }
    }
};

/// \private
template<>
struct TypeHandler<JsonTokens>
{
public:
    static inline Error to(JsonTokens &to_type, ParseContext &context)
    {
        return TypeHandler<std::vector<Token>>::to(to_type.data, context);
    }
    static inline void from(const JsonTokens &from, Token &token, Serializer &serializer)
    {
        return TypeHandler<std::vector<Token>>::from(from.data, token, serializer);
    }
};

/// \private
template<>
struct TypeHandler<JsonArrayRef>
{
    static inline Error to(JsonArrayRef &to_type, ParseContext &context)
    {
        if (context.token.value_type != JS::Type::ArrayStart)
            return Error::ExpectedArrayStart;

        bool buffer_change = false;
        auto ref = context.tokenizer.registerNeedMoreDataCallback([&buffer_change](JS::Tokenizer &tokenizer)
                                                                  {
                                                                  JS_UNUSED(tokenizer);
                                                                  buffer_change = true;
                                                                  });

        to_type.ref.data = context.token.value.data;

        size_t level = 1;
        Error error = Error::NoError;
        while (error == JS::Error::NoError && level && buffer_change == false) {
            error = context.nextToken();
            if (context.token.value_type == Type::ArrayStart)
                level++;
            else if (context.token.value_type == Type::ArrayEnd)
                level--;
        }
        if (buffer_change)
            return Error::NonContigiousMemory;

        to_type.ref.size = size_t(context.token.value.data + context.token.value.size - to_type.ref.data);

        return error;
    }

    static inline void from(const JsonArrayRef &from_type, Token &token, Serializer &serializer)
    {
        token.value = from_type.ref;
        token.value_type = Type::Verbatim;
        serializer.write(token);
    }
};

/// \private
template<>
struct TypeHandler<JsonArray>
{
    static inline Error to(JsonArray &to_type, ParseContext &context)
    {
        if (context.token.value_type != JS::Type::ArrayStart)
            return Error::ExpectedArrayStart;

        context.tokenizer.copyFromValue(context.token, to_type.data);

        size_t level = 1;
        Error error = Error::NoError;
        while (error == JS::Error::NoError && level) {
            error = context.nextToken();
            if (context.token.value_type == Type::ArrayStart)
                level++;
            else if (context.token.value_type == Type::ArrayEnd)
                level--;
        }

        if (error == JS::Error::NoError)
            context.tokenizer.copyIncludingValue(context.token, to_type.data);

        return error;
    }

    static inline void from(const JsonArray &from_type, Token &token, Serializer &serializer)
    {
        token.value_type = JS::Type::Verbatim; //Need to fool the serializer to just write value as verbatim

        if (from_type.data.empty())
        {
            std::string emptyArray("[]");
            token.value = DataRef(emptyArray);
            serializer.write(token);
        }
        else
        {
            token.value = DataRef(from_type.data);
            serializer.write(token);
        }
    }
};

/// \private
template<>
struct TypeHandler<JsonObjectRef> {
    static inline Error to(JsonObjectRef &to_type, ParseContext &context)
    {
        if (context.token.value_type != JS::Type::ObjectStart)
            return Error::ExpectedObjectStart;

        bool buffer_change = false;
        auto ref = context.tokenizer.registerNeedMoreDataCallback([&buffer_change](JS::Tokenizer &tokenizer)
                                                                  {
                                                                  JS_UNUSED(tokenizer);
                                                                  buffer_change = true;
                                                                  });

        to_type.ref.data = context.token.value.data;
        size_t level = 1;
        Error error = Error::NoError;
        while (error == JS::Error::NoError && level && buffer_change == false) {
            error = context.nextToken();
            if (context.token.value_type == Type::ObjectStart)
                level++;
            else if (context.token.value_type == Type::ObjectEnd)
                level--;
        }
        if (buffer_change)
            return Error::NonContigiousMemory;

        to_type.ref.size = size_t(context.token.value.data + context.token.value.size - to_type.ref.data);
        return error;
    }

    static inline void from(const JsonObjectRef &from_type, Token &token, Serializer &serializer)
    {
        token.value = from_type.ref;
        token.value_type = Type::Verbatim;
        serializer.write(token);
    }
};

/// \private
template<>
struct TypeHandler<JsonObject>
{
    static inline Error to(JsonObject &to_type, ParseContext &context)
    {
        if (context.token.value_type != JS::Type::ObjectStart)
            return Error::ExpectedObjectStart;

        context.tokenizer.copyFromValue(context.token, to_type.data);

        size_t level = 1;
        Error error = Error::NoError;
        while (error == JS::Error::NoError && level) {
            error = context.nextToken();
            if (context.token.value_type == Type::ObjectStart)
                level++;
            else if (context.token.value_type == Type::ObjectEnd)
                level--;
        }

        context.tokenizer.copyIncludingValue(context.token, to_type.data);

        return error;
    }

    static inline void from(const JsonObject &from_type, Token &token, Serializer &serializer)
    {
        token.value_type = JS::Type::Verbatim; //Need to fool the serializer to just write value as verbatim

        if (from_type.data.empty())
        {
            std::string emptyObject("{}");
            token.value = DataRef(emptyObject);
            serializer.write(token);
        }
        else
        {
            token.value = DataRef(from_type.data);
			serializer.write(token);
        }
    }
};

/// \private
template<>
struct TypeHandler<JsonObjectOrArrayRef> {
    static inline Error to(JsonObjectOrArrayRef &to_type, ParseContext &context)
    {
        JS::Type openType;
        JS::Type closeType;
        if (context.token.value_type == JS::Type::ObjectStart)
        {
            openType = JS::Type::ObjectStart;
            closeType = JS::Type::ObjectEnd;
        }
        else if (context.token.value_type == JS::Type::ArrayStart)
        {
            openType = JS::Type::ArrayStart;
            closeType = JS::Type::ArrayEnd;
        }
        else {
            return Error::ExpectedObjectStart;
        }

        bool buffer_change = false;
        auto ref = context.tokenizer.registerNeedMoreDataCallback([&buffer_change](JS::Tokenizer &tokenizer)
                                                                  {
                                                                  JS_UNUSED(tokenizer);
                                                                  buffer_change = true;
                                                                  });

        to_type.ref.data = context.token.value.data;
        size_t level = 1;
        Error error = Error::NoError;
        while (error == JS::Error::NoError && level && buffer_change == false) {
            error = context.nextToken();
            if (context.token.value_type == openType)
                level++;
            else if (context.token.value_type == closeType)
                level--;
        }
        if (buffer_change)
            return Error::NonContigiousMemory;

        to_type.ref.size = size_t(context.token.value.data + context.token.value.size - to_type.ref.data);
        return error;
    }

    static inline void from(const JsonObjectOrArrayRef &from_type, Token &token, Serializer &serializer)
    {
        token.value = from_type.ref;
        token.value_type = Type::Verbatim;
        serializer.write(token);
    }
};

/// \private
template<>
struct TypeHandler<JsonObjectOrArray>
{
    static inline Error to(JsonObjectOrArray &to_type, ParseContext &context)
    {
        JS::Type openType;
        JS::Type closeType;
        if (context.token.value_type == JS::Type::ObjectStart)
        {
            openType = JS::Type::ObjectStart;
            closeType = JS::Type::ObjectEnd;
        }
        else if (context.token.value_type == JS::Type::ArrayStart)
        {
            openType = JS::Type::ArrayStart;
            closeType = JS::Type::ArrayEnd;
        }
        else {
            return Error::ExpectedObjectStart;
        }


        context.tokenizer.copyFromValue(context.token, to_type.data);

        size_t level = 1;
        Error error = Error::NoError;
        while (error == JS::Error::NoError && level) {
            error = context.nextToken();
            if (context.token.value_type == openType)
                level++;
            else if (context.token.value_type == closeType)
                level--;
        }

        context.tokenizer.copyIncludingValue(context.token, to_type.data);

        return error;
    }

    static inline void from(const JsonObjectOrArray &from_type, Token &token, Serializer &serializer)
    {
        token.value_type = JS::Type::Verbatim; //Need to fool the serializer to just write value as verbatim

        if (from_type.data.empty())
        {
            std::string emptyObjectOrArray("{}"); // Use object as default
            token.value = DataRef(emptyObjectOrArray);
            serializer.write(token);
        }
        else
        {
            token.value = DataRef(from_type.data);
            serializer.write(token);
        }
    }
};

namespace Internal
{
template<size_t INDEX, typename ...Ts>
struct TupleTypeHandler
{
    static inline Error to(JS::Tuple<Ts...> &to_type, ParseContext &context)
    {
        using Type = typename JS::TypeAt<sizeof...(Ts) - INDEX, Ts...>::type;
        Error error = TypeHandler<Type>::to(to_type.template get<sizeof...(Ts) - INDEX>(), context);
        if (error != JS::Error::NoError)
            return error;
        error = context.nextToken();
        if (error != JS::Error::NoError)
            return error;
        return TupleTypeHandler<INDEX - 1, Ts...>::to(to_type, context);
    }

    static inline void from(const JS::Tuple<Ts...> &from_type, Token &token, Serializer &serializer)
    {
        using Type = typename JS::TypeAt<sizeof...(Ts) - INDEX, Ts...>::type;
        TypeHandler<Type>::from(from_type.template get<sizeof...(Ts) - INDEX>(), token, serializer);
        TupleTypeHandler<INDEX - 1, Ts...>::from(from_type, token, serializer);
    }

};

/// \private
template<typename ...Ts>
struct TupleTypeHandler<0, Ts...>
{
    static inline Error to(JS::Tuple<Ts...>, ParseContext &context)
    {
        JS_UNUSED(context);
        return Error::NoError;
    }

    static inline void from(const JS::Tuple<Ts...> &from_type, Token &token, Serializer &serializer)
    {
        JS_UNUSED(from_type);
        JS_UNUSED(token);
        JS_UNUSED(serializer);
    }
};
}

/// \private
template<typename ...Ts>
struct TypeHandler<JS::Tuple<Ts...>>
{
    static inline Error to(JS::Tuple<Ts...> &to_type, ParseContext &context)
    {
        if (context.token.value_type != JS::Type::ArrayStart)
            return Error::ExpectedArrayStart;
        Error error = context.nextToken();
        if (error != JS::Error::NoError)
            return error;
        error = JS::Internal::TupleTypeHandler<sizeof...(Ts), Ts...>::to(to_type, context);
        if (error != JS::Error::NoError)
            return error;
        if (context.token.value_type != JS::Type::ArrayEnd)
            return Error::ExpectedArrayEnd;
        return Error::NoError;
    }

    static inline void from(const JS::Tuple<Ts...> &from_type, Token &token, Serializer &serializer)
    {
        token.value_type = Type::ArrayStart;
        token.value = DataRef("[");
        serializer.write(token);

        token.name = DataRef("");

        JS::Internal::TupleTypeHandler<sizeof...(Ts), Ts...>::from(from_type, token, serializer);
        token.name = DataRef("");

        token.value_type = Type::ArrayEnd;
        token.value = DataRef("]");
        serializer.write(token);
    }
};

namespace Internal
{
template<size_t INDEX, typename ...Ts>
struct StdTupleTypeHandler
{
    static inline Error to(std::tuple<Ts...> &to_type, ParseContext &context)
    {
        using Type = typename std::tuple_element<sizeof...(Ts) - INDEX, std::tuple<Ts...>>::type;
        Error error = TypeHandler<Type>::to(std::get<sizeof...(Ts) - INDEX>(to_type), context);
        if (error != JS::Error::NoError)
            return error;
        error = context.nextToken();
        if (error != JS::Error::NoError)
            return error;
        return StdTupleTypeHandler<INDEX - 1, Ts...>::to(to_type, context);
    }

    static inline void from(const JS::Tuple<Ts...> &from_type, Token &token, Serializer &serializer)
    {
        using Type = typename std::tuple_element<sizeof...(Ts) - INDEX, std::tuple<Ts...>>::type;
        TypeHandler<Type>::from(std::get<sizeof...(Ts) - INDEX>(from_type), token, serializer);
        StdTupleTypeHandler<INDEX - 1, Ts...>::from(from_type, token, serializer);
    }

};

/// \private
template<typename ...Ts>
struct StdTupleTypeHandler<0, Ts...>
{
    static inline Error to(std::tuple<Ts...>, ParseContext &context)
    {
        JS_UNUSED(context);
        return Error::NoError;
    }

    static inline void from(const std::tuple<Ts...> &from_type, Token &token, Serializer &serializer)
    {
        JS_UNUSED(from_type);
        JS_UNUSED(token);
        JS_UNUSED(serializer);
    }
};
}
/// \private
template<typename ...Ts>
struct TypeHandler<std::tuple<Ts...>>
{
    static inline Error to(std::tuple<Ts...> &to_type, ParseContext &context)
    {
        if (context.token.value_type != JS::Type::ArrayStart)
            return Error::ExpectedArrayStart;
        Error error = context.nextToken();
        if (error != JS::Error::NoError)
            return error;
        error = JS::Internal::StdTupleTypeHandler<sizeof...(Ts), Ts...>::to(to_type, context);
        if (error != JS::Error::NoError)
            return error;
        if (context.token.value_type != JS::Type::ArrayEnd)
            return Error::ExpectedArrayEnd;
        return Error::NoError;
    }

    static inline void from(const std::tuple<Ts...> &from_type, Token &token, Serializer &serializer)
    {
        token.value_type = Type::ArrayStart;
        token.value = DataRef("[");
        serializer.write(token);

        token.name = DataRef("");

        JS::Internal::StdTupleTypeHandler<sizeof...(Ts), Ts...>::from(from_type, token, serializer);
        token.name = DataRef("");

        token.value_type = Type::ArrayEnd;
        token.value = DataRef("]");
        serializer.write(token);
    }
};

template<typename T>
struct OneOrMany
{
    std::vector<T> data;
};

template<typename T>
class TypeHandler<OneOrMany<T>>
{
public:
    static inline Error to(OneOrMany<T> &to_type, ParseContext &context)
    {
        if (context.token.value_type == Type::ArrayStart)
        {
            context.error = TypeHandler<std::vector<T>>::to(to_type.data, context);
        }
        else {
            to_type.data.push_back(T());
            context.error = TypeHandler<T>::to(to_type.data.back(), context);
        }
        return context.error;
    }
    static void from(const OneOrMany<T> &from, Token &token, Serializer &serializer)
    {
        if (from.data.empty())
            return;
        if (from.data.size() > 1) {
            TypeHandler<std::vector<T>>::from(from.data, token, serializer);
        }
        else {
            TypeHandler<T>::from(from.data.front(), token, serializer);
        }
    }
};

template<typename T, size_t N>
class TypeHandler<T[N]>
{
public:
    static inline Error to(T (&to_type)[N], ParseContext &context)
    {
        if (context.token.value_type != Type::ArrayStart)
            return JS::Error::ExpectedArrayStart;

		context.nextToken();
        for (size_t i = 0; i < N; i++)
        {
			if (context.error != JS::Error::NoError)
				return context.error;
            context.error = TypeHandler<T>::to(to_type[i], context);
            if (context.error != JS::Error::NoError)
                return context.error;

			context.nextToken();
        }

        if (context.token.value_type != Type::ArrayEnd)
            return JS::Error::ExpectedArrayEnd;
        return context.error;
    }
    static void from(const T (&from)[N], Token &token, Serializer &serializer)
    {
        token.value_type = Type::ArrayStart;
        token.value = DataRef("[");
        serializer.write(token);

        token.name = DataRef("");
        for (size_t i = 0; i < N; i++)
            TypeHandler<T>::from(from[i], token, serializer);
        
        token.name = DataRef("");
        token.value_type = Type::ArrayEnd;
        token.value = DataRef("]");
        serializer.write(token);
    }
};
#ifdef JS_STD_UNORDERED_MAP
template<typename Key, typename Value>
class TypeHandler<std::unordered_map<Key, Value>>
{
public:
    static inline Error to(std::unordered_map<Key, Value> &to_type, ParseContext &context)
    {
        if (context.token.value_type != Type::ObjectStart)
        {
            return JS::Error::ExpectedObjectStart;
        }

        Error error = context.nextToken();
        if (error != JS::Error::NoError)
            return error;
        while (context.token.value_type != Type::ObjectEnd)
        {
            Value v;
            error = TypeHandler<Value>::to(v, context);
            to_type[Key(context.token.name.data, context.token.name.size)] = v;
            if (error != JS::Error::NoError)
                return error;
            error = context.nextToken();
        }

        return error;
    }

    static void from(const std::unordered_map<Key, Value> &from, Token &token, Serializer &serializer)
    {
        token.value_type = Type::ObjectStart;
        token.value = DataRef("{");
        serializer.write(token);
        for (auto it = from.begin(); it != from.end(); ++it)
        {
            token.name = DataRef(it->first);
            token.name_type = Type::String;
            TypeHandler<Value>::from(it->second, token, serializer);
        }
        token.name.size = 0;
        token.name.data = "";
        token.name_type = Type::String;
        token.value_type = Type::ObjectEnd;
        token.value = DataRef("}");
        serializer.write(token);
    }
};
#endif
} //Namespace
#endif //JSON_STRUCT_H
