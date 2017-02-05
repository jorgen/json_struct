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

#ifndef JSON_TOOLS_H
#define JSON_TOOLS_H

#include <stddef.h>
#include <functional>
#include <vector>
#include <string>
#include <algorithm>
#include <stdlib.h>
#include <memory>
#include <tuple>
#include <assert.h>
#include <atomic>

#ifdef _MSC_VER
#if _MSC_VER > 1800
#define JT_CONSTEXPR constexpr
#define JT_HAVE_CONSTEXPR 1
#else
#define JT_CONSTEXPR
#define JT_HAVE_CONSTEXPR 0
#endif
#else
#define JT_CONSTEXPR constexpr
#define JT_HAVE_CONSTEXPR 1
#endif

namespace JT {
struct DataRef
{
    DataRef()
        : data("")
        , size(0)
    {}

    DataRef(const char *data, size_t size)
        : data(data)
        , size(size)
    {}

    template <size_t N>
    static DataRef asDataRef(const char (&data)[N])
    {
        return DataRef(data, N - 1);
    }

    static DataRef asDataRef(const std::string &str)
    {
        return DataRef(&str[0], str.size());
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
    Null
};

struct Token
{
    Token();

    Type name_type;
    Type value_type;
    DataRef name;
    DataRef value;
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
    IlligalPropertyName,
    IlligalPropertyType,
    IlligalDataValue,
    EncounteredIlligalChar,
    CouldNotCreateNode,
    NodeNotFound,
    MissingPropertyMember,
    FailedToParseBoolen,
    FailedToParseDouble,
    FailedToParseFloat,
    FailedToParseInt,
    UnassignedRequiredMember,
    NonContigiousMemory,
    UnknownError,
    UserDefinedErrors
};

class ErrorContext
{
public:
    size_t line = 0;
    size_t character = 0;
    Error error = Error::NoError;
    std::vector<std::string> lines;

    void clear()
    {
        line = 0;
        character = 0;
        error = Error::NoError;
        lines.clear();
    }
};

namespace Internal {
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
        : index(index)
        , callbackContainer(callbackContainer)
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
        ref.store(other.load());
        callback = other.callback;
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
    ~Tokenizer();

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
    void registerTokenTransformer(std::function<void(Token &next_token)> token_transformer);
    Error nextToken(Token &next_token);

    void copyFromValue(const Token &token, std::string &to_buffer);
    void copyIncludingValue(const Token &token, std::string &to_buffer);

    std::string makeErrorString() const;
    void setErrorContextConfig(size_t lineContext, size_t rangeContext);
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
    void updateErrorContext(Error error);

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
    size_t callback_id;
    Internal::IntermediateToken intermediate_token;
    std::vector<DataRef> data_list;
    Internal::CallbackContainer<void(const char *)> release_callbacks;
    Internal::CallbackContainer<void(Tokenizer &)> need_more_data_callbacks;
    std::vector<std::pair<size_t, std::string *>> copy_buffers;
    const std::vector<Token> *parsed_data_vector;
    std::function<void(Token &next_token)> token_transformer;
    ErrorContext error_context;
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

    int depth() const;
    void setDepth(int depth);

    void skipDelimiter(bool skip);

    const std::string &prefix() const;
    const std::string &tokenDelimiter() const;
    const std::string &valueDelimiter() const;
    const std::string &postfix() const;

private:
    unsigned char m_shift_size;
    unsigned char m_depth;
    Style m_style : 1;
    bool m_convert_ascii_to_string : 1;

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
    void registerTokenTransformer(std::function<const Token&(const Token &)> token_transformer);

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
    std::function<const Token&(const Token &)> m_token_transformer;
};

// IMPLEMENTATION

inline Token::Token()
    : name_type(Type::String)
    , name()
    , value_type(Type::String)
    , value()
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
    , callback_id(0)
    , parsed_data_vector(nullptr)
{}
inline Tokenizer::~Tokenizer()
{}

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
    data_list.push_back(DataRef::asDataRef(data));
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
    if (parsed_data_vector) {
        next_token = (*parsed_data_vector)[cursor_index];
        cursor_index++;
        if (cursor_index == parsed_data_vector->size()) {
            cursor_index = 0;
            parsed_data_vector = nullptr;
        }
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

    if (error == Error::NoError && token_transformer != nullptr) {
        token_transformer(next_token);
    }

    return error;
}

inline void Tokenizer::registerTokenTransformer(std::function<void(Token &next_token)> token_transformer)
{
    token_transformer = token_transformer;
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
        assert(token.value.data > data_list.front().data && token.value.data < data_list.front().data + data_list.front().size);
        size_t index = token.value.data - data_list.front().data;
        auto pair = std::make_pair(index, &to_buffer);
        copy_buffers.push_back(pair);
    }

    

}
inline void Tokenizer::copyIncludingValue(const Token &token, std::string &to_buffer)
{
    auto it = std::find_if(copy_buffers.begin(), copy_buffers.end(), [&to_buffer] (const std::pair<size_t, std::string *> &pair) { return &to_buffer == pair.second; });
    assert(it != copy_buffers.end());
    assert(it->first <= cursor_index);
    std::string data(data_list.front().data + it->first, cursor_index - it->first);
    to_buffer += data;
    copy_buffers.erase(it);
}

inline std::string Tokenizer::makeErrorString() const
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
        "IlligalPropertyName",
        "IlligalPropertyType",
        "IlligalDataValue",
        "EncounteredIlligalChar",
        "CouldNotCreateNode",
        "NodeNotFound",
        "MissingPropertyMember",
        "FailedToParseBoolen",
        "FailedToParseDouble",
        "FailedToParseFloat",
        "FailedToParseInt",
        "UnassignedRequiredMember",
        "NonContigiousMemory",
        "UnknownError",
    };
    static_assert(sizeof(error_strings) / sizeof*error_strings == size_t(Error::UserDefinedErrors) , "Please add missing error message");

    std::string retString("Error ");
    retString += error_strings[int(error_context.error)] + std::string(":\n");
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
    for (size_t end = cursor_index; end < json_data.size; end++) {
        if (is_escaped) {
            is_escaped = false;
            continue;
        }
        switch (*(json_data.data + end)) {
        case '\\':
            is_escaped = true;
            break;
        case '"':
            *chars_ahead = end + 1 - cursor_index;
            return Error::NoError;

        default:
            break;
        }
    }
    return Error::NeedMoreData;
}

