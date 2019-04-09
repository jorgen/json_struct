#include <json_tools.h>
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
	JT::ParseContext pc(first_child_with_data_json);
	JT::JsonTokens tokens;
	pc.parseTo(tokens);
	JT_ASSERT(pc.error == JT::Error::NoError);
	std::vector<JT::JsonMeta> meta = JT::metaForTokens(tokens);
	size_t first_child = JT::Internal::findFirstChildWithData(meta, 0);
	fprintf(stderr, "%zu\n", first_child);
        JT_ASSERT(first_child == 2);
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
	JT::ParseContext pc(first_child_with_data_json_last);
	JT::JsonTokens tokens;
	pc.parseTo(tokens);
	JT_ASSERT(pc.error == JT::Error::NoError);
	std::vector<JT::JsonMeta> meta = JT::metaForTokens(tokens);
	size_t first_child = JT::Internal::findFirstChildWithData(meta, 0);
	fprintf(stderr, "%zu\n", first_child);
        JT_ASSERT(first_child == 3);
}
int main()
{
	find_first_child_with_data();
	find_first_child_with_data_last();
	return 0;
}
