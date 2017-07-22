#include <stdio.h>
#include <json_tools.h>
#include "assert.h"

JT_ENUM(Colors, Red , Green , Blue, Yellow4 ,
        Purple );

void check_enum_strings()
{
    auto &strings =  jt_Colors_string_struct::strings();
    JT_ASSERT(strings.size() == 5);
    std::string str(strings[0].data, strings[0].size);
    JT_ASSERT(str == "Red");
    str = std::string(strings[1].data, strings[1].size);
    JT_ASSERT(str == "Green");
    str = std::string(strings[2].data, strings[2].size);
    JT_ASSERT(str == "Blue");
    str = std::string(strings[3].data, strings[3].size);
    JT_ASSERT(str == "Yellow4");
    str = std::string(strings[4].data, strings[4].size);
    JT_ASSERT(str == "Purple");
}

struct TestEnumParser
{
    Colors colors;

    JT_STRUCT(
            JT_MEMBER(colors)
            );

};
JT_ENUM_DECLARE_STRING_PARSER(Colors);

const char json[] = R"json({
    "colors" : "Green"
})json";

void check_enum_parser()
{
    JT::ParseContext pc(json);
    TestEnumParser ep;
    pc.parseTo(ep);

    JT_ASSERT(ep.colors == Colors::Green);

    std::string jsonout = JT::serializeStruct(ep);
    JT_ASSERT(jsonout == json);
}

namespace FOO
{
namespace BAR
{
    JT_ENUM(Cars,
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

        JT_STRUCT(JT_MEMBER(car));
    };
}
}

JT_ENUM_NAMESPACE_DECLARE_STRING_PARSER(FOO::BAR, Cars);

const char car_json[] = R"json({
    "car" : "BMW"
})json";

void check_enum_parser_namespace()
{
    JT::ParseContext pc(car_json);
    One::Two::CarContainer cc;
    pc.parseTo(cc);

    JT_ASSERT(cc.car == FOO::BAR::Cars::BMW);

    std::string jsonout = JT::serializeStruct(cc);
    JT_ASSERT(jsonout == car_json);
}

int main()
{
    check_enum_strings();
    check_enum_parser();
    check_enum_parser_namespace();
    return 0;
}