inline Error Tokenizer::findAsciiEnd(const DataRef &json_data, size_t *chars_ahead)
{
    assert(property_type == Type::Ascii);
    for (size_t end = cursor_index; end < json_data.size; end++) {
        char ascii_code = *(json_data.data + end);
        if ((ascii_code >= 'A' && ascii_code <= 'Z') ||
            (ascii_code >= '^' && ascii_code <= 'z') ||
            (ascii_code >= '0' && ascii_code <= '9')) {
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
    for (size_t end = cursor_index; end < json_data.size; end++) {
        char number_code = *(json_data.data + end);
        if ((number_code >= '0' && number_code <= '9'))
            continue;
        switch(number_code) {
        case '.':
        case '+':
        case '-':
        case 'e':
        case 'E':
            continue;
        default:
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
        switch (*(json_data.data + current_pos)) {
        case ' ':
        case '\n':
        case '\r':
        case '\t':
        case '\0':
            break;
        case '"':
            *type = Type::String;
            *chars_ahead = current_pos - cursor_index;
            return Error::NoError;
        case '{':
            *type = Type::ObjectStart;
            *chars_ahead = current_pos - cursor_index;
            return Error::NoError;
        case '}':
            *type = Type::ObjectEnd;
            *chars_ahead = current_pos - cursor_index;
            return Error::NoError;
        case '[':
            *type = Type::ArrayStart;
            *chars_ahead = current_pos - cursor_index;
            return Error::NoError;
        case ']':
            *type = Type::ArrayEnd;
            *chars_ahead = current_pos - cursor_index;
            return Error::NoError;
        case '-':
        case '+':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            *type = Type::Number;
            *chars_ahead = current_pos - cursor_index;
            return Error::NoError;
        default:
            char ascii_code = *(json_data.data + current_pos);
            if ((ascii_code >= 'A' && ascii_code <= 'Z') ||
                (ascii_code >= '^' && ascii_code <= 'z')) {
                *type = Type::Ascii;
                *chars_ahead = current_pos - cursor_index;;
                return Error::NoError;
            } else {
                *chars_ahead = current_pos - cursor_index;
                return Error::EncounteredIlligalChar;
            }
            break;
        }

    }
    return Error::NeedMoreData;
}

inline Error Tokenizer::findDelimiter(const DataRef &json_data, size_t *chars_ahead)
{
    for (size_t end = cursor_index; end < json_data.size; end++) {
        switch(*(json_data.data + end)) {
        case ':':
            token_state = InTokenState::FindingData;
            *chars_ahead = end + 1 - cursor_index;
            return Error::NoError;
        case ',':
            token_state = InTokenState::FindingName;
            *chars_ahead = end + 1 - cursor_index;
            return Error::NoError;
        case ']':
            token_state = InTokenState::FindingName;
            *chars_ahead = end - cursor_index;
            return Error::NoError;
        case ' ':
        case '\n':
        case '\r':
        case '\t':
        case '\0':
            break;
        default:
            return Error::ExpectedDelimiter;
            break;
        }
    }
    return Error::NeedMoreData;
}

inline Error Tokenizer::findTokenEnd(const DataRef &json_data, size_t *chars_ahead)
{
    for (size_t end = cursor_index; end < json_data.size; end++) {
        const char *data_at = json_data.data + end;
        switch(*(json_data.data + end)) {
        case ',':
            expecting_prop_or_annonymous_data = true;
            *chars_ahead = end + 1 - cursor_index;
            return Error::NoError;
        case '\n':
            if (allow_new_lines) {
                *chars_ahead = end + 1 - cursor_index;
                return Error::NoError;
            }
            break;
        case ']':
        case '}':
            *chars_ahead = end - cursor_index;
            return Error::NoError;
        case ' ':
        case '\t':
        case '\r':
        case '\0':
            break;
        default:
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

    int size_adjustment = 0;
    if (property_state == InPropertyState::FindingEnd) {
        switch (type) {
        case Type::String:
            error = findStringEnd(json_data, &diff);
            size_adjustment = -1;
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
        data.size = cursor_index - current_data_start + size_adjustment;
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
                data = DataRef::asDataRef(intermediate_token.name);
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
                        return Error::IlligalPropertyName;
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
                tmp_token.name = DataRef::asDataRef(intermediate_token.name);
                tmp_token.name_type = intermediate_token.name_type;
                data = DataRef::asDataRef(intermediate_token.data);
                type = intermediate_token.data_type;
            }

            tmp_token.value = data;
            tmp_token.value_type = Internal::getType(type, tmp_token.value.data, tmp_token.value.size);

            if (tmp_token.value_type  == Type::Ascii && !allow_ascii_properties)
                return Error::IlligalDataValue;

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

inline void Tokenizer::updateErrorContext(Error error)
{
    error_context.error = error;
    std::vector<Internal::Lines> lines;
    lines.push_back({0, cursor_index});
    const DataRef &json_data = data_list.front();
    const size_t stop_back = cursor_index - std::min(cursor_index, line_range_context);
    const size_t stop_forward = std::min(cursor_index + line_range_context, json_data.size);
    assert(cursor_index <= json_data.size);
    size_t lines_back = 0;
    size_t lines_forward = 0;
    size_t cursor_back;
    size_t cursor_forward;
    for (cursor_back = cursor_index - 1; cursor_back > stop_back; cursor_back--)
    {
        if (*(json_data.data + cursor_back) == '\n') {
            lines.front().start = cursor_back + 1;
            lines_back++;
            if (lines_back == 1)
                error_context.character = cursor_index - cursor_back;
            if (lines_back == line_context) {
                lines_back--;
                break;
            }

            lines.insert(lines.begin(), {0, cursor_back});
        }
    }
    if (lines.front().start == 0)
        lines.front().start = cursor_back;
    for (cursor_forward = cursor_index; cursor_forward < stop_forward; cursor_forward++)
    {
        if (*(json_data.data + cursor_forward) == '\n') {
            lines.back().end = cursor_forward;
            lines_forward++;
            if (lines_forward == line_context)
                break;
            lines.push_back({cursor_forward, 0});
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

        size_t left = cursor_index > range_context ? cursor_index - range_context : 0;
        size_t right = cursor_index + range_context > json_data.size ? json_data.size : cursor_index + range_context;
        error_context.character = cursor_index - left;
        error_context.lines.push_back(std::string(json_data.data + left, right - left));
    }
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

inline int SerializerOptions::depth() const { return m_depth; }

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

inline void SerializerOptions::setDepth(int depth)
{
    m_depth = depth;
    m_prefix = m_style == Pretty ? std::string(depth * m_shift_size, ' ') : std::string();
}

inline const std::string &SerializerOptions::prefix() const { return m_prefix; }
inline const std::string &SerializerOptions::tokenDelimiter() const { return m_token_delimiter; }
inline const std::string &SerializerOptions::valueDelimiter() const { return m_value_delimiter; }
inline const std::string &SerializerOptions::postfix() const { return m_postfix; }

inline bool SerializerBuffer::append(const char *data, size_t size)
{
    if (used + size > this->size)
        return false;

    memcpy(buffer + used, data, size);
    used += size;
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
    const Token &token =
        m_token_transformer == nullptr? in_token : m_token_transformer(in_token);
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

inline void Serializer::registerTokenTransformer(std::function<const Token&(const Token &)> token_transformer)
{
    this->m_token_transformer = token_transformer;
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

    written = write(data.data,data.size);
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

template<typename T, typename A = std::allocator<T>>
class SilentVector : public std::vector<T, A>
{
    using std::vector<T, A>::vector;
};

template<typename T, typename Deleter = std::default_delete<T>>
class SilentUniquePtr : public std::unique_ptr<T, Deleter>
{
    using std::unique_ptr<T, Deleter>::unique_ptr;
};

class JsonObjectRef : public DataRef
{
};

struct JsonObject : public std::string
{
};

struct JsonArrayRef : public DataRef
{
};

struct JsonArray : public std::string
{
};

template <typename T>
struct IsOptionalType {
    typedef char yes[1];
    typedef char no[2];

    template <typename C>
    static JT_CONSTEXPR yes& test_in_optional(typename C::IsOptionalType*);

    template <typename>
    static JT_CONSTEXPR no& test_in_optional(...);

    static JT_CONSTEXPR const bool value = sizeof(test_in_optional<T>(nullptr)) == sizeof(yes);
};

template <typename T>
struct IsOptionalType<std::unique_ptr<T>>
{
    static JT_CONSTEXPR const bool value = true;
};

struct ParseContext
{
    ParseContext()
    {}
    ParseContext(const char *data, size_t size)
    {
        tokenizer.addData(data, size);
    }
    template<size_t SIZE>
    ParseContext(const char (&data)[SIZE])
    {
        tokenizer.addData(data);
    }

    Tokenizer tokenizer;
    Token token;
    Error error = Error::NoError;
    std::vector<std::string> missing_members;
    std::vector<std::string> unassigned_required_members;
    bool allow_missing_members = true;
    bool allow_unnasigned_required__members = true;
};

#define JT_MEMBER(name) JT::makeMemberInfo(#name, &JT_STRUCT_T::name)

#define JT_SUPER_CLASS(super) JT::Internal::SuperInfo<super>(std::string(#super))

#define JT_SUPER_CLASSES(...) std::make_tuple(__VA_ARGS__)

#define JT_STRUCT(...) \
    template<typename JT_STRUCT_T> \
    struct JsonToolsBase \
    { \
       static const decltype(std::make_tuple(__VA_ARGS__)) &jt_static_meta_data_info() \
       { static auto ret = std::make_tuple(__VA_ARGS__); return ret; } \
       static const decltype(std::make_tuple()) &jt_static_meta_super_info() \
       { static auto ret = std::make_tuple(); return ret; } \
    };

#define JT_STRUCT_WITH_SUPER(super_list, ...) \
    template<typename JT_STRUCT_T> \
    struct JsonToolsBase \
    { \
       static const decltype(std::make_tuple(__VA_ARGS__)) &jt_static_meta_data_info() \
       { static auto ret = std::make_tuple(__VA_ARGS__); return ret; } \
        static const decltype(super_list) &jt_static_meta_super_info() \
        { static auto ret = super_list; return ret; } \
    };

namespace Internal {
    template <typename T>
    struct HasJsonToolsBase {
        typedef char yes[1];
        typedef char no[2];

        template <typename C>
        static JT_CONSTEXPR yes& test_in_base(typename C::template JsonToolsBase<C>*);

        template <typename>
        static JT_CONSTEXPR no& test_in_base(...);

        static JT_CONSTEXPR const bool value = sizeof(test_in_base<T>(nullptr)) == sizeof(yes);
    };

    template<typename T, typename U, size_t NAME_SIZE>
    struct MemberInfo
    {
        const char *name;
        T U::* member;
        typedef T type;
    };

    template<typename T>
    struct SuperInfo
    {
        explicit
            SuperInfo(const std::string &name)
            : name(name)
        {}
        const std::string name;
        typedef T type;
    };
}
template<typename T, typename U, size_t NAME_SIZE>
JT_CONSTEXPR Internal::MemberInfo<T, U, NAME_SIZE - 1> makeMemberInfo(const char(&name)[NAME_SIZE], T U::* member)
{
    return{ name, member };
}

template<typename T, typename specifier>
class TokenParser
{
public:
    static inline Error unpackToken(T &to_type, ParseContext &context)
    {
        static_assert(sizeof(T) == 0xffffffff, "Missing TokenParser specialisation\n");
        return Error::NoError;
    }

    static inline void serializeToken(const T &from_type, Token &token, Serializer &serializer)
    {
        static_assert(sizeof(T) == 0xffffffff, "Missing TokenParser specialisation\n");
    }
};

namespace Internal {
    template<typename T, typename MI_T, typename MI_M, size_t MI_S>
    inline Error unpackMember(T &to_type, const MemberInfo<MI_T, MI_M, MI_S> &memberInfo, ParseContext &context, size_t index, bool *assigned_members, const char *super_name)
    {
        if (MI_S == context.token.name.size && memcmp(memberInfo.name, context.token.name.data, MI_S) == 0)
        {
#if JT_HAVE_CONSTEXPR
            assigned_members[index] = true;
#endif
            return TokenParser<MI_T, MI_T>::unpackToken(to_type.*memberInfo.member, context);
        }
        return Error::MissingPropertyMember;
    }

    template<typename MI_T, typename MI_M, size_t MI_S>
    inline Error verifyMember(const MemberInfo<MI_T, MI_M, MI_S> &memberInfo, size_t index, bool *assigned_members, std::vector<std::string> &missing_members, const char *super_name)
    {
#if JT_HAVE_CONSTEXPR
        if (assigned_members[index])
            return Error::NoError;
        if (IsOptionalType<MI_T>::value)
            return Error::NoError;

        std::string to_push = strlen(super_name) ? std::string(super_name) + "::" : std::string();
        to_push += std::string(memberInfo.name, MI_S);
        missing_members.push_back(to_push);
        return Error::UnassignedRequiredMember;
#else
        return Error::NoError;
#endif
    }

    template<typename T, typename MI_T, typename MI_M, size_t MI_S>
    inline void serializeMember(const T &from_type, const MemberInfo<MI_T, MI_M, MI_S> &memberInfo, Token &token, Serializer &serializer, const char *super_name)
    {
        token.name.data = memberInfo.name;
        token.name.size = MI_S;
        token.name_type = Type::Ascii;

        TokenParser<MI_T, MI_T>::serializeToken(from_type.*memberInfo.member, token, serializer);
    }

    template<typename T, size_t PAGE, size_t INDEX>
    struct SuperClassHandler
    {
        static Error handleSuperClasses(T &to_type, ParseContext &context, bool *assigned_members);
        static Error verifyMembers(bool *assigned_members, std::vector<std::string> &missing_members);
        static JT_CONSTEXPR size_t membersInSuperClasses();
        static void serializeMembers(const T &from_type, Token &token, Serializer &serializer);
    };

    template<typename T, size_t PAGE, size_t SIZE>
    struct StartSuperRecursion
    {
        static Error start(T &to_type, ParseContext &context, bool *assigned)
        {
            return SuperClassHandler<T, PAGE, SIZE - 1>::handleSuperClasses(to_type, context, assigned);
        }

        static Error verifyMembers(bool *assigned_members, std::vector<std::string> &missing_members)
        {
            return SuperClassHandler<T, PAGE, SIZE - 1>::verifyMembers(assigned_members, missing_members);
        }

        static JT_CONSTEXPR size_t membersInSuperClasses()
        {
            return SuperClassHandler<T, PAGE, SIZE - 1>::membersInSuperClasses();
        }

        static void serializeMembers(const T &from_type, Token &token, Serializer &serializer)
        {
            return SuperClassHandler<T, PAGE, SIZE - 1>::serializeMembers(from_type, token, serializer);
        }
    };

    template<typename T, size_t PAGE>
    JT_CONSTEXPR size_t memberCount()
    {
        using Members = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_data_info())>::type;
        using SuperMeta = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_super_info())>::type;
        return std::tuple_size<Members>::value + StartSuperRecursion<T, PAGE + std::tuple_size<Members>::value, std::tuple_size<SuperMeta>::value>::membersInSuperClasses();
    }

    template<typename T, size_t PAGE>
    struct StartSuperRecursion<T, PAGE, 0>
    {
        static Error start(T &to_type, ParseContext &context, bool *assigned)
        {
            return Error::MissingPropertyMember;
        }

        static Error verifyMembers(bool *assigned_members, std::vector<std::string> &missing_members)
        {
            return Error::NoError;
        }

        static JT_CONSTEXPR size_t membersInSuperClasses()
        {
            return 0;
        }

        static void serializeMembers(const T &from_type, Token &token, Serializer &serializer)
        {
        }
    };

    template<typename T, typename Members, size_t PAGE, size_t INDEX>
    struct MemberChecker
    {
        static Error unpackMembers(T &to_type, const Members &members, ParseContext &context, bool *assigned_members, const char *super_name)
        {
            Error error = unpackMember(to_type, std::get<INDEX>(members), context, PAGE + INDEX, assigned_members, super_name);
            if (error != Error::MissingPropertyMember)
                return error;

            return MemberChecker<T, Members, PAGE, INDEX - 1>::unpackMembers(to_type, members, context, assigned_members, super_name);
        }

        static Error verifyMembers(const Members &members, bool *assigned_members, std::vector<std::string> &missing_members, const char *super_name)
        {
            Error memberError = verifyMember(std::get<INDEX>(members), PAGE + INDEX, assigned_members, missing_members, super_name);
            Error error = MemberChecker<T, Members, PAGE, INDEX - 1>::verifyMembers(members, assigned_members, missing_members, super_name);
            if (memberError != Error::NoError)
                return memberError;
            return error;
        }
        static void serializeMembers(const T &from_type, const Members &members, Token &token, Serializer &serializer, const char *super_name)
        {
            serializeMember(from_type, std::get<std::tuple_size<Members>::value - INDEX - 1>(members), token, serializer, super_name);
            MemberChecker<T, Members, PAGE, INDEX - 1>::serializeMembers(from_type, members, token, serializer, super_name);
        }
    };

    template<typename T, typename Members, size_t PAGE>
    struct MemberChecker<T, Members, PAGE, 0>
    {
        static Error unpackMembers(T &to_type, const Members &members, ParseContext &context, bool *assigned_members, const char *super_name)
        {
            Error error = unpackMember(to_type, std::get<0>(members), context, PAGE, assigned_members, super_name);
            if (error != Error::MissingPropertyMember)
                return error;

            using Super = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_super_info())>::type;
            return StartSuperRecursion<T, PAGE + std::tuple_size<Members>::value, std::tuple_size<Super>::value>::start(to_type, context, assigned_members);
        }

        static Error verifyMembers(const Members &members, bool *assigned_members, std::vector<std::string> &missing_members, const char *super_name)
        {
            Error memberError = verifyMember(std::get<0>(members), PAGE, assigned_members, missing_members, super_name);
            using Super = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_super_info())>::type;
            Error superError = StartSuperRecursion<T, PAGE + std::tuple_size<Members>::value, std::tuple_size<Super>::value>::verifyMembers(assigned_members, missing_members);
            if (memberError != Error::NoError)
                return memberError;
            return superError;
        }

        static void serializeMembers(const T &from_type, const Members &members, Token &token, Serializer &serializer, const char *super_name)
        {
            serializeMember(from_type, std::get<std::tuple_size<Members>::value - 1>(members), token, serializer, super_name);
            using Super = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_super_info())>::type;
            StartSuperRecursion<T, PAGE + std::tuple_size<Members>::value, std::tuple_size<Super>::value>::serializeMembers(from_type, token, serializer);

        }
    };

    template<typename T, size_t PAGE, size_t INDEX>
    Error SuperClassHandler<T, PAGE, INDEX>::handleSuperClasses(T &to_type, ParseContext &context, bool *assigned_members)
    {
        using SuperMeta = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_super_info())>::type;
        using Super = typename std::tuple_element<INDEX, SuperMeta>::type::type;
        using Members = typename std::remove_reference<decltype(Super::template JsonToolsBase<Super>::jt_static_meta_data_info())>::type;
        using T_Members = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_data_info())>::type;
        auto &members = Super::template JsonToolsBase<Super>::jt_static_meta_data_info();
        const char *super_name = std::get<INDEX>(T::template JsonToolsBase<T>::jt_static_meta_super_info()).name.c_str();
        Error error = MemberChecker<Super, Members, PAGE, std::tuple_size<Members>::value - 1>::unpackMembers(static_cast<Super &>(to_type), members, context, assigned_members, super_name);
        if (error != Error::MissingPropertyMember)
            return error;
#if JT_HAVE_CONSTEXPR
        return SuperClassHandler<T, PAGE + memberCount<Super, 0>(), INDEX - 1>::handleSuperClasses(to_type, context, assigned_members);
#else
        return SuperClassHandler<T, PAGE, INDEX - 1>::handleSuperClasses(to_type, context, assigned_members);
#endif
    }

    template<typename T, size_t PAGE, size_t INDEX>
    Error SuperClassHandler<T, PAGE, INDEX>::verifyMembers(bool *assigned_members, std::vector<std::string> &missing_members)
    {
        using SuperMeta = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_super_info())>::type;
        using Super = typename std::tuple_element<INDEX, SuperMeta>::type::type;
        using Members = typename std::remove_reference<decltype(Super::template JsonToolsBase<Super>::jt_static_meta_data_info())>::type;
        using T_Members = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_data_info())>::type;
        auto &members = Super::template JsonToolsBase<Super>::jt_static_meta_data_info();
        const char *super_name = std::get<INDEX>(T::template JsonToolsBase<T>::jt_static_meta_super_info()).name.c_str();
        Error error = MemberChecker<Super, Members, PAGE, std::tuple_size<Members>::value - 1>::verifyMembers(members, assigned_members, missing_members, super_name);
#if JT_HAVE_CONSTEXPR
        Error superError = SuperClassHandler<T, PAGE + memberCount<Super, 0>(), INDEX - 1>::verifyMembers(assigned_members, missing_members);
#else
        Error superError = SuperClassHandler<T, PAGE, INDEX - 1>::verifyMembers(assigned_members, missing_members);
#endif
        if (error != Error::NoError)
            return error;
        return superError;
    }

    template<typename T, size_t PAGE, size_t INDEX>
    size_t JT_CONSTEXPR SuperClassHandler<T, PAGE, INDEX>::membersInSuperClasses()
    {
        using SuperMeta = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_super_info())>::type;
        using Super = typename std::tuple_element<INDEX, SuperMeta>::type::type;
#if JT_HAVE_CONSTEXPR
        return memberCount<Super, PAGE>() + SuperClassHandler<T, PAGE + memberCount<Super, PAGE>(), INDEX - 1>::membersInSuperClasses();
#else
        return memberCount<Super, PAGE>() + SuperClassHandler<T, PAGE, INDEX - 1>::membersInSuperClasses();
#endif
    }

    template<typename T, size_t PAGE, size_t INDEX>
    void SuperClassHandler<T, PAGE, INDEX>::serializeMembers(const T &from_type, Token &token, Serializer &serializer)
    {
        using SuperMeta = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_super_info())>::type;
        using Super = typename std::tuple_element<INDEX, SuperMeta>::type::type;
        using Members = typename std::remove_reference<decltype(Super::template JsonToolsBase<Super>::jt_static_meta_data_info())>::type;
        auto &members = Super::template JsonToolsBase<Super>::jt_static_meta_data_info();
        const char *super_name = std::get<INDEX>(T::template JsonToolsBase<T>::jt_static_meta_super_info()).name.c_str();
        MemberChecker<Super, Members, PAGE, std::tuple_size<Members>::value - 1>::serializeMembers(from_type, members, token, serializer, "");
#if JT_HAVE_CONSTEXPR
        SuperClassHandler<T, PAGE + memberCount<Super, 0>(), INDEX - 1>::serializeMembers(from_type, token, serializer);
#else
        SuperClassHandler<T, PAGE, INDEX - 1>::serializeMembers(from_type, token, serializer);
#endif
    }

    template<typename T, size_t PAGE>
    struct SuperClassHandler<T, PAGE, 0>
    {
        static Error handleSuperClasses(T &to_type, ParseContext &context, bool *assigned_members)
        {
            using Meta = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_super_info())>::type;
            using Super = typename std::tuple_element<0, Meta>::type::type;
            using Members = typename std::remove_reference<decltype(Super::template JsonToolsBase<Super>::jt_static_meta_data_info())>::type;
            auto &members = Super::template JsonToolsBase<Super>::jt_static_meta_data_info();
            const char *super_name = std::get<0>(T::template JsonToolsBase<T>::jt_static_meta_super_info()).name.c_str();
            return MemberChecker<Super, Members, PAGE, std::tuple_size<Members>::value - 1>::unpackMembers(static_cast<Super &>(to_type), members, context, assigned_members, super_name);
        }
        static Error verifyMembers(bool *assigned_members, std::vector<std::string> &missing_members)
        {
            using SuperMeta = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_super_info())>::type;
            using Super = typename std::tuple_element<0, SuperMeta>::type::type;
            using Members = typename std::remove_reference<decltype(Super::template JsonToolsBase<Super>::jt_static_meta_data_info())>::type;
            auto &members = Super::template JsonToolsBase<Super>::jt_static_meta_data_info();
            const char *super_name = std::get<0>(T::template JsonToolsBase<T>::jt_static_meta_super_info()).name.c_str();
            return MemberChecker<Super, Members, PAGE, std::tuple_size<Members>::value - 1>::verifyMembers(members, assigned_members, missing_members, super_name);
        }
        JT_CONSTEXPR static size_t membersInSuperClasses()
        {
            using SuperMeta = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_super_info())>::type;
            using Super = typename std::tuple_element<0, SuperMeta>::type::type;
            return memberCount<Super, PAGE>();
        }
        static void serializeMembers(const T &from_type, Token &token, Serializer &serializer)
        {
            using SuperMeta = typename std::remove_reference<decltype(T::template JsonToolsBase<T>::jt_static_meta_super_info())>::type;
            using Super = typename std::tuple_element<0, SuperMeta>::type::type;
            using Members = typename std::remove_reference<decltype(Super::template JsonToolsBase<Super>::jt_static_meta_data_info())>::type;
            auto &members = Super::template JsonToolsBase<Super>::jt_static_meta_data_info();
            const char *super_name = std::get<0>(T::template JsonToolsBase<T>::jt_static_meta_super_info()).name.c_str();
            MemberChecker<Super, Members, PAGE, std::tuple_size<Members>::value - 1>::serializeMembers(from_type, members, token, serializer, "");
        }
    };

    static void skipToNext(Error &error, Token &token, Tokenizer &tokenizer)
    {
        assert(error == Error::NoError);
        Type end_type;
        if (token.value_type == Type::ObjectStart) {
            end_type = Type::ObjectEnd;
        }
        else if (token.value_type == Type::ArrayStart) {
            end_type = Type::ArrayEnd;
        }
        else {
            error = tokenizer.nextToken(token);
            return;
        }
        while (error == Error::NoError && token.value_type != end_type) {
            error = tokenizer.nextToken(token);
            if (token.value_type == Type::ObjectStart
                || token.value_type == Type::ArrayStart) {
                skipToNext(error, token, tokenizer);
                if (error != Error::NoError)
                    return;
                error = tokenizer.nextToken(token);
            }
        }
    }
}
template<typename T>
class TokenParser<T, typename std::enable_if<Internal::HasJsonToolsBase<T>::value, T>::type>
{
public:
    static inline Error unpackToken(T &to_type, ParseContext &context)
    {
        if (context.token.value_type != JT::Type::ObjectStart)
            return Error::ExpectedObjectStart;
        Error error = context.tokenizer.nextToken(context.token);
        if (error != JT::Error::NoError)
            return error;
        auto members = T::template JsonToolsBase<T>::jt_static_meta_data_info();
#if JT_HAVE_CONSTEXPR
        bool assigned_members[Internal::memberCount<T, 0>()];
        memset(assigned_members, 0, sizeof(assigned_members));
#else
        bool *assigned_members = nullptr;
#endif
        while(context.token.value_type != JT::Type::ObjectEnd)
        {
            std::string token_name(context.token.name.data, context.token.name.size);
            error = Internal::MemberChecker<T, decltype(members), 0 ,std::tuple_size<decltype(members)>::value - 1>::unpackMembers(to_type, members, context, assigned_members, "");
            if (error == Error::MissingPropertyMember) {
                context.missing_members.push_back(token_name);
                if (!context.allow_missing_members)
                    return error;
            } else if (error != Error::NoError) {
                return error;
            }
            error = Error::NoError;
            Internal::skipToNext(error, context.token, context.tokenizer);

            if (error != Error::NoError)
                return error;
        }
        assert(error == Error::NoError);
        std::vector<std::string> unassigned_required_members;
        error = Internal::MemberChecker<T, decltype(members), 0, std::tuple_size<decltype(members)>::value - 1>::verifyMembers(members, assigned_members, unassigned_required_members, "");
        if (error == Error::UnassignedRequiredMember) {
            context.unassigned_required_members.insert(context.unassigned_required_members.end(),unassigned_required_members.begin(), unassigned_required_members.end());
            if (context.allow_unnasigned_required__members)
                error = Error::NoError;
        }
        return error;
    }

