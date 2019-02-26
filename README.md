[![Build Status](https://travis-ci.org/jorgen/json_tools.svg?branch=master)](https://travis-ci.org/jorgen/json_tools)

json_tools is a single header only library meant to tokenize, parse and
validate json.

It is dependent on some C++11 features and is tested on newer versions of gcc
and clang. It is also tested on VS 2013 and VS 2015.

json_tools can parse json and automatically populate structures with content.

```c++
{
    "One" : 1,
    "Two" : two,
    "Three" : 3.333
}
```

can be parsed into a structure defined like this:

```c++
struct JsonObject
{
    int One;
    std::string Two;
    double Three;

    JT_STRUCT(JT_MEMBER(One),
              JT_MEMBER(Two),
              JT_MEMBER(Three));
};
```

or

```c++
struct JsonObject
{
    int One;
    std::string Two;
    double Three;
};
JT_STRUCT_EXTERNAL(JT_MEMBER(One),
                   JT_MEMBER(Two),
                   JT_MEMBER(Three));
```

Populating the struct would look like this:

```c++
JT::ParseContext context(json_data);
JsonObject obj;
context.parseTo(obj);
```

Serializing the struct to json could be done like this:

```c++
std::string pretty_json = JT::serializeStruct(obj);
// or
std::string compact_json = JT::serializeStruct(obj, JT::SerializerOptions(JT::SerializerOptions::Compact));
```


The JT_STRUCT macro adds a static metaobject to the struct/class, and the
JT_MEMBER adds member pointer with a string representation for the name. So for
objects the meta information is generated with JT_STRUCT, but there might be
types like that doesn't fit the meta information interface, like ie. a matrix
or a vector class. Then its possible to define how these specific classes are
serialized and deserialized with the TypeHandler interface.

When the JT::ParseContext tries to parse a type it will look for a template
specialisation of the type:

```c++
namespace JT {
    template<typename T>
    struct TypeHandler;
}
```

There are a number of predefined template specialisations for types such as:

* `std::string`
* `double`
* `float`
* `uint8_t`
* `int16_t`
* `uint16_t`
* `int32_t`
* `uint32_t`
* `int64_t`
* `uint64_t`
* `std::unique_ptr`
* `bool`
* `std::vector`
* `[T]`

Its not often necessary, but when you need to define your own serialization and
deserialization it's done like this:

```c++
namespace JT {
template<>
struct TypeHandler<uint32_t>
{
public:
    static inline Error unpackToken(uint32_t &to_type, ParseContext &context)
    {
        char *pointer;
        unsigned long value = strtoul(context.token.value.data, &pointer, 10);
        to_type = static_cast<unsigned int>(value);
        if (context.token.value.data == pointer)
            return Error::FailedToParseInt;
        return Error::NoError;
    }

    static void serializeToken(const uint32_t &from_type, Token &token, Serializer &serializer)
    {
        std::string buf = std::to_string(from_type);
        token.value_type = Type::Number;
        token.value.data = buf.data();
        token.value.size = buf.size();
        serializer.write(token);
    }
};
}
```

This gives you complete control of serialization deserialization of a type and it can unfold to a json object or array if needed.

For more information checkout the not so complete documentation at:
http://jorgen.github.io/json_tools/

and have a look at the more complete unit tests at:
https://github.com/jorgen/json_tools/blob/master/tests
