#include <json_tools.h>

#include "assert.h"

const char json_data[] = R"json(
{
    "execute_one" : {
        "prop1" : 4,
        "prop2" : "Property 2"
    },
    "execute_two" : {
        "first_prop" : "some string",
        "second_prop" : 8
    }
}
)json";

struct ExecuteOneData
{
    int prop1;
    std::string prop2;
    std::string prop3;
    JT_STRUCT(JT_MEMBER(prop1),
              JT_MEMBER(prop2),
              JT_MEMBER(prop3));
};
struct ExecuteTwoData
{
    std::string first_prop;
    JT_STRUCT(JT_MEMBER(first_prop));
};
struct ExecuterTwoReturn
{
    std::string string_data;
    int value;
    std::vector<int> values;
    JT_STRUCT(JT_MEMBER(string_data),
              JT_MEMBER(value),
              JT_MEMBER(values));
};
struct Executor
{
    void execute_one(const ExecuteOneData &data)
    {
        execute_one_called = true;
    }
    ExecuterTwoReturn execute_two(const ExecuteTwoData &data)
    {
        execute_two_called = true;
        ExecuterTwoReturn ret;
        ret.string_data = "Ret data";
        ret.value = 999;
        ret.values = { 3, 4, 5, 7, 8 };
        return ret;
    }
    bool execute_one_called = false;
    bool execute_two_called = false;

    JT_FUNCTION_CONTAINER(JT_FUNCTION(execute_one),
                          JT_FUNCTION(execute_two));
};
void test_simple()
{
    Executor executor;
    JT::DefaultCallFunctionContext<> context(json_data, std::string());
    JT::Error called = context.callFunctions(executor);

    JT_ASSERT(context.error_list.size() == 2);
    JT_ASSERT(context.error_list[0].unassigned_required_members.size() == 1);
    JT_ASSERT(context.error_list[0].unassigned_required_members[0] == "prop3");
    JT_ASSERT(context.error_list[0].missing_members.size() == 0);

    JT_ASSERT(context.error_list[1].missing_members.size() == 1);
    JT_ASSERT(context.error_list[1].missing_members[0] == "second_prop");
    JT_ASSERT(context.error_list[1].unassigned_required_members.size() == 0);
}

int main()
{
    test_simple();
    return 0;
}