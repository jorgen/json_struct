#include "catch2/catch.hpp"
#include "json_struct.h"
#include <stdio.h>

JS_ENUM(Colors, Red, Green, Blue, Yellow4, Purple);

namespace
{
void check_enum_strings()
{
  auto &strings = js_Colors_string_struct::strings();
  REQUIRE(strings.size() == 5);
  std::string str(strings[0].data, strings[0].size);
  REQUIRE(str == "Red");
  str = std::string(strings[1].data, strings[1].size);
  REQUIRE(str == "Green");
  str = std::string(strings[2].data, strings[2].size);
  REQUIRE(str == "Blue");
  str = std::string(strings[3].data, strings[3].size);
  REQUIRE(str == "Yellow4");
  str = std::string(strings[4].data, strings[4].size);
  REQUIRE(str == "Purple");
}

struct TestEnumParser
{
  Colors colors;

  JS_OBJECT(JS_MEMBER(colors));
};
} // namespace

JS_ENUM_DECLARE_STRING_PARSER(Colors);

namespace
{
const char json[] = R"json({
    "colors" : "Green"
})json";

TEST_CASE("check_enum_parser", "[json_struct][enum]")
{
  JS::ParseContext pc(json);
  TestEnumParser ep;
  pc.parseTo(ep);

  REQUIRE(ep.colors == Colors::Green);

  std::string jsonout = JS::serializeStruct(ep);
  REQUIRE(jsonout == json);
}

namespace FOO
{
namespace BAR
{
JS_ENUM(Cars, Fiat, VW, BMW, Peugeot, Mazda);
}
} // namespace FOO
namespace One
{
namespace Two
{
struct CarContainer
{
  FOO::BAR::Cars car;

  JS_OBJECT(JS_MEMBER(car));
};
} // namespace Two
} // namespace One

} // namespace

JS_ENUM_NAMESPACE_DECLARE_STRING_PARSER(FOO::BAR, Cars);

namespace
{
const char car_json[] = R"json({
    "car" : "BMW"
})json";

TEST_CASE("check_enum_parser_namespace", "[json_struct][enum]")
{
  JS::ParseContext pc(car_json);
  One::Two::CarContainer cc;
  pc.parseTo(cc);

  REQUIRE(cc.car == FOO::BAR::Cars::BMW);

  std::string jsonout = JS::serializeStruct(cc);
  REQUIRE(jsonout == car_json);
}

} // namespace
