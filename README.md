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

Checkout the not so complete documentation at:
http://jorgen.github.io/json_tools/

and have a look at the more complete unit tests at:
https://github.com/jorgen/json_tools/blob/master/tests
