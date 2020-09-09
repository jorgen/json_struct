//!!!!!
//THIS IS A VERY ADVANCED SAMPLE
//PLEASE LOOK AT THE OTHER SAMPLES FIRST
#include <string>
#include <json_struct.h>

const char json[] = R"json(
{
    "function_a" : "Some text",
    "function_b" : {
        "paramA" : 123.4,
        "paramB" : "some string parameter"
    },
    "function_c" : {
        "this_function" : 3,
        "can_fail_at_runtime" : true
    },
    "function_d" : 567
}
)json";

struct FunctionBArguments
{
    float paramA;
    std::string paramB;
    JS_OBJECT(JS_MEMBER(paramA),
              JS_MEMBER(paramB));
};

struct FunctionBReturn
{
    float functionBReturnA;
    std::string functionBReturnB;
    double functionBReturnC[3];

    JS_OBJECT(JS_MEMBER(functionBReturnA),
              JS_MEMBER(functionBReturnB),
              JS_MEMBER(functionBReturnC));
};

struct FunctionCArguments
{
    int this_function;
    bool can_fail_at_runtime;

    JS_OBJECT(JS_MEMBER(this_function),
              JS_MEMBER(can_fail_at_runtime));
};

struct FunctionCReturn
{
    int this_return;
    int type_will_not;
    int be_serialized;
    int on_failure;
    JS_OBJECT(JS_MEMBER(this_return),
              JS_MEMBER(type_will_not),
              JS_MEMBER(be_serialized),
              JS_MEMBER(on_failure));
};

struct JsonFunctions
{
    void function_a(const std::string &str)
    {
        fprintf(stderr, "Function a was called with %s\n", str.c_str());
    }

    FunctionBReturn function_b(const FunctionBArguments &arg)
    {
        fprintf(stderr, "Function b was called with %f - %s\n", arg.paramA, arg.paramB.c_str());
        FunctionBReturn ret;
        ret.functionBReturnA = arg.paramA;
        ret.functionBReturnB = "This is the return object";
        ret.functionBReturnC[0] = 3.3;
        ret.functionBReturnC[1] = 4.4;
        ret.functionBReturnC[2] = 5.5;
        return ret;
    }

    FunctionCReturn function_c(const FunctionCArguments &arg, JS::CallFunctionErrorContext &context)
    {
        fprintf(stderr, "Function c was called and its going to fail miserably\n");
        FunctionCReturn ret;
        context.setError(JS::Error::UserDefinedErrors, "Making the error"
                         " context have failure marked so that it will not"
                         " serialize the return type");
        return ret;
    }

    bool function_d(int arg)
    {
        fprintf(stderr, "Function d shows that just simple types can be used - %d\n", arg);
        return arg;
    }
    JS_FUNCTION_CONTAINER(JS_FUNCTION(function_a),
                          JS_FUNCTION(function_b),
                          JS_FUNCTION(function_c),
                          JS_FUNCTION(function_d));
};

int main()
{
    JsonFunctions functionObject;
    std::string output;
    JS::DefaultCallFunctionContext callFunctionContext(json, output);
    callFunctionContext.stop_execute_on_fail = false;
    if (callFunctionContext.callFunctions(functionObject) != JS::Error::NoError)
    {
        std::string errorStr = callFunctionContext.parse_context.makeErrorString();
        fprintf(stderr, "Error parsing struct %s\n", errorStr.c_str());
    }

    for (auto &executed : callFunctionContext.execution_list)
    {
        std::string executionStateJson = JS::serializeStruct(executed);
        fprintf(stderr, "###\n%s\n", executionStateJson.c_str());
    }

    fprintf(stderr, "This is the result:\n%s\n", output.c_str());
    return 0;
}


