#include "assert.h"
#include "generated.json.h"
#include <json_struct/json_struct.h>
#include <chrono>

#include "catch2/catch.hpp"

#include "include/rapidjson/document.h"
#include "include/simdjson/simdjson.h"
#include "include/nlohmann/json.hpp"

static constexpr std::string_view message = R"(
{
   "fixed_object": {
      "int_array": [0, 1, 2, 3, 4, 5, 6],
      "float_array": [0.1, 0.2, 0.3, 0.4, 0.5, 0.6],
      "double_array": [3288398.238, 233e22, 289e-1, 0.928759872, 0.22222848, 0.1, 0.2, 0.3, 0.4]
   },
   "fixed_name_object": {
      "name0": "James",
      "name1": "Abraham",
      "name2": "Susan",
      "name3": "Frank",
      "name4": "Alicia"
   },
   "another_object": {
      "string": "here is some text",
      "another_string": "Hello World",
      "boolean": false,
      "nested_object": {
         "v3s": [[0.12345, 0.23456, 0.001345],
                  [0.3894675, 97.39827, 297.92387],
                  [18.18, 87.289, 2988.298]],
         "id": "298728949872"
      }
   },
   "string_array": ["Cat", "Dog", "Elephant", "Tiger"],
   "string": "Hello world",
   "number": 3.14,
   "boolean": true,
   "another_bool": false
}
)";

#include <chrono>
#include <iostream>
#include <unordered_map>

struct fixed_object_t
{
   std::vector<int> int_array;
   std::vector<float> float_array;
   std::vector<double> double_array;
};

struct fixed_name_object_t
{
   std::string name0{};
   std::string name1{};
   std::string name2{};
   std::string name3{};
   std::string name4{};
};

struct nested_object_t
{
   std::vector<std::array<double, 3>> v3s{};
   std::string id{};
};

struct another_object_t
{
   std::string string{};
   std::string another_string{};
   bool boolean{};
   nested_object_t nested_object{};
};

struct obj_t
{
   fixed_object_t fixed_object{};
   fixed_name_object_t fixed_name_object{};
   another_object_t another_object{};
   std::vector<std::string> string_array{};
   std::string string{};
   double number{};
   bool boolean{};
   bool another_bool{};
};

#define JS_STL_ARRAY 1
#include "json_struct/json_struct.h"

JS_OBJ_EXT(fixed_object_t, int_array, float_array, double_array);
JS_OBJ_EXT(fixed_name_object_t, name0, name1, name2, name3, name4);
JS_OBJ_EXT(nested_object_t, v3s, id);
JS_OBJ_EXT(another_object_t, string, another_string, boolean, nested_object);
JS_OBJ_EXT(obj_t, fixed_object, fixed_name_object, another_object, string_array, string, number, boolean, another_bool);

#ifdef NDEBUG
static constexpr size_t iterations = 1'000'000;
#else
static constexpr size_t iterations = 100'000;
#endif

TEST_CASE("GlazeTestjson_struct", "[performance]")
{
   std::string buffer{ message };

   obj_t obj;

   auto t0 = std::chrono::steady_clock::now();

   for (size_t i = 0; i < iterations; ++i) {
     JS::ParseContext context(buffer);
     context.track_member_assignement_state = false;
     context.parseTo(obj);
     if (context.error != JS::Error::NoError)
     {
      std::cout << "json_struct error: " << context.makeErrorString() << '\n';
     }
     buffer.clear();
     buffer = JS::serializeStruct(obj);
   }
   auto t1 = std::chrono::steady_clock::now();

   auto runtime = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() * 1e-6;

   std::cout << "json_struct roundtrip runtime: " << runtime << '\n';
   
   // write performance
   t0 = std::chrono::steady_clock::now();
   
   for (size_t i = 0; i < iterations; ++i) {
      JS::ParseContext context(buffer);
      context.parseTo(obj);
   }
   
   t1 = std::chrono::steady_clock::now();
   
   runtime = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() * 1e-6;
   
   auto mbytes_per_sec = iterations * buffer.size() / (runtime * 1048576);
   std::cout << "json_struct write_json size: " << buffer.size() << " bytes\n";
   std::cout << "json_struct write_json: " << runtime << " s, " << mbytes_per_sec
             << " MB/s"
             << "\n";
   
   // read performance
   
   t0 = std::chrono::steady_clock::now();
   
   for (size_t i = 0; i < iterations; ++i) {
      buffer.clear();
      buffer = JS::serializeStruct(obj);
   }
   
   t1 = std::chrono::steady_clock::now();
   
   runtime = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() * 1e-6;
   
   mbytes_per_sec = iterations * buffer.size() / (runtime * 1048576);
   std::cout << "json_struct read_json size: " << buffer.size() << " bytes\n";
   std::cout << "json_struct read_json: " << runtime << " s, " << mbytes_per_sec
             << " MB/s"
             << "\n";
   
   std::cout << '\n';
}
