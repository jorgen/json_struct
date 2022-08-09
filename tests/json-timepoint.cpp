#include "catch2/catch.hpp"

#include <chrono>
#define JS_STD_TIMEPOINT 1
#include <json_struct/json_struct.h>

namespace
{

using sys_tp_t = std::chrono::system_clock::time_point;
using hpc_tp_t = std::chrono::high_resolution_clock::time_point;
using t_s = std::chrono::seconds;
using t_ms = std::chrono::milliseconds;
using t_us = std::chrono::microseconds;
using t_ns = std::chrono::nanoseconds;

#define TP_0_S  1625409496
#define TP_0_MS 1625409496000
#define TP_0_US 1625409496000000
#define TP_0_NS 1625409496000000000
#define TP_1_S  1625409497
#define TP_2_MS 1625409497002
#define TP_3_US 1625409497002003
#define TP_4_NS 1625409497002003004

#define _STR_(x) #x
#define _STR(x) _STR_(x)

const char json[] = "{"
  "\"tp_0_s\" : " _STR(TP_0_S)  ","
  "\"tp_0_ms\": " _STR(TP_0_MS) ","
  "\"tp_0_us\": " _STR(TP_0_US) ","
  "\"tp_0_ns\": " _STR(TP_0_NS) ","
  "\"tp_1_s\" : " _STR(TP_1_S)  ","
  "\"tp_2_ms\": " _STR(TP_2_MS) ","
  "\"tp_3_us\": " _STR(TP_3_US) ","
  "\"tp_4_ns\": " _STR(TP_4_NS)
"}";

#ifdef JS_STD_TIMEPOINT
struct JsonData
{
  sys_tp_t tp_0_s;
  sys_tp_t tp_0_ms;
  sys_tp_t tp_0_us;
  hpc_tp_t tp_0_ns;
  sys_tp_t tp_1_s;
  sys_tp_t tp_2_ms;
  sys_tp_t tp_3_us;
  hpc_tp_t tp_4_ns;
  JS_OBJ(tp_0_s, tp_0_ms, tp_0_us, tp_0_ns, tp_1_s, tp_2_ms, tp_3_us, tp_4_ns);
};
#endif

unsigned long long ToU64(sys_tp_t tp)
{
	return (unsigned long long)
		std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();
}

unsigned long long ToU64(hpc_tp_t tp)
{
	return (unsigned long long)
		std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
}

TEST_CASE("time_point", "json_struct")
{
  JsonData dataStruct;
  JS::ParseContext parseContext(json);
  REQUIRE(parseContext.parseTo(dataStruct) == JS::Error::NoError);

  REQUIRE(dataStruct.tp_0_s  == sys_tp_t{t_s {TP_0_S }});
  REQUIRE(dataStruct.tp_0_ms == sys_tp_t{t_ms{TP_0_MS}});
  REQUIRE(dataStruct.tp_0_us == sys_tp_t{t_us{TP_0_US}});
  REQUIRE(dataStruct.tp_0_ns == hpc_tp_t{t_ns{TP_0_NS}});
  REQUIRE(dataStruct.tp_1_s  == sys_tp_t{t_s {TP_1_S }});
  REQUIRE(dataStruct.tp_2_ms == sys_tp_t{t_ms{TP_2_MS}});
  REQUIRE(dataStruct.tp_3_us == sys_tp_t{t_us{TP_3_US}});
  REQUIRE(dataStruct.tp_4_ns == hpc_tp_t{t_ns{TP_4_NS}});

  // Check if integers with differing number of decimal digits that represent the same time are all parsed to equal timepoints
  REQUIRE(dataStruct.tp_0_s == dataStruct.tp_0_ms);
  REQUIRE(dataStruct.tp_0_s == dataStruct.tp_0_us);
  REQUIRE(ToU64(dataStruct.tp_0_us)*1000 == ToU64(dataStruct.tp_0_ns));

  std::string genjson = JS::serializeStruct(dataStruct);
  JsonData dataStruct2;
  REQUIRE(memcmp(&dataStruct2, &dataStruct, sizeof(JsonData)));
  JS::ParseContext parseContext2(genjson);
  REQUIRE(parseContext2.parseTo(dataStruct2) == JS::Error::NoError);
  REQUIRE(dataStruct2.tp_0_s  == sys_tp_t{t_s {TP_0_S }});
  REQUIRE(dataStruct2.tp_0_ms == sys_tp_t{t_ms{TP_0_MS}});
  REQUIRE(dataStruct2.tp_0_us == sys_tp_t{t_us{TP_0_US}});
  REQUIRE(dataStruct2.tp_0_ns == hpc_tp_t{t_ns{TP_0_NS}});
  REQUIRE(dataStruct2.tp_1_s  == sys_tp_t{t_s {TP_1_S }});
  REQUIRE(dataStruct2.tp_2_ms == sys_tp_t{t_ms{TP_2_MS}});
  REQUIRE(dataStruct2.tp_3_us == sys_tp_t{t_us{TP_3_US}});
  REQUIRE(dataStruct2.tp_4_ns == hpc_tp_t{t_ns{TP_4_NS}});
}
} // namespace
