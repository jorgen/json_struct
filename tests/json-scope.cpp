#include <stdio.h>
#include "assert.h"
#include "json_tools.h"

const char json[] = R"json({
    "func1" : {
        "arg1" : "hello",
        "arg2" : "world"
    },
    "func2" : {
        "one" : [ 1, 2, 3, 4 ],
        "two" : true
    },
    "func3" : {
        "first" :  {
            "advanced" : true"
        },
        "second" : false
    }
})json";

struct Func1Arg
{
    int arg1;
    std::string arg2;
    JT_STRUCT(
        JT_MEMBER(arg1),
        JT_MEMBER(arg2)
    );
};

struct Func2Arg
{
    int one[4];
    bool two;
    JT_STRUCT(
        JT_MEMBER(one),
        JT_MEMBER(two)
    );
};

struct Func3Arg
{
    int first;
    double second;
    JT_STRUCT(
        JT_MEMBER(first),
        JT_MEMBER(second)
    );
};

struct FunctionCont
{
    void func1(const Func1Arg &)
    {
        func1_called = true;
    }

    void func2(const Func2Arg &arg)
    {
        func2_called = true;
    }

    void func3(const Func3Arg &arg)
    {
        func3_called = true;
    }

    bool func1_called = false;
    bool func2_called = false;
    bool func3_called = false;
    JT_FUNCTION_CONTAINER(
        JT_FUNCTION(func1),
        JT_FUNCTION(func2),
        JT_FUNCTION(func3)
    );
};

int main()
{
    FunctionCont cont;
    std::string json_out;
    JT::DefaultCallFunctionContext context(json, json_out);
    JT::Error error = context.callFunctions(cont);
    
}