#include "json_struct.h"
#include "assert.h"

#include <stdio.h>

struct Foo
{
    JS::Tuple<int, std::string, float> data;
    JS_OBJECT(JS_MEMBER(data));
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
    std::string bar = JS::serializeStruct(out);

    fprintf(stderr, "Out is:\n%s\n", bar.c_str());

    Foo in;
    JS::ParseContext context(json);
    context.parseTo(in);
    JS_ASSERT(context.error == JS::Error::NoError);
    JS_ASSERT(in.data.get<0>() == 9876);
    JS_ASSERT(std::string("Tuples are cool") == in.data.get<1>());
    JS_ASSERT(in.data.get<2>() > 3.14);
    JS_ASSERT(in.data.get<2>() < 3.15);
    return 0;
}
