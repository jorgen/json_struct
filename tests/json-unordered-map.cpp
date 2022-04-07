#include "json_struct.h"
#include "catch2/catch.hpp"

namespace
{

const char json[] = R"json({
  "unordered_map": {
    "foo": [ 1.0 ],
    "bar": [ 2.0 ]
  }
})json";

struct JsonData
{
#ifdef JS_STD_UNORDERED_MAP
  std::unordered_map<std::string, std::vector<double>> unordered_map;
#else
  JS::JsonObject unordered_map;
#endif
  JS_OBJ(unordered_map);
};

TEST_CASE("unordered_map_complex_value", "json_struct")
{
  JsonData dataStruct;
  JS::ParseContext parseContext(json);
  REQUIRE(parseContext.parseTo(dataStruct) == JS::Error::NoError);

  std::vector<double> one;
  one.push_back(1.0);

  std::vector<double> two;
  two.push_back(2.0);
  REQUIRE(dataStruct.unordered_map["foo"] == one);
  REQUIRE(dataStruct.unordered_map["bar"] == two);

  std::string genjson = JS::serializeStruct(dataStruct);

  JsonData dataStruct2;
  REQUIRE(dataStruct2.unordered_map != dataStruct.unordered_map);
  JS::ParseContext parseContext2(genjson);
  REQUIRE(parseContext2.parseTo(dataStruct2) == JS::Error::NoError);

  REQUIRE(dataStruct2.unordered_map == dataStruct.unordered_map);
}
} // namespace
