#include <string>

#include <json_tools.h>

struct B
{
    bool first;
    short second;
    JT::OptionalChecked<std::string> third;
    
    JT_STRUCT(
        JT_MEMBER(first),
        JT_MEMBER(second),
        JT_MEMBER(third)
    );
};

struct B_Function_Object
{
    void call_function(const B &args)
    {

    }

    JT_FUNCTION_CONTAINER(
        JT_FUNCTION(call_function)
    );
};

void check_b(const std::string &json)
{
    std::string out;
    JT::DefaultCallFunctionContext cc(json.c_str(), json.size(), out);
    B_Function_Object fc;
    cc.callFunctions(fc);
}
