#include <json_tools.h>
#include "assert.h"

#include <stdio.h>

struct Foo
{
    JT::Internal::Tuple<int, std::string, float> data;
    JT_STRUCT(JT_MEMBER(data));
};

const char json[] = R"json(
    {
        "data" : [
            9876,
            "Tuples are cool",
            3.1415
        ]	
    }
)json";

int main()
{

    Foo out;
    out.data.get<0>() = 12345;
    out.data.get<1>() = "Hello world";
    out.data.get<2>() = 44.50;
    std::string bar = JT::serializeStruct(out);

    fprintf(stderr, "Out is:\n%s\n", bar.c_str());

    Foo in;
    JT::ParseContext context(json);
    context.parseTo(in);
    JT_ASSERT(context.error == JT::Error::NoError);
    JT_ASSERT(in.data.get<0>() == 9876);
    JT_ASSERT(std::string("Tuples are cool") == in.data.get<1>());
    JT_ASSERT(in.data.get<2>() > 3.14);
    JT_ASSERT(in.data.get<2>() < 3.15);
    return 0;
}
