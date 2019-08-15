#include "json_struct.h"

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
    JS::ParseContext context(json_string);
    JS::JsonTokens tokens;
    context.parseTo(tokens);
    JS_ASSERT(context.error == JS::Error::NoError);

    std::vector<JS::JsonMeta> metaInfo = JS::metaForTokens(tokens);
    JS_ASSERT(metaInfo.size());
    JS_ASSERT(!metaInfo[3].is_array);
    JS::Token token = tokens.data.at(metaInfo.at(3).position);
    JS_ASSERT(std::string("member_three") == std::string(token.name.data, token.name.size));
    token = tokens.data.at(metaInfo.at(3).position + metaInfo.at(3).size);
    JS_ASSERT(std::string("member_four") == std::string(token.name.data, token.name.size));
    token = tokens.data.at(metaInfo.at(6).position);
    JS_ASSERT(std::string("fourth_member") == std::string(token.name.data, token.name.size));
    JS_ASSERT((1 + metaInfo.at(1).skip + metaInfo.at(1 + metaInfo.at(1).skip).skip) == 7);
}

int main()
{
    testMetaForTokens();
}
