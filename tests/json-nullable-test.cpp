#include "catch2/catch.hpp"
#include <json_struct/json_struct.h>

namespace
{
struct SmallStructWithoutNullable
{
  int a;
  float b;

  JS_OBJECT(JS_MEMBER(a), JS_MEMBER(b));
};

struct SmallStruct
{
  int a;
  JS::Nullable<float> b = 2.2f;

  JS_OBJECT(JS_MEMBER(a), JS_MEMBER(b));
};

struct SmallStructNullableChecked
{
  int a;
  JS::NullableChecked<float> b = 2.2f;

  JS_OBJECT(JS_MEMBER(a), JS_MEMBER(b));
};

const char json[] = R"json(
{
  "a": 1,
  "b": null
}
)json";

TEST_CASE("test_nullable", "[json_struct]")
{
  {
    JS::ParseContext context(json);
    SmallStructWithoutNullable data;
    context.parseTo(data);
    REQUIRE(context.error != JS::Error::NoError);
  }
  {
    JS::ParseContext context(json);
    SmallStruct data;
    context.parseTo(data);
    REQUIRE(context.error == JS::Error::NoError);
    REQUIRE(data.a == 1);
    REQUIRE(data.b() > 2.199);
    REQUIRE(data.b() < 2.201);
  }
  {
    JS::ParseContext context(json);
    SmallStructNullableChecked data;
    context.parseTo(data);
    REQUIRE(context.error == JS::Error::NoError);
    REQUIRE(data.a == 1);
    REQUIRE(data.b.null);
    REQUIRE(data.b() > 2.199);
    REQUIRE(data.b() < 2.201);
  }
}
} // namespace