    static inline void serializeToken(const T &from_type, Token &token, Serializer &serializer)
    {
        static const char objectStart[] = "{";
        static const char objectEnd[] = "}";
        token.value_type = Type::ObjectStart;
        token.value = DataRef::asDataRef(objectStart);
        serializer.write(token);
        auto members = T::template JsonToolsBase<T>::jt_static_meta_data_info();
        Internal::MemberChecker<T, decltype(members), 0, std::tuple_size<decltype(members)>::value - 1>::serializeMembers(from_type, members, token, serializer, "");
        token.name.size = 0;
        token.name.data = "";
        token.name_type = Type::String;
        token.value_type = Type::ObjectEnd;
        token.value = DataRef::asDataRef(objectEnd);
        serializer.write(token);
    }
};

template<>
inline Error TokenParser<std::string, std::string>::unpackToken(std::string &to_type, ParseContext &context)
{
    to_type = std::string(context.token.value.data, context.token.value.size);
    return Error::NoError;
}

template<>
inline void TokenParser<std::string, std::string>::serializeToken(const std::string &str, Token &token, Serializer &serializer)
{
    token.value_type = Type::Ascii;
    token.value.data = &str[0];
    token.value.size = str.size();
    serializer.write(token);
}

template<>
inline Error TokenParser<double, double>::unpackToken(double &to_type, ParseContext &context)
{
    char *pointer;
    to_type = strtod(context.token.value.data, &pointer);
    if (context.token.value.data == pointer)
        return Error::FailedToParseFloat;
    return Error::NoError;
}

