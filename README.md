# **Structurize your JSON!!!**

[![Build Status](https://travis-ci.org/oysteinmyrmo/json_struct.svg?branch=master)](https://travis-ci.org/oysteinmyrmo/json_struct)

json_struct is a single header only library parse JSON to C++ structs/classes
and serializing structs/classes to JSON.

It is intented to be used by copying the json_struct.h file from the include
folder into the include path for the project. It is only the json_struct.h file
that is needed to serialize and deserialize json from structures.

It is dependent on some C++11 features and is tested on newer versions of gcc
and clang. It is also tested on VS 2013 and VS 2015.

json_struct can parse json and automatically populate structures with content
by adding some metadata to the C++ structs.

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

    JS_OBJECT(JS_MEMBER(One),
              JS_MEMBER(Two),
              JS_MEMBER(Three));
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
JS_OBJECT_EXTERNAL(JsonObject,
                   JS_MEMBER(One),
                   JS_MEMBER(Two),
                   JS_MEMBER(Three));
```

Populating the struct would look like this:

```c++
JS::ParseContext context(json_data);
JsonObject obj;
context.parseTo(obj);
```

Serializing the struct to json could be done like this:

```c++
std::string pretty_json = JS::serializeStruct(obj);
// or
std::string compact_json = JS::serializeStruct(obj, JS::SerializerOptions(JS::SerializerOptions::Compact));
```


The JS_OBJECT macro adds a static metaobject to the struct/class, and the
JS_MEMBER adds member pointer with a string representation for the name. So for
objects the meta information is generated with JS_OBJECT, but there might be
types like that doesn't fit the meta information interface, like ie. a matrix
or a vector class. Then its possible to define how these specific classes are
serialized and deserialized with the TypeHandler interface.

When the JS::ParseContext tries to parse a type it will look for a template
specialisation of the type:

```c++
namespace JS {
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
namespace JS {
template<>
struct TypeHandler<uint32_t>
{
public:
    static inline Error to(uint32_t &to_type, ParseContext &context)
    {
        char *pointer;
        unsigned long value = strtoul(context.token.value.data, &pointer, 10);
        to_type = static_cast<unsigned int>(value);
        if (context.token.value.data == pointer)
            return Error::FailedToParseInt;
        return Error::NoError;
    }

    static void from(const uint32_t &from_type, Token &token, Serializer &serializer)
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

For more information checkout the examples at:
https://github.com/jorgen/json_struct/tree/master/examples

and have a look at the more complete unit tests at:
https://github.com/jorgen/json_struct/tree/master/tests
