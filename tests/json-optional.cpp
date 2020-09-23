#include "json_struct.h"
#ifdef JS_STD_OPTIONAL
#include <optional>
#endif
#include "catch2/catch.hpp"

namespace
{
struct SmallStructWithoutOptional
{
  int a;
  float b = 2.2f;

  JS_OBJECT(JS_MEMBER(a), JS_MEMBER(b));
};

#ifdef JS_STD_OPTIONAL
struct SmallStructStd
{
  int a;
  std::optional<float> b = 2.2f;

  JS_OBJECT(JS_MEMBER(a), JS_MEMBER(b));
};
#endif

const char json[] = R"json(
{
	"a" : 1
}
)json";

TEST_CASE("test_optional", "[json_struct]")
{
  {
    JS::ParseContext context(json);
    context.allow_unnasigned_required_members = false;
    SmallStructWithoutOptional data;
    context.parseTo(data);
    REQUIRE(context.error != JS::Error::NoError);
  }
#ifdef JS_STD_OPTIONAL
  {
    JS::ParseContext context(json);
    context.allow_unnasigned_required_members = false;
    SmallStructStd data;
    context.parseTo(data);
    REQUIRE(context.error == JS::Error::NoError);
    REQUIRE(data.a == 1);
    REQUIRE(data.b.value() > 2.199);
    REQUIRE(data.b.value() < 2.201);
  }
#endif
}
} // namespace