namespace Internal {
    template<typename ...Ts>
    static int jt_snprintf(char *dst, size_t max, const char * format, Ts ...args)
    {
#ifdef _MSC_VER
        return _snprintf_s(dst, max, max, format, args...);
#else
        return snprintf(dst, max, format, args...);
#endif
    }
}

template<>
inline void TokenParser<double, double>::serializeToken(const double &d, Token &token, Serializer &serializer)
{
    //char buf[1/*'-'*/ + (DBL_MAX_10_EXP+1)/*308+1 digits*/ + 1/*'.'*/ + 6/*Default? precision*/ + 1/*\0*/];
    char buf[1 + (308+1)/*308+1 digits*/ + 1/*'.'*/ + 6/*Default? precision*/ + 1/*\0*/];

    int size = Internal::jt_snprintf(buf, sizeof buf / sizeof *buf, "%f", d);

    if (size < 0) {
        fprintf(stderr, "error serializing float token\n");
        return;
    }

    token.value_type = Type::Number;
    token.value.data = buf;
    token.value.size = size;
    serializer.write(token);
}

template<>
inline Error TokenParser<float, float>::unpackToken(float &to_type, ParseContext &context)
{
    char *pointer;
    to_type = strtof(context.token.value.data, &pointer);
    if (context.token.value.data == pointer)
        return Error::FailedToParseFloat;
    return Error::NoError;
}

