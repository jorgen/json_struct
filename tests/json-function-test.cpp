/*
 * Copyright © 2016 Jorgen Lind
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include "json_tools.h"

#include "assert.h"

const char json[] =
"{"
"    \"execute_one\" : {\n"
"        \"number\" : 45,\n"
"        \"valid\" : \"false\"\n"
"    },"
"    \"execute_two\" : 99,\n"
"    \"execute_three\" : [\n"
"        4,\n"
"        6,\n"
"        8\n"
"    ]\n"
"}\n";

struct SimpleData
{
    float number;
    bool valid;

    JT_STRUCT(JT_MEMBER(number),
              JT_MEMBER(valid));
};
struct CallFunction
{
    virtual void execute_one(const SimpleData &data)
    {
        fprintf(stderr, "execute one executed %f : %d\n", data.number, data.valid);
        called_one = true;
    }

    int execute_two(const double &data, JT::CallFunctionContext &context)
    {
        fprintf(stderr, "execute two executed %f\n", data);
        called_two = true;
        return 2;
    }

    void execute_three(const std::vector<double> &data, JT::CallFunctionContext &context)
    {
        fprintf(stderr, "execute three\n");
        for (auto x : data)
            fprintf(stderr, "\t%f\n", x);
        called_three = true;
    }
    JT_FUNCTION_CONTAINER(JT_FUNCTION(execute_one),
                          JT_FUNCTION(execute_two),
                          JT_FUNCTION(execute_three));

    bool called_one = false;
    bool called_two = false;
    bool called_three = false;
};

void simpleTest()
{
    CallFunction cont;
    std::string json_out;
    JT::DefaultCallFunctionContext context(json, sizeof(json), json_out);
    context.callFunctions(cont);

    JT_ASSERT(cont.called_one);
    JT_ASSERT(cont.called_two);
    JT_ASSERT(cont.called_three);

    if (context.parse_context.error != JT::Error::NoError)
        fprintf(stderr, "callFunction failed \n%s\n", context.parse_context.tokenizer.makeErrorString().c_str());
    JT_ASSERT(context.parse_context.error == JT::Error::NoError);
}

struct CallFunctionSuperSuper
{
    void execute_one(const SimpleData &data)
    {
        fprintf(stderr, "execute one executed %f : %d\n", data.number, data.valid);
        called_one = true;
    }

    bool called_one = false;

    JT_FUNCTION_CONTAINER(JT_FUNCTION(execute_one))
};

struct CallFunctionSuper
{
    void execute_two(const double &data)
    {
        fprintf(stderr, "execute two executed %f\n", data);
        called_two = true;
    }

    bool called_two = false;
    JT_FUNCTION_CONTAINER(JT_FUNCTION(execute_two));
};

struct ExecuteThreeReturn
{
    bool valid = true;
    int error_code = 0;
    std::string data = "hello world";
    JT_STRUCT(JT_MEMBER(valid),
              JT_MEMBER(error_code),
              JT_MEMBER(data));
};
struct CallFunctionSub : public CallFunctionSuperSuper, public CallFunctionSuper
{
    ExecuteThreeReturn execute_three(const std::vector<double> &data)
    {
        fprintf(stderr, "execute three\n");
        for (auto x : data)
            fprintf(stderr, "\t%f\n", x);
        called_three = true;
        return ExecuteThreeReturn();
    }

    bool called_three = false;
    JT_FUNCTION_CONTAINER_WITH_SUPER(JT_SUPER_CLASSES(JT_SUPER_CLASS(CallFunctionSuperSuper),
                                                      JT_SUPER_CLASS(CallFunctionSuper)),
                                     JT_FUNCTION(execute_three));
};

void inheritanceTest()
{
    CallFunctionSub cont;
    std::string json_out;
    JT::DefaultCallFunctionContext context(json, json_out);
    context.callFunctions(cont);

    JT_ASSERT(cont.called_one);
    JT_ASSERT(cont.called_two);
    JT_ASSERT(cont.called_three);

    if (context.parse_context.error != JT::Error::NoError)
        fprintf(stderr, "callFunction failed \n%s\n", context.parse_context.tokenizer.makeErrorString().c_str());
    JT_ASSERT(context.parse_context.error == JT::Error::NoError);
}

struct CallFunctionVirtualOverload : public CallFunction
{
    virtual void execute_one(const SimpleData &data) override
    {
        override_called = true;
    }
    bool override_called = false;
};

void virtualFunctionTest()
{
    std::string json_out;
    json_out.reserve(512);
    CallFunctionVirtualOverload cont;
    JT::DefaultCallFunctionContext context(json,json_out);
    context.callFunctions(cont);

    JT_ASSERT(cont.override_called);
    JT_ASSERT(!cont.called_one);
    JT_ASSERT(cont.called_two);
    JT_ASSERT(cont.called_three);

    fprintf(stderr, "return string\n%s\n", json_out.c_str());
    if (context.parse_context.error != JT::Error::NoError)
        fprintf(stderr, "callFunction failed \n%s\n", context.parse_context.tokenizer.makeErrorString().c_str());
    JT_ASSERT(context.parse_context.error == JT::Error::NoError);
}

const char json_two[] = R"json(
{
    "execute_one" : {
        "number" : 45,
        "valid" : false,
        "more_data" : "string data",
        "super_data" : "hello"
    }
}
)json";

struct SuperParamOne
{
    int number;
    JT_STRUCT(JT_MEMBER(number));
};

struct SuperParamTwo
{
    bool valid;
    JT_STRUCT(JT_MEMBER(valid));
};

struct SuperParamTwoOne : public SuperParamTwo
{
    std::string more_data;
    JT_STRUCT_WITH_SUPER(JT_SUPER_CLASSES(JT_SUPER_CLASS(SuperParamTwo)), JT_MEMBER(more_data));
};
struct Param : public SuperParamOne, public SuperParamTwoOne
{
    std::string super_data;
    JT_STRUCT_WITH_SUPER(JT_SUPER_CLASSES(JT_SUPER_CLASS(SuperParamOne),
                                          JT_SUPER_CLASS(SuperParamTwoOne)),
                         JT_MEMBER(super_data));
};

struct SuperParamCallable
{
    void execute_one(const Param &param)
    {
        execute_one_executed = true;
    }
    bool execute_one_executed = false;

    JT_FUNCTION_CONTAINER(JT_FUNCTION(execute_one));
};

void super_class_param_test()
{
    SuperParamCallable cont;
    std::string json_out;
    JT::DefaultCallFunctionContext context(json_two, json_out);
    context.callFunctions(cont);

    JT_ASSERT(cont.execute_one_executed);
    if (context.parse_context.error != JT::Error::NoError)
        fprintf(stderr, "callFunction failed \n%s\n", context.parse_context.tokenizer.makeErrorString().c_str());
    JT_ASSERT(context.parse_context.error == JT::Error::NoError);
}

const char call_void_json[] = R"json(
{
    "call_void" : [],
    "call_void_context" : null,
    "call_int_void" : {},
    "call_int_void_context" : {},
    "call_void_with_value" : 4,
	"call_void_error" : {}
}
)json";

struct CallVoidStruct
{
    void call_void()
    {
        executed_1 = true;
    }

    void call_void_context(JT::CallFunctionContext &context)
    {
        executed_2 = true;
    }

    int call_int_void() {
        executed_3 = true;
        return 3;
    }

    int call_int_void_context(JT::CallFunctionContext &context)
    {
        executed_4 = true;
        return 7;
    }

    void call_void_error(JT::CallFunctionErrorContext &error)
    {
        executed_6 = true;
    }

    void call_void_with_value()
    {
        executed_5 = true;
    }

    bool executed_1 = false;
    bool executed_2 = false;
    bool executed_3 = false;
    bool executed_4 = false;
    bool executed_5 = false;
    bool executed_6 = false;
    JT_FUNCTION_CONTAINER(
        JT_FUNCTION(call_void),
        JT_FUNCTION(call_void_context),
        JT_FUNCTION(call_int_void),
        JT_FUNCTION(call_int_void_context),
        JT_FUNCTION(call_void_with_value),
        JT_FUNCTION(call_void_error));
};

void call_void_test()
{
    CallVoidStruct voidStruct;
    std::string json_out;
    JT::DefaultCallFunctionContext context(call_void_json, json_out);
    context.callFunctions(voidStruct);

    if (context.error_context.getLatestError() != JT::Error::NoError)
        fprintf(stderr, "error %s\n", context.parse_context.tokenizer.makeErrorString().c_str());
    JT_ASSERT(context.error_context.getLatestError() == JT::Error::NoError);
    JT_ASSERT(voidStruct.executed_1);
    JT_ASSERT(voidStruct.executed_2);
    JT_ASSERT(voidStruct.executed_3);
    JT_ASSERT(voidStruct.executed_4);
    JT_ASSERT(!voidStruct.executed_5);
    JT_ASSERT(context.execution_list.size() == 6);
    JT_ASSERT(context.execution_list[4].error == JT::Error::IlligalVoidFunctionArgument);
    JT_ASSERT(voidStruct.executed_6);
}

int main()
{
    simpleTest();
    inheritanceTest();
    virtualFunctionTest();
    super_class_param_test();
    call_void_test();
}

