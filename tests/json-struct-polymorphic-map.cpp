/*
 * Copyright © 2021 Jorgen Lind
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#define JS_EXPERIMENTAL_MAP
#include "json_struct.h"
#include "catch2/catch.hpp"


namespace
{
static const char json[] = R"json(
{
  "Field1" : 4,
  "Field2" : true,
  "ComplexFields" : { "Hello" : 4, "World" : 2 },
  "Field3" : "432",
  "ComplexFields2" : { "SimpleMember" : true, "ArrayOfValues" : [4, 3, 5, 7], "SubObject" : { "SimpleMember" : false, "MoreValues": [ "Hello", "World"] } },
  "Field4" : 567
}
)json";

struct ComplexFields_t
{
  int Hello;
  int World;
  JS_OBJ(Hello, World);
};

struct SubObject_t
{
  bool SimpleMember;
  std::vector<std::string> MoreValues;
  JS_OBJ(SimpleMember, MoreValues);
};
struct ComplexFields2_t
{
  ComplexFields2_t(bool init)
    : SimpleMember(init)
  {
  }
  bool SimpleMember;
  std::vector<int> ArrayOfValues;
  SubObject_t SubObject;
  JS_OBJ(SimpleMember, ArrayOfValues, SubObject);
};

struct Root_t
{
  Root_t()
    : ComplexFields2(true)
  {
  }
  int Field1;
  bool Field2;
  std::string Field3;
  int Field4;
  ComplexFields_t ComplexFields;
  ComplexFields2_t ComplexFields2;
  JS_OBJ(Field1, Field2, Field3, Field4, ComplexFields, ComplexFields2);
};

TEST_CASE("polymorphic_map_basic", "json_struct")
{
  JS::Error error;
  JS::Map map = JS::createMap(json, error);
  REQUIRE(error == JS::Error::NoError);

  REQUIRE(map.castTo<int>("Field1", error) == 4);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(map.castTo<bool>("Field2", error) == true);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(map.castTo<std::string>("Field3", error) == "432");
  REQUIRE(error == JS::Error::NoError);
  ComplexFields_t complexFields1 = map.castTo<ComplexFields_t>("ComplexFields", error);
  REQUIRE(error == JS::Error::NoError);
  ComplexFields_t complexFields1_2;
  error = map.castToType("ComplexFields", complexFields1_2);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(complexFields1.Hello == complexFields1_2.Hello);
  REQUIRE(complexFields1.World == complexFields1_2.World);
  REQUIRE(complexFields1.Hello == 4);
  REQUIRE(complexFields1.World == 2);

  ComplexFields2_t complexFields2(false);
  error = map.castToType("ComplexFields2", complexFields2);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(complexFields2.ArrayOfValues[1] == 3);
  REQUIRE(complexFields2.SubObject.MoreValues[1] == "World");

  REQUIRE(map.castTo<int>("Field4", error) == 567);
  REQUIRE(error == JS::Error::NoError);

  Root_t root = map.castTo<Root_t>(error);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(root.Field1 == 4);
  REQUIRE(root.ComplexFields2.SubObject.MoreValues[0] == "Hello");

  auto it = map.find("Field2");
  REQUIRE(it != map.end());
  REQUIRE(it->value_type == JS::Type::Bool);
}

} // namespace