template<>
inline void TokenParser<float, float>::serializeToken(const float &f, Token &token, Serializer &serializer)
{
    //char buf[1/*'-'*/ + (FLT_MAX_10_EXP+1)/*38+1 digits*/ + 1/*'.'*/ + 6/*Default? precision*/ + 1/*\0*/];
    char buf[1/*'-'*/ + (38+1)/*38+1 digits*/ + 1/*'.'*/ + 6/*Default? precision*/ + 1/*\0*/];
    int size = Internal::jt_snprintf(buf, sizeof buf/sizeof *buf, "%f",f);
    if (size < 0) {
        fprintf(stderr, "error serializing float token\n");
        return;
    }

    token.value_type = Type::Number;
    token.value.data = buf;
    token.value.size = size;
    serializer.write(token);
}

template<>
inline Error TokenParser<int, int>::unpackToken(int &to_type, ParseContext &context)
{
    char *pointer;
    to_type = strtol(context.token.value.data, &pointer, 10);
    if (context.token.value.data == pointer)
        return Error::FailedToParseInt;
    return Error::NoError;
}

template<>
inline void TokenParser<int, int>::serializeToken(const int&d, Token &token, Serializer &serializer)
{
    char buf[10];
    int size = Internal::jt_snprintf(buf, sizeof buf / sizeof *buf, "%d", d);
    if (size < 0) {
        fprintf(stderr, "error serializing int token\n");
        return;
    }

    token.value_type = Type::Number;
    token.value.data = buf;
    token.value.size = size;
    serializer.write(token);
}

