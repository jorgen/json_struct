#include <json_tools.h>

struct A
{
    bool a;
    int b;
    JT::Optional<std::string> c;

    JT_STRUCT(
      JT_MEMBER(a),
      JT_MEMBER(b),
      JT_MEMBER(c)
    );
};

struct A_Function_Container
{
    void some_function(const A &a)
    {
        function_called = true;
    }

    bool function_called = false;

    JT_FUNCTION_CONTAINER(
        JT_FUNCTION(some_function)
    );
};

void check_A(const std::string &json)
{
    std::string out;
    JT::DefaultCallFunctionContext cc(json.c_str(), json.size(), out);
    A_Function_Container fc;
    cc.callFunctions(fc);
}
