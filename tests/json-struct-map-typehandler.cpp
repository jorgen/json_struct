#define JS_STL_UNORDERED_SET
#include <json_struct/json_struct.h>
#include "catch2/catch.hpp"

namespace JS
{
template<typename T>
struct TypeHandler<std::unordered_map<int, T>>
{
  static inline Error to(std::unordered_map<int,T> &to_type, ParseContext &context)
  {
    if (context.token.value_type != Type::ObjectStart)
    {
      return JS::Error::ExpectedObjectStart;
    }

    Error error = context.nextToken();
    if (error != JS::Error::NoError)
      return error;

    const char *pointer = nullptr;
    int key = 0;

    while (context.token.value_type != Type::ObjectEnd)
    {
      auto parse_error = Internal::ft::integer::to_integer(context.token.name.data, context.token.name.size, key, pointer);
      if (parse_error != Internal::ft::parse_string_error::ok || context.token.name.data == pointer)
        return Error::FailedToParseInt;

      T value;
      error = TypeHandler<T>::to(value, context);
      to_type[std::move(key)] = std::move(value);
      if (error != JS::Error::NoError)
        return error;
      error = context.nextToken();
    }

    return error;
  }
  static inline void from(const std::unordered_map<int, T> &from_type, Token &token, Serializer &serializer)
  {
    token.value_type = Type::ObjectStart;
    token.value = DataRef("{");
    serializer.write(token);
    char buf[40];
    int digits_truncated;
    for (auto it = from_type.begin(); it != from_type.end(); ++it)
    {
      int size = Internal::ft::integer::to_buffer(it->first, buf, sizeof(buf), &digits_truncated);
      if (size <= 0 || digits_truncated)
        return;
      token.name = DataRef(buf, size);
      token.name_type = Type::String;
      TypeHandler<T>::from(it->second, token, serializer);
    }
    token.name.size = 0;
    token.name.data = "";
    token.name_type = Type::String;
    token.value_type = Type::ObjectEnd;
    token.value = DataRef("}");
    serializer.write(token);
  }
};
}

namespace
{
const char json[] = R"json(
{
    "15": {
        "88": {
            "List": {
                "-1": {
                    "1": {
                        "List2": {
                            "1": "1",
                            "2": "2",
                            "3": "3"
                        },
                        "Val": "private",
                        "Val2": "private"
                    }
                },
                "3366": {
                    "1": {
                        "List2": {
                            "1": "1",
                            "2": "2",
                            "3": "3"
                        },
                        "Val": "private",
                        "Val2": "private"
                    }
                }
            },
            "Val": "private"
        },
        "89": {
            "List": {
                "-1": {
                    "1": {
                        "List2": null,
                        "Val": "private",
                        "Val2": "private"
                    }
                }
            },
            "Val": "private"
        }
    }
}
)json";

struct One
{
  JS::Nullable<std::unordered_map<int, std::string>> List2;
  std::string Val;
  std::string Val2;
  JS_OBJ(List2, Val, Val2);
};

struct ListVal
{
  std::unordered_map<int, std::unordered_map<int, One>> List;
  std::string Val;
  JS_OBJ(List, Val);
};

TEST_CASE("map_typehandler", "json_struct")
{
  std::unordered_map<int, std::unordered_map<int, ListVal>> obj;
  JS::ParseContext pc(json);
  auto error = pc.parseTo(obj);
  if (error != JS::Error::NoError)
  {
    auto errorStr = pc.makeErrorString();
    fprintf(stderr, "%s\n", errorStr.c_str());
  }
  REQUIRE(error == JS::Error::NoError);

}
}