template<typename T>
class TokenParser<Optional<T>, Optional<T>>
{
public:
    static inline Error unpackToken(Optional<T> &to_type, ParseContext &context)
    {
        return TokenParser<T,T>::unpackToken(to_type.data, context);
    }

    static void serializeToken(const Optional<T> &opt, Token &token, Serializer &serializer)
    {
        TokenParser<T,T>::serializeToken(opt(), token, serializer);
    }
};

template<typename T>
class TokenParser<OptionalChecked<T>, OptionalChecked<T>>
{
public:
    static inline Error unpackToken(OptionalChecked<T> &to_type, ParseContext &context)
    {
        to_type.assigned = true;
        return TokenParser<T,T>::unpackToken(to_type.data, context);
    }

    static void serializeToken(const OptionalChecked<T> &opt, Token &token, Serializer &serializer)
    {
        if (opt.assigned)
            TokenParser<T,T>::serializeToken(opt(), token, serializer);
    }
};

template<typename T>
class TokenParser<std::unique_ptr<T>, std::unique_ptr<T>>
{
public:
    static inline Error unpackToken(std::unique_ptr<T> &to_type, ParseContext &context)
    {
        if (context.token.value_type != Type::Null) {
            to_type.reset(new T());
            return TokenParser<T,T>::unpackToken(*to_type.get(), context);
        }
        to_type.reset(nullptr);
        return Error::NoError;
    }

    static void serializeToken(const std::unique_ptr<T> &unique, Token &token, Serializer &serializer)
    {
        if (unique) {
            TokenParser<T,T>::serializeToken(*unique.get(), token, serializer);
        } else {
            const char nullChar[] = "null";
            token.value_type = Type::Null;
            token.value = DataRef::asDataRef(nullChar);
            serializer.write(token);
        }
    }

};

template<>
inline Error TokenParser<bool, bool>::unpackToken(bool &to_type, ParseContext &context)
{
    if (context.token.value.size == sizeof("true") - 1 && memcmp("true", context.token.value.data, sizeof("true") - 1) == 0)
        to_type = true;
    else if (context.token.value.size == sizeof("false") - 1 && memcmp("false", context.token.value.data, sizeof("false") - 1) == 0)
        to_type = false;
    else
        return Error::FailedToParseBoolen;

    return Error::NoError;
}

template<>
inline void TokenParser<bool, bool>::serializeToken(const bool &b, Token &token, Serializer &serializer)
{
    const char trueChar[] = "true";
    const char falseChar[] = "false";
    token.value_type = Type::Bool;
    if (b) {
        token.value = DataRef::asDataRef(trueChar);
    } else {
        token.value =  DataRef::asDataRef(falseChar);
    }
    serializer.write(token);
}

template<typename T>
class TokenParser<std::vector<T>, std::vector<T>>
{
public:
    static inline Error unpackToken(std::vector<T> &to_type, ParseContext &context)
    {
        if (context.token.value_type != JT::Type::ArrayStart)
            return Error::ExpectedArrayStart;
        Error error = context.tokenizer.nextToken(context.token);
        if (error != JT::Error::NoError)
            return error;
        while(context.token.value_type != JT::Type::ArrayEnd)
        {
            to_type.push_back(T());
            error = TokenParser<T,T>::unpackToken(to_type.back(), context);
            if (error != JT::Error::NoError)
                break;
            error = context.tokenizer.nextToken(context.token);
            if (error != JT::Error::NoError)
                break;
        }

        return error;
    }

    static void serializeToken(const std::vector<T> &vec, Token &token, Serializer &serializer)
    {
        static const char arrayStart[] = "[";
        static const char arrayEnd[] = "]";
        token.value_type = Type::ArrayStart;
        token.value = DataRef::asDataRef(arrayStart);
        serializer.write(token);

        token.name = DataRef::asDataRef("");

        for (auto &index : vec)
        {
            TokenParser<T,T>::serializeToken(index, token, serializer);
        }

        token.name = DataRef::asDataRef("");

        token.value_type = Type::ArrayEnd;
        token.value = DataRef::asDataRef(arrayEnd);
        serializer.write(token);
    }
};

template<typename T>
class TokenParser<SilentVector<T>, SilentVector<T>>
{
public:
    static inline Error unpackToken(SilentVector<T> &to_type, ParseContext &context)
    {
        return TokenParser<std::vector<T>, std::vector<T>>::unpackToken(to_type, context);
    }

    static void serializeToken(const SilentVector<T> &vec, Token &token, Serializer &serializer)
    {
        if (vec.size()) {
            TokenParser<std::vector<T>, std::vector<T>>::serializeToken(vec, token, serializer);
        }
    }
};

template<typename T>
class TokenParser<SilentUniquePtr<T>, SilentUniquePtr<T>>
{
public:
    static inline Error unpackToken(SilentUniquePtr<T> &to_type, ParseContext &context)
    {
        return TokenParser<std::unique_ptr<T>, std::unique_ptr<T>>::unpackToken(to_type, context);
    }

