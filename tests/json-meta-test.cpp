#include "json_struct.h"
#include "assert.h"

const char first_child_with_data_json[] = R"json([ [], [],  [
[],
[],
{
	"this has a member" : true
},
[]
], [], []])json";

void find_first_child_with_data()
{
	JS::ParseContext pc(first_child_with_data_json);
	JS::JsonTokens tokens;
	pc.parseTo(tokens);
	JS_ASSERT(pc.error == JS::Error::NoError);
	std::vector<JS::JsonMeta> meta = JS::metaForTokens(tokens);
	size_t first_child = JS::Internal::findFirstChildWithData(meta, 0);
	fprintf(stderr, "%zu\n", first_child);
        JS_ASSERT(first_child == 2);
}
const char first_child_with_data_json_last[] = R"json([ [], [], [],  [
[],
[],
{
	"this has a member" : true
},
[]
]])json";

void find_first_child_with_data_last()
{
	JS::ParseContext pc(first_child_with_data_json_last);
	JS::JsonTokens tokens;
	pc.parseTo(tokens);
	JS_ASSERT(pc.error == JS::Error::NoError);
	std::vector<JS::JsonMeta> meta = JS::metaForTokens(tokens);
	size_t first_child = JS::Internal::findFirstChildWithData(meta, 0);
	fprintf(stderr, "%zu\n", first_child);
        JS_ASSERT(first_child == 3);
}
int main()
{
	find_first_child_with_data();
	find_first_child_with_data_last();
	return 0;
}
