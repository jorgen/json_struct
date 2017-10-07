#include <json_tools.h>
#include "assert.h"

struct SmallStruct
{
	int a;
	float b;

	JT_STRUCT(
		JT_MEMBER(a),
		JT_MEMBER(b)
	);
};

const char json[] = R"json(
{
	"a" : 1,
	"b" : 2.2
}
)json";

int main()
{
	JT::ParseContext context(json);
    SmallStruct data;
    context.parseTo(data);
	JT_ASSERT(data.a == 1);
	JT_ASSERT(data.b > 2.199 && data.b < 2.201);
	return 0;
}