    static void serializeToken(const SilentUniquePtr<T> &ptr, Token &token, Serializer &serializer)
    {
        if (ptr) {
            TokenParser<std::unique_ptr<T>, std::unique_ptr<T>>::serializeToken(ptr, token, serializer);
        }
    }
};

template<>
inline Error TokenParser<JsonArrayRef,JsonArrayRef>::unpackToken(JsonArrayRef &to_type, ParseContext &context)
{
    if (context.token.value_type != JT::Type::ArrayStart)
        return Error::ExpectedArrayStart;

    bool buffer_change = false;
    auto ref = context.tokenizer.registerNeedMoreDataCallback([&buffer_change] (JT::Tokenizer &tokenizer)
                                                               {
                                                                    buffer_change = true;
                                                               });

    size_t level = 1;
    Error error = Error::NoError;
    while(error != JT::Error::NoError && level && buffer_change == false) {
        error = context.tokenizer.nextToken(context.token);
        if (context.token.value_type == Type::ArrayStart)
            level++;
        else if (context.token.value_type == Type::ArrayEnd)
            level--;
    }
    if (buffer_change)
        return Error::NonContigiousMemory;

    return error;
}

template<>
inline void TokenParser<JsonArrayRef, JsonArrayRef>::serializeToken(const JsonArrayRef &from_type, Token &token, Serializer &serializer)
{
    serializer.write(from_type.data, from_type.size);
}

template<>
inline Error TokenParser<JsonArray,JsonArray>::unpackToken(JsonArray &to_type, ParseContext &context)
{
    if (context.token.value_type != JT::Type::ArrayStart)
        return Error::ExpectedArrayStart;

    context.tokenizer.copyFromValue(context.token, to_type);

    size_t level = 1;
    Error error = Error::NoError;
    while(error != JT::Error::NoError && level) {
        error = context.tokenizer.nextToken(context.token);
        if (context.token.value_type == Type::ArrayStart)
            level++;
        else if (context.token.value_type == Type::ArrayStart)
            level--;
    }

    context.tokenizer.copyIncludingValue(context.token, to_type);

    return error;
}

template<>
inline void TokenParser<JsonArray, JsonArray>::serializeToken(const JsonArray &from_type, Token &token, Serializer &serializer)
{
    serializer.write(from_type);
}

template<>
inline Error TokenParser<JsonObjectRef,JsonObjectRef>::unpackToken(JsonObjectRef &to_type, ParseContext &context)
{
    if (context.token.value_type != JT::Type::ObjectStart)
        return Error::ExpectedObjectStart;

    bool buffer_change = false;
    auto ref = context.tokenizer.registerNeedMoreDataCallback([&buffer_change] (JT::Tokenizer &tokenizer)
                                                               {
                                                                    buffer_change = true;
                                                               });

    size_t level = 1;
    Error error = Error::NoError;
    while(error != JT::Error::NoError && level && buffer_change == false) {
        error = context.tokenizer.nextToken(context.token);
        if (context.token.value_type == Type::ObjectStart)
            level++;
        else if (context.token.value_type == Type::ObjectEnd)
            level--;
    }
    if (buffer_change)
        return Error::NonContigiousMemory;

    return error;
}

template<>
inline void TokenParser<JsonObjectRef, JsonObjectRef>::serializeToken(const JsonObjectRef &from_type, Token &token, Serializer &serializer)
{
    serializer.write(from_type.data, from_type.size);
}

template<>
inline Error TokenParser<JsonObject,JsonObject>::unpackToken(JsonObject &to_type, ParseContext &context)
{
    if (context.token.value_type != JT::Type::ObjectStart)
        return Error::ExpectedObjectStart;

    context.tokenizer.copyFromValue(context.token, to_type);

    size_t level = 1;
    Error error = Error::NoError;
    while(error != JT::Error::NoError && level) {
        error = context.tokenizer.nextToken(context.token);
        if (context.token.value_type == Type::ObjectStart)
            level++;
        else if (context.token.value_type == Type::ObjectEnd)
            level--;
    }

    context.tokenizer.copyIncludingValue(context.token, to_type);

    return error;
}

template<>
inline void TokenParser<JsonObject, JsonObject>::serializeToken(const JsonObject &from_type, Token &token, Serializer &serializer)
{
    serializer.write(from_type);
}

template<typename T>
void parseData(T &to_type, ParseContext &context)
{
    context.error = context.tokenizer.nextToken(context.token);
    if (context.error != JT::Error::NoError)
        return;
    context.error = TokenParser<T,T>::unpackToken(to_type, context);
    return;
}

template<size_t SIZE>
struct SerializerContext
{
    char outBuffer[SIZE];
    Serializer serializer;
    BufferRequestCBRef cbRef;
    SerializerContext()
        : serializer(outBuffer, SIZE)
        , cbRef(serializer.addRequestBufferCallback([this](Serializer &serializer)
                                                    {
                                                        return_string += std::string(outBuffer, sizeof(outBuffer));
                                                        serializer.appendBuffer(outBuffer, sizeof(outBuffer));
                                                    }))
    {
    }
    void flush()
    {
        return_string += std::string(outBuffer, serializer.buffers().back().used);
        serializer.clearBuffers();
    }

    const std::string &returnString()
    {
        flush();
        return return_string;
    }
private:
    std::string return_string;;
};
template<typename T>
std::string serializeStruct(const T &from_type)
{
    SerializerContext<512> serializeContext;
    Token token;
    TokenParser<T,T>::serializeToken(from_type, token, serializeContext.serializer);
    return serializeContext.returnString();
}

template<typename T, typename Ret, typename Arg, size_t NAME_SIZE>
struct FunctionInfo
{
    typedef Ret(T::*Function)(const Arg &);
    typedef Ret returnType;
    const char *name;
    Function function;
};

template<typename T, typename Ret, typename Arg, size_t NAME_SIZE>
JT_CONSTEXPR FunctionInfo<T, Ret, Arg, NAME_SIZE - 1> makeFunctionInfo(const char(&name)[NAME_SIZE], Ret(T::*function)(const Arg &))
{
    return{ name, function };
}

#define JT_FUNCTION(name) JT::makeFunctionInfo(#name, &JT_CONTAINER_STRUCT_T::name)
#define JT_FUNCTION_CONTAINER(...) \
    template<typename JT_CONTAINER_STRUCT_T> \
    struct JsonToolsFunctionContainer \
    { \
        static const decltype(std::make_tuple(__VA_ARGS__)) jt_static_meta_functions_info() \
        { static auto ret = std::make_tuple(__VA_ARGS__); return ret; } \
       static const decltype(std::make_tuple()) &jt_static_meta_super_info() \
       { static auto ret = std::make_tuple(); return ret; } \
    };

#define JT_FUNCTION_CONTAINER_WITH_SUPER(super_list, ...) \
    template<typename JT_CONTAINER_STRUCT_T> \
    struct JsonToolsFunctionContainer \
    { \
       static const decltype(std::make_tuple(__VA_ARGS__)) &jt_static_meta_functions_info() \
       { static auto ret = std::make_tuple(__VA_ARGS__); return ret; } \
       static const decltype(super_list) &jt_static_meta_super_info() \
       { static auto ret = super_list; return ret; } \
    };

struct CallFunctionError
{
    std::string name;
    Error error;
    std::vector<std::string> missing_members;
    std::vector<std::string> unassigned_required_members;
};

template<size_t BUFFER_SIZE = 512>
struct CallFunctionContext
{
    CallFunctionContext() = default;
    CallFunctionContext(const char *data, size_t size)
        : parse_context(data,size)
    {}
    template<size_t SIZE>
    CallFunctionContext(const char(&data)[SIZE])
        : parse_context(data)
    {}

