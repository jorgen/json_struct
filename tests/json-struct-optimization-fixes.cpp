/*
 * Regression tests for correctness bugs found during the optimization/bug hunt.
 */

#include <json_struct/json_struct.h>

#include "catch2/catch_all.hpp"

#include <cmath>
#include <limits>

namespace
{

struct IntBox
{
  int x = -1;
  JS_OBJ(x);
};

TEST_CASE("integer_requires_full_token_consumption", "[json_struct][number]")
{
  // A well-formed integer parses and consumes the whole token.
  {
    JS::ParseContext c("{\"x\": 100}");
    IntBox b;
    REQUIRE(c.parseTo(b) == JS::Error::NoError);
    REQUIRE(b.x == 100);
  }
  {
    JS::ParseContext c("{\"x\": 1e2}");
    IntBox b;
    REQUIRE(c.parseTo(b) == JS::Error::NoError);
    REQUIRE(b.x == 100);
  }
  // Malformed number-class tokens (still a single Number token to the tokenizer)
  // must now be rejected instead of silently truncated.
  {
    JS::ParseContext c("{\"x\": 1e2e3}");
    IntBox b;
    REQUIRE(c.parseTo(b) != JS::Error::NoError);
  }
  {
    JS::ParseContext c("{\"x\": 12-3}");
    IntBox b;
    REQUIRE(c.parseTo(b) != JS::Error::NoError);
  }
}

struct DoubleBox
{
  double d = 0.0;
  JS_OBJ(d);
};

struct FloatBox
{
  float f = 0.0f;
  JS_OBJ(f);
};

TEST_CASE("nonfinite_serializes_as_valid_json", "[json_struct][number]")
{
  {
    DoubleBox b;
    b.d = std::numeric_limits<double>::quiet_NaN();
    std::string out = JS::serializeStruct(b);
    REQUIRE(out.find("nan") == std::string::npos);
    REQUIRE(out.find("null") != std::string::npos);
    // The emitted document must be valid JSON (previously it was the bare token
    // "nan"). A plain double field cannot absorb null, so tokenize-verify that the
    // output is structurally well-formed instead of round-tripping into a double.
    JS::Tokenizer tok;
    tok.addData(out.c_str(), out.size());
    JS::Token token;
    JS::Error e = JS::Error::NoError;
    do
    {
      e = tok.nextToken(token);
    } while (e == JS::Error::NoError && token.value_type != JS::Type::ObjectEnd);
    REQUIRE(e == JS::Error::NoError);
    REQUIRE(token.value_type == JS::Type::ObjectEnd);
  }
  {
    DoubleBox b;
    b.d = std::numeric_limits<double>::infinity();
    std::string out = JS::serializeStruct(b);
    REQUIRE(out.find("inf") == std::string::npos);
    REQUIRE(out.find("null") != std::string::npos);
  }
  {
    FloatBox b;
    b.f = -std::numeric_limits<float>::infinity();
    std::string out = JS::serializeStruct(b);
    REQUIRE(out.find("inf") == std::string::npos);
    REQUIRE(out.find("null") != std::string::npos);
  }
}

TEST_CASE("newline_delimiter_simd_gate", "[tokenizer]")
{
  // Newline-delimited array (no commas), long enough that the >=16/32-byte SIMD
  // whitespace-skip in findTokenEnd triggers. Before the gate fix the SIMD path
  // consumed the newline delimiter and findTokenEnd returned InvalidToken.
  static const char data[] = "[\n"
                             "  \"aaaaaaaaaaaaaaaa\"\n"
                             "  \"bbbbbbbbbbbbbbbb\"\n"
                             "  \"cccccccccccccccc\"\n"
                             "  \"dddddddddddddddd\"\n"
                             "]";
  JS::Tokenizer tok;
  tok.allowNewLineAsTokenDelimiter(true);
  tok.addData(data, sizeof(data) - 1);

  JS::Token token;
  JS::Error e = JS::Error::NoError;
  int strings = 0;
  do
  {
    e = tok.nextToken(token);
    if (e == JS::Error::NoError && token.value_type == JS::Type::String)
      strings++;
  } while (e == JS::Error::NoError && token.value_type != JS::Type::ArrayEnd);

  REQUIRE(e == JS::Error::NoError);
  REQUIRE(strings == 4);
}

struct MultiInt
{
  int a = 0;
  int b = 0;
  int c = 0;
  JS_OBJ(a, b, c);
};

struct NestChild
{
  int a = 0;
  JS_OBJ(a);
};

struct NestParent
{
  int collide = 0;
  NestChild child;
  JS_OBJ(collide, child);
};

TEST_CASE("nested_unknown_field_strict_stops_scanning", "[json_struct][error]")
{
  // The child object contains a field ("collide") that is unknown to NestChild
  // but collides with a NestParent member name. In strict mode this previously
  // made the parent resume scanning its own members against tokens inside the
  // half-parsed child, silently assigning collide=99 and returning NoError.
  static const char json_data[] = R"json({"child":{"a":1,"collide":99},"collide":7})json";

  JS::ParseContext context(json_data);
  context.allow_missing_members = false;
  NestParent p;
  auto error = context.parseTo(p);

  // Must now report an error instead of silently corrupting data.
  REQUIRE(error != JS::Error::NoError);
  REQUIRE(p.collide != 99); // the stray child field must not leak into the parent
}

TEST_CASE("nested_unknown_field_allowed_by_default", "[json_struct][error]")
{
  // With the default allow_missing_members=true, an unknown nested field is
  // simply skipped and parsing succeeds.
  static const char json_data[] = R"json({"child":{"a":1,"extra":99},"collide":7})json";

  JS::ParseContext context(json_data);
  NestParent p;
  auto error = context.parseTo(p);

  REQUIRE(error == JS::Error::NoError);
  REQUIRE(p.collide == 7);
  REQUIRE(p.child.a == 1);
}

TEST_CASE("pretty_skip_delimiter_omits_comma", "[serializer]")
{
  MultiInt m;
  m.a = 1;
  m.b = 2;
  m.c = 3;
  JS::SerializerOptions opts(JS::SerializerOptions::Pretty);
  opts.skipDelimiter(true);
  std::string out = JS::serializeStruct(m, opts);
  // The Pretty shortcut path must honor skipDelimiter: no separating commas.
  REQUIRE(out.find(',') == std::string::npos);
}

} // namespace
