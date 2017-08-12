#include <string>
#include <json_tools.h>

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

    JT_STRUCT(JT_MEMBER(key),
              JT_MEMBER(number),
              JT_MEMBER(boolean));
};

int main()
{
    JT::ParseContext parseContext(json);
    JsonData dataStruct;
    parseContext.parseTo(dataStruct);

    fprintf(stdout, "Key is: %s, number is %d bool is %d\n",
            dataStruct.key.c_str(),
            dataStruct.number,
            dataStruct.boolean);

    return 0;
}