    ParseContext parse_context;
    std::vector<CallFunctionError> error_list;
    SerializerContext<BUFFER_SIZE> return_serializer;
    bool allow_missing = false;
};

namespace Internal {
    template<typename T, typename U, typename Ret, typename Arg, size_t NAME_SIZE>
    struct ReturnSerializer
    {
        static void serialize(T &container, FunctionInfo<U, Ret, Arg, NAME_SIZE> &functionInfo, Arg &arg, Serializer &return_json)
        {
            Token token;
            TokenParser<Ret, Ret>::serializeToken((container.*functionInfo.function)(arg), token, return_json);
        }
    };

    template<typename T, typename U, typename Arg, size_t NAME_SIZE>
    struct ReturnSerializer<T, U, void, Arg, NAME_SIZE>
    {
        static void serialize(T &container, FunctionInfo<U, void, Arg, NAME_SIZE> &functionInfo, Arg &arg, Serializer &return_json)
        {
            (container.*functionInfo.function)(arg);
        }
    };
}
template<typename T, typename U, typename Ret, typename Arg, size_t NAME_SIZE>
Error callFunctionHandler(T &container, ParseContext &context, FunctionInfo<U,Ret,Arg,NAME_SIZE> &functionInfo, Serializer &return_json)
{
    if (context.token.name.size == NAME_SIZE && memcmp(functionInfo.name, context.token.name.data, NAME_SIZE) == 0)
    {
        Arg arg;
        context.error = TokenParser<Arg,Arg>::unpackToken(arg, context);
        if (context.error != Error::NoError)
            return context.error;

       Internal::ReturnSerializer<T, U, Ret, Arg, NAME_SIZE>::serialize(container, functionInfo, arg, return_json);
       return Error::NoError;
    }
    return Error::MissingPropertyMember;
}
namespace Internal {
    template<typename T, size_t INDEX>
    struct FunctionalSuperRecursion
    {
        static Error callFunction(T &container, ParseContext &context, Serializer &return_json);
    };

    template<typename T, size_t SIZE>
    struct StartFunctionalSuperRecursion
    {
        static Error callFunction(T &container, ParseContext &context, Serializer &return_json)
        {
            return FunctionalSuperRecursion<T, SIZE - 1>::callFunction(container, context, return_json);
        }
    };
    template<typename T>
    struct StartFunctionalSuperRecursion<T, 0>
    {
        static Error callFunction(T &container, ParseContext &context, Serializer &return_json)
        {
            return Error::MissingPropertyMember;
        }
    };

    template<typename T, typename Functions, size_t INDEX>
    struct FunctionHandler
    {
        static Error call(T &container, ParseContext &context, Functions &functions, Serializer &return_json)
        {
            auto function = std::get<INDEX>(functions);
            Error error = callFunctionHandler(container, context, function, return_json);
            if (error == Error::NoError)
                return Error::NoError;
            if (context.error != Error::NoError)
                return context.error;
            return FunctionHandler<T, Functions, INDEX - 1>::call(container, context, functions, return_json);
        }
    };

    template<typename T, typename Functions>
    struct FunctionHandler<T, Functions, 0>
    {
        static Error call(T &container, ParseContext &context, Functions &functions, Serializer &return_json)
        {
            auto function = std::get<0>(functions);
            Error error = callFunctionHandler(container, context, function, return_json);
            if (error == Error::NoError)
                return Error::NoError;
            using SuperMeta = typename std::remove_reference<decltype(T::template JsonToolsFunctionContainer<T>::jt_static_meta_super_info())>::type;
            return StartFunctionalSuperRecursion<T, std::tuple_size<SuperMeta>::value>::callFunction(container, context, return_json);
        }
    };

    static void add_error(ParseContext &context, std::vector<CallFunctionError> &error_list)
    {
        if (!context.missing_members.size() && !context.unassigned_required_members.size())
            return;
        error_list.push_back(CallFunctionError());
        error_list.back().name = std::string(context.token.name.data, context.token.name.size);
        std::swap(error_list.back().missing_members, context.missing_members);
        std::swap(error_list.back().unassigned_required_members, context.unassigned_required_members);
    }
}
template<typename T>
Error callFunction(T &container, ParseContext &context, Serializer &return_json, std::vector<CallFunctionError> &error_list, bool allow_missing)
{
    context.error = context.tokenizer.nextToken(context.token);
    if (context.error != JT::Error::NoError)
        return context.error;
    if (context.token.value_type != JT::Type::ObjectStart)
        return Error::ExpectedObjectStart;
    context.error = context.tokenizer.nextToken(context.token);
    if (context.error != JT::Error::NoError)
        return context.error;
    Token token;
    token.value_type = Type::ArrayStart;
    token.value = DataRef::asDataRef("[");
    return_json.write(token);
    auto functions = T::template JsonToolsFunctionContainer<T>::jt_static_meta_functions_info();
    while (context.token.value_type != JT::Type::ObjectEnd)
    {
        Error error = Internal::FunctionHandler<T, decltype(functions), std::tuple_size<decltype(functions)>::value - 1>::call(container, context, functions,return_json);
        Internal::add_error(context, error_list);
        if (error != JT::Error::NoError && (allow_missing && context.error == Error::NoError &&  error == Error::MissingPropertyMember))
            return error;
        context.error = context.tokenizer.nextToken(context.token);
        if (context.error != JT::Error::NoError)
            return context.error;
    }
    token.value_type = Type::ArrayEnd;
    token.value = DataRef::asDataRef("]");
    return_json.write(token);
    return Error::NoError;
}

template<typename T, size_t BUFFER_SIZE>
Error  callFunction(T &container, CallFunctionContext<BUFFER_SIZE> &call_function_context)
{
    return callFunction(container, call_function_context.parse_context, call_function_context.return_serializer.serializer, call_function_context.error_list, call_function_context.allow_missing);
}
namespace Internal {
    template<typename T, size_t INDEX>
    Error FunctionalSuperRecursion<T, INDEX>::callFunction(T &container, ParseContext &context, Serializer &return_json)
    {
        using SuperMeta = typename std::remove_reference<decltype(T::template JsonToolsFunctionContainer<T>::jt_static_meta_super_info())>::type;
        using Super = typename std::tuple_element<INDEX, SuperMeta>::type::type;
        auto functions = Super::template JsonToolsFunctionContainer<Super>::jt_static_meta_functions_info();
        if (FunctionHandler<Super, decltype(functions), std::tuple_size<decltype(functions)>::value - 1>::call(container, context, functions, return_json))
            return true;

        return FunctionalSuperRecursion<T, INDEX - 1>::callFunction(container, context, return_json);
    }

    template<typename T>
    struct FunctionalSuperRecursion<T, 0>
    {
        static Error callFunction(T &container, ParseContext &context, Serializer &return_json)
        {
            using SuperMeta = typename std::remove_reference<decltype(T::template JsonToolsFunctionContainer<T>::jt_static_meta_super_info())>::type;
            using Super = typename std::tuple_element<0, SuperMeta>::type::type;
            auto functions = Super::template JsonToolsFunctionContainer<Super>::jt_static_meta_functions_info();
            return FunctionHandler<Super, decltype(functions), std::tuple_size<decltype(functions)>::value - 1>::call(container, context, functions, return_json);
        }
    };
}
} //Namespace
#endif //JSON_TOOLS_H
