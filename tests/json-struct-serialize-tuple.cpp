#include "catch2/catch.hpp"
#include <json_struct/json_struct.h>

#include <stdio.h>

namespace json_struct_serialize_tuple
{

struct Foo
{
  JS::Tuple<int, std::string, float> data;
  JS_OBJECT(JS_MEMBER(data));
};

const char json[] = R"json(
{
  "data": [
    9876,
    "Tuples are cool",
    3.1415
  ]
}
)json";

TEST_CASE("serialize_tuple", "[json_struct][tuple]")
{

  Foo out;
  out.data.get<0>() = 12345;
  out.data.get<1>() = "Hello world";
  out.data.get<2>() = 44.50;
  std::string bar = JS::serializeStruct(out);

  Foo in;
  JS::ParseContext context(json);
  context.parseTo(in);
  REQUIRE(context.error == JS::Error::NoError);
  REQUIRE(in.data.get<0>() == 9876);
  REQUIRE(std::string("Tuples are cool") == in.data.get<1>());
  REQUIRE(in.data.get<2>() > 3.14);
  REQUIRE(in.data.get<2>() < 3.15);
}

} // namespace json_struct_serialize_tuple
