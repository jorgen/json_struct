#include <json_tools.h>

#include "assert.h"

const char json_string[] = R"json(
    [
        {
            "member_one" : "hello world",
            "member_two" : [ 5, 6, 7, 8, 9],
            "member_three" : {
                "member_three_sub_one" : 5,
                "member_three_sub_two" : null,
                "member_three_sub_three" : [ "hello", "world", "bye"]
            },
            "member_four" : true
        },
        {
            "first_member" : false,
            "second_member" : "sky is blue",
            "third_member" : "grass is green",
            "fourth_member" : [10, 20, 30, 40, 50]
        },
        {
            "last_obj" : true
        }
    ]
)json";

void testMetaForTokens()
{
    JT::ParseContext context(json_string);
    JT::JsonTokens tokens;
    context.parseTo(tokens);
    JT_ASSERT(context.error == JT::Error::NoError);

    std::vector<JT::JsonMeta> metaInfo = JT::metaForTokens(tokens);
    JT_ASSERT(metaInfo.size());
    JT_ASSERT(!metaInfo[3].is_array);
    JT::Token token = tokens.at(metaInfo.at(3).position);
    JT_ASSERT(std::string("member_three") == std::string(token.name.data, token.name.size));
    token = tokens.at(metaInfo.at(3).position + metaInfo.at(3).size);
    JT_ASSERT(std::string("member_four") == std::string(token.name.data, token.name.size));
    token = tokens.at(metaInfo.at(6).position);
    JT_ASSERT(std::string("fourth_member") == std::string(token.name.data, token.name.size));
    JT_ASSERT((1 + metaInfo.at(1).skip + metaInfo.at(1 + metaInfo.at(1).skip).skip) == 7);
}

int main()
{
    testMetaForTokens();
}