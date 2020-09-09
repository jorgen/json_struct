#include "json_struct.h"
#include "assert.h"

struct SmallStructWithoutNullable
{
	int a;
        float b;

	JS_OBJECT(
		JS_MEMBER(a),
		JS_MEMBER(b)
	);
};

struct SmallStruct
{
	int a;
        JS::Nullable<float> b = 2.2f;

	JS_OBJECT(
		JS_MEMBER(a),
		JS_MEMBER(b)
	);
};

struct SmallStructNullableChecked
{
	int a;
        JS::NullableChecked<float> b = 2.2f;

	JS_OBJECT(
		JS_MEMBER(a),
		JS_MEMBER(b)
	);
};

const char json[] = R"json(
{
	"a" : 1,
	"b" : null
}
)json";

int main()
{
    {
        JS::ParseContext context(json);
        SmallStructWithoutNullable data;
        context.parseTo(data);
        JS_ASSERT(context.error != JS::Error::NoError);
    }
    {
        JS::ParseContext context(json);
        SmallStruct data;
        context.parseTo(data);
        JS_ASSERT(context.error == JS::Error::NoError);
        JS_ASSERT(data.a == 1);
        JS_ASSERT(data.b() > 2.199 && data.b() < 2.201);
        return 0;
    }
    {
        JS::ParseContext context(json);
        SmallStructNullableChecked data;
        context.parseTo(data);
        JS_ASSERT(context.error == JS::Error::NoError);
        JS_ASSERT(data.a == 1);
        JS_ASSERT(data.b.null);
        JS_ASSERT(data.b() > 2.199 && data.b() < 2.201);
        return 0;
    }
}
