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

    JT_STRUCT(
       JT_MEMBER(One),
       JT_MEMBER(Two),
       JT_MEMBER(Three));
}
```

Populating the struct would look like this:

```c++
JT::ParseContext context(json_data);
JsonObject obj;
context.parseTo(obj);
```

The JT_STRUCT macro adds a static metaobject to the struct/class, and the
JT_MEMBER adds member pointer with a string representation for the name. When
the JT::ParseContext tries to parse a type it will look for a template
specialisation of the type:

```c++
namespace JT {
    template<typename T, typename specifier>
    struct TypeHandler;
}
```

There is already specified a template specialisation for types that contain the
meta data object supplied by the JT_STRUCT macro. There is also supplied
template specialisations for a number of types such as:

* `std::string`
* `double`
* `float`
* `int`
* `unsigned int`
* `int65_t`
* `uint64_t`
* `int16_t`
* `uint16_t`
* `std::unique_ptr`
* `bool`
* `std::vector`

There are also specialisation for some json_tools helper types:
* `JT::Optional`
* `JT::OptionalChecked`
* `JT::SilentString`
* `JT::SilentVector`
* `JT::SilentUniquePtr`

If there is a new type that should be populated with some special semantic from
json then a new specialisation of the TypeHandler struct can be added.

Here is the specialisation for populating and extracting to a std::string:

```c++
namespace JT {
template<>
struct TypeHandler<std::string, std::string>
{
	static inline JT::Error unpackToken(std::string &to_type, JT::ParseContext &context)
	{
		to_type = std::string(context.token.value.data, context.token.value.size);
		return JT::Error::NoError;
	}

	static inline void serializeToken(const std::string &str, JT::Token &token,
            JT::Serializer &serializer)
	{
		token.value_type = JT::Type::String;
		token.value.data = &str[0];
		token.value.size = str.size();
		serializer.write(token);
	}
};
}
```

TypeHandler has two functions:

    - unpackToken and serializeToken. unpackToken takes data from the json
      stream and puts into the supplied reference.

    - serializeToken takes data from the supplied const reference and puts into
      the serializer stream.

JT::ParseContext is a helper struct that holds the json tokenizer called
JT::Tokenizer. JT::Tokenizer takes json data and allows you to itterate over the
json key value pairs. The toplevel object or array is just a value, and elements
of an array has also just value data.

The definition of JT::Token is as follows.

```c++
namespace JT {
struct Token
{
    JT::Type name_type;
    JT::Type value_type;
    JT::DataRef name;
    JT::DataRef value;
};
}
```

The enum JT::Type is defined as follows:

```c++
namespace JT {
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
}
```

and JT::DataRef is defined as follows:
```c++
namespace JT
{
struct DataRef
{
    ... //constructors

    const char *data;
    size_t size;
};
}
```

This shows that a token just points to position in the input stream, and gives
the "json type" of these positions.

