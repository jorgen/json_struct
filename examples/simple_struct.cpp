#include <string>
#include <json_struct.h>

const char json[] = R"json(
{
    "key" : "value",
    "number" : 100,
    "boolean" : true
}
)json";

struct JsonData
{
    std::string key;
    int number;
    bool boolean;

    JS_OBJECT(JS_MEMBER(key),
              JS_MEMBER(number),
              JS_MEMBER(boolean));
};

int main()
{
    JS::ParseContext parseContext(json);
    JsonData dataStruct;
    parseContext.parseTo(dataStruct);

    fprintf(stdout, "Key is: %s, number is %d bool is %d\n",
            dataStruct.key.c_str(),
            dataStruct.number,
            dataStruct.boolean);

    return 0;
}

