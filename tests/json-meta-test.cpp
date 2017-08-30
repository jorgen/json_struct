#pragma once

#include <json_tools.h>
#include "assert.h"
const char remove_empty_siblings[] = R"json([ [], [
[],
[],
{
	"this has a member" : true
},
[]
]])json";

void remove_empty_siblings_test()
{
	JT::ParseContext pc(remove_empty_siblings);
	JT::JsonTokens tokens;
	pc.parseTo(tokens);
	JT_ASSERT(pc.error == JT::Error::NoError);
	std::vector<JT::JsonMeta> meta = JT::metaForTokens(tokens);
	int first_child = JT::Internal::findFirstChildWithData(meta, 0);
	fprintf(stderr, "%d\n", first_child);
}
int main()
{
	remove_empty_siblings_test();
	return 0;
}