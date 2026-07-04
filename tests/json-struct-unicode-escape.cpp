/*
 * Tests for \uXXXX escape decoding into UTF-8 std::string, covering the
 * 3-byte BMP lead byte and UTF-16 surrogate-pair (astral) handling.
 */

#include <json_struct/json_struct.h>

#include "catch2/catch_all.hpp"

namespace
{

struct UStruct
{
  std::string euro;   // U+20AC, 3-byte
  std::string cjk;    // U+4E2D, 3-byte
  std::string oslash; // U+00F8, 2-byte
  std::string grin;   // U+1F600, 4-byte (surrogate pair)
  std::string ascii;  // U+0041, 1-byte
  JS_OBJ(euro, cjk, oslash, grin, ascii);
};

// Uses \uXXXX escapes (raw string keeps the backslashes literal) so the parser
// exercises the escape-decoding path rather than raw UTF-8 passthrough.
static const char unicode_json[] = R"json({
  "euro": "\u20AC",
  "cjk": "\u4e2d",
  "oslash": "\u00f8",
  "grin": "\uD83D\uDE00",
  "ascii": "\u0041"
})json";

TEST_CASE("unicode_escape_decoding", "[json_struct][utf-8]")
{
  JS::ParseContext context(unicode_json);
  UStruct s;
  auto error = context.parseTo(s);

  REQUIRE(error == JS::Error::NoError);
  // 3-byte lead byte must be 0xE2 (1110xxxx), not 0xD2 -- the previous 0xd0 typo.
  REQUIRE(s.euro == std::string("\xE2\x82\xAC"));
  REQUIRE(s.cjk == std::string("\xE4\xB8\xAD"));
  REQUIRE(s.oslash == std::string("\xC3\xB8"));
  // Surrogate pair U+D83D/U+DE00 -> U+1F600 -> 4-byte F0 9F 98 80.
  REQUIRE(s.grin == std::string("\xF0\x9F\x98\x80"));
  REQUIRE(s.grin.size() == 4);
  REQUIRE(s.ascii == std::string("A"));
}

TEST_CASE("unicode_escape_roundtrip", "[json_struct][utf-8]")
{
  // Parse then serialize then parse again: the decoded UTF-8 must survive.
  JS::ParseContext context(unicode_json);
  UStruct s;
  REQUIRE(context.parseTo(s) == JS::Error::NoError);

  std::string serialized = JS::serializeStruct(s);

  JS::ParseContext context2(serialized);
  UStruct s2;
  REQUIRE(context2.parseTo(s2) == JS::Error::NoError);
  REQUIRE(s2.euro == s.euro);
  REQUIRE(s2.grin == s.grin);
  REQUIRE(s2.cjk == s.cjk);
}

} // namespace
