#include "json_struct.h"
#include <optional>
#include "assert.h"

struct SmallStructWithoutOptional
{
	int a;
        float b = 2.2;

	JS_OBJECT(
		JS_MEMBER(a),
		JS_MEMBER(b)
	);
};

struct SmallStructStd
{
	int a;
        std::optional<float> b = 2.2;

	JS_OBJECT(
		JS_MEMBER(a),
		JS_MEMBER(b)
	);
};


const char json[] = R"json(
{
	"a" : 1
}
)json";

int main()
{
    {
        JS::ParseContext context(json);
        context.allow_unnasigned_required_members = false;
        SmallStructWithoutOptional data;
        context.parseTo(data);
        JS_ASSERT(context.error != JS::Error::NoError);
    }
    {
        JS::ParseContext context(json);
        context.allow_unnasigned_required_members = false;
        SmallStructStd data;
        context.parseTo(data);
        JS_ASSERT(context.error == JS::Error::NoError);
        JS_ASSERT(data.a == 1);
        JS_ASSERT(data.b.value() > 2.199 && data.b.value() < 2.201);
        return 0;
    }
}
