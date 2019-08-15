#include <stdio.h>
#include "json_struct.h"
#include "assert.h"

JS_ENUM(Colors, Red , Green , Blue, Yellow4 ,
        Purple );

void check_enum_strings()
{
    auto &strings =  js_Colors_string_struct::strings();
    JS_ASSERT(strings.size() == 5);
    std::string str(strings[0].data, strings[0].size);
    JS_ASSERT(str == "Red");
    str = std::string(strings[1].data, strings[1].size);
    JS_ASSERT(str == "Green");
    str = std::string(strings[2].data, strings[2].size);
    JS_ASSERT(str == "Blue");
    str = std::string(strings[3].data, strings[3].size);
    JS_ASSERT(str == "Yellow4");
    str = std::string(strings[4].data, strings[4].size);
    JS_ASSERT(str == "Purple");
}

struct TestEnumParser
{
    Colors colors;

    JS_OBJECT(
            JS_MEMBER(colors)
            );

};
JS_ENUM_DECLARE_STRING_PARSER(Colors);

const char json[] = R"json({
    "colors" : "Green"
})json";

void check_enum_parser()
{
    JS::ParseContext pc(json);
    TestEnumParser ep;
    pc.parseTo(ep);

    JS_ASSERT(ep.colors == Colors::Green);

    std::string jsonout = JS::serializeStruct(ep);
    JS_ASSERT(jsonout == json);
}

namespace FOO
{
namespace BAR
{
    JS_ENUM(Cars,
            Fiat,
            VW,
            BMW,
            Peugeot,
            Mazda);
}
}
namespace One
{
namespace Two
{
    struct CarContainer
    {
        FOO::BAR::Cars car;

        JS_OBJECT(JS_MEMBER(car));
    };
}
}

JS_ENUM_NAMESPACE_DECLARE_STRING_PARSER(FOO::BAR, Cars);

const char car_json[] = R"json({
    "car" : "BMW"
})json";

void check_enum_parser_namespace()
{
    JS::ParseContext pc(car_json);
    One::Two::CarContainer cc;
    pc.parseTo(cc);

    JS_ASSERT(cc.car == FOO::BAR::Cars::BMW);

    std::string jsonout = JS::serializeStruct(cc);
    JS_ASSERT(jsonout == car_json);
}

int main()
{
    check_enum_strings();
    check_enum_parser();
    check_enum_parser_namespace();
    return 0;
}
