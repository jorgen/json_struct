#include "json_struct.h"

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
    JS_OBJECT(JS_MEMBER(prop1),
              JS_MEMBER(prop2),
              JS_MEMBER(prop3));
};
struct ExecuteTwoData
{
    std::string first_prop;
    JS_OBJECT(JS_MEMBER(first_prop));
};
struct ExecuterTwoReturn
{
    std::string string_data;
    int value;
    std::vector<int> values;
    JS_OBJECT(JS_MEMBER(string_data),
              JS_MEMBER(value),
              JS_MEMBER(values));
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

    JS_FUNCTION_CONTAINER(JS_FUNCTION(execute_one),
                          JS_FUNCTION(execute_two));
};
void test_simple()
{
    Executor executor;
    std::string json_out;
    JS::DefaultCallFunctionContext context(json_data, json_out);
    context.callFunctions(executor);

    JS_ASSERT(context.execution_list.size() == 2);
#if JS_HAVE_CONSTEXPR
    JS_ASSERT(context.execution_list[0].unassigned_required_members.data.size() == 1);
    JS_ASSERT(context.execution_list[0].unassigned_required_members.data[0] == "prop3");
#endif
    JS_ASSERT(context.execution_list[0].missing_members.data.size() == 0);

#if JS_HAVE_CONSTEXPR
    JS_ASSERT(context.execution_list[1].missing_members.data.size() == 1);
    JS_ASSERT(context.execution_list[1].missing_members.data[0] == "second_prop");
#endif
    JS_ASSERT(context.execution_list[1].unassigned_required_members.data.size() == 0);
}

int main()
{
   test_simple();
    return 0;
}
