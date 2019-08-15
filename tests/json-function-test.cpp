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

#include "json_struct.h"

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

    JS_OBJECT(JS_MEMBER(number),
              JS_MEMBER(valid));
};
struct CallFunction
{
    virtual void execute_one(const SimpleData &data)
    {
        fprintf(stderr, "execute one executed %f : %d\n", data.number, data.valid);
        called_one = true;
    }

    int execute_two(const double &data, JS::CallFunctionContext &context)
    {
        fprintf(stderr, "execute two executed %f\n", data);
        called_two = true;
        return 2;
    }

    void execute_three(const std::vector<double> &data, JS::CallFunctionContext &context)
    {
        fprintf(stderr, "execute three\n");
        for (auto x : data)
            fprintf(stderr, "\t%f\n", x);
        called_three = true;
    }
    JS_FUNCTION_CONTAINER(JS_FUNCTION(execute_one),
                          JS_FUNCTION(execute_two),
                          JS_FUNCTION(execute_three));

    bool called_one = false;
    bool called_two = false;
    bool called_three = false;
};

void simpleTest()
{
    CallFunction cont;
    std::string json_out;
    JS::DefaultCallFunctionContext context(json, sizeof(json), json_out);
    context.callFunctions(cont);

    JS_ASSERT(cont.called_one);
    JS_ASSERT(cont.called_two);
    JS_ASSERT(cont.called_three);

    if (context.parse_context.error != JS::Error::NoError)
        fprintf(stderr, "callFunction failed \n%s\n", context.parse_context.tokenizer.makeErrorString().c_str());
    JS_ASSERT(context.parse_context.error == JS::Error::NoError);
}

struct CallFunctionSuperSuper
{
    void execute_one(const SimpleData &data)
    {
        fprintf(stderr, "execute one executed %f : %d\n", data.number, data.valid);
        called_one = true;
    }

    bool called_one = false;

    JS_FUNCTION_CONTAINER(JS_FUNCTION(execute_one));
};

struct CallFunctionSuper
{
    void execute_two(const double &data)
    {
        fprintf(stderr, "execute two executed %f\n", data);
        called_two = true;
    }

    bool called_two = false;
    JS_FUNCTION_CONTAINER(JS_FUNCTION(execute_two));
};

struct ExecuteThreeReturn
{
    bool valid = true;
    int error_code = 0;
    std::string data = "hello world";
    JS_OBJECT(JS_MEMBER(valid),
              JS_MEMBER(error_code),
              JS_MEMBER(data));
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
    JS_FUNCTION_CONTAINER_WITH_SUPER(JS_SUPER_CLASSES(JS_SUPER_CLASS(CallFunctionSuperSuper),
                                                      JS_SUPER_CLASS(CallFunctionSuper)),
                                     JS_FUNCTION(execute_three));
};

void inheritanceTest()
{
    CallFunctionSub cont;
    std::string json_out;
    JS::DefaultCallFunctionContext context(json, json_out);
    context.callFunctions(cont);

    JS_ASSERT(cont.called_one);
    JS_ASSERT(cont.called_two);
    JS_ASSERT(cont.called_three);

    if (context.parse_context.error != JS::Error::NoError)
        fprintf(stderr, "callFunction failed \n%s\n", context.parse_context.tokenizer.makeErrorString().c_str());
    JS_ASSERT(context.parse_context.error == JS::Error::NoError);
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
    JS::DefaultCallFunctionContext context(json,json_out);
    context.callFunctions(cont);

    JS_ASSERT(cont.override_called);
    JS_ASSERT(!cont.called_one);
    JS_ASSERT(cont.called_two);
    JS_ASSERT(cont.called_three);

    fprintf(stderr, "return string\n%s\n", json_out.c_str());
    if (context.parse_context.error != JS::Error::NoError)
        fprintf(stderr, "callFunction failed \n%s\n", context.parse_context.tokenizer.makeErrorString().c_str());
    JS_ASSERT(context.parse_context.error == JS::Error::NoError);
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
    JS_OBJECT(JS_MEMBER(number));
};

struct SuperParamTwo
{
    bool valid;
    JS_OBJECT(JS_MEMBER(valid));
};

struct SuperParamTwoOne : public SuperParamTwo
{
    std::string more_data;
    JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(JS_SUPER_CLASS(SuperParamTwo)), JS_MEMBER(more_data));
};
struct Param : public SuperParamOne, public SuperParamTwoOne
{
    std::string super_data;
    JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(JS_SUPER_CLASS(SuperParamOne),
                                          JS_SUPER_CLASS(SuperParamTwoOne)),
                         JS_MEMBER(super_data));
};

struct SuperParamCallable
{
    void execute_one(const Param &param)
    {
        execute_one_executed = true;
    }
    bool execute_one_executed = false;

    JS_FUNCTION_CONTAINER(JS_FUNCTION(execute_one));
};

void super_class_param_test()
{
    SuperParamCallable cont;
    std::string json_out;
    JS::DefaultCallFunctionContext context(json_two, json_out);
    context.callFunctions(cont);

    JS_ASSERT(cont.execute_one_executed);
    if (context.parse_context.error != JS::Error::NoError)
        fprintf(stderr, "callFunction failed \n%s\n", context.parse_context.tokenizer.makeErrorString().c_str());
    JS_ASSERT(context.parse_context.error == JS::Error::NoError);
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

    void call_void_context(JS::CallFunctionContext &context)
    {
        executed_2 = true;
    }

    int call_int_void() {
        executed_3 = true;
        return 3;
    }

    int call_int_void_context(JS::CallFunctionContext &context)
    {
        executed_4 = true;
        return 7;
    }

    void call_void_error(JS::CallFunctionErrorContext &error)
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
    JS_FUNCTION_CONTAINER(
        JS_FUNCTION(call_void),
        JS_FUNCTION(call_void_context),
        JS_FUNCTION(call_int_void),
        JS_FUNCTION(call_int_void_context),
        JS_FUNCTION(call_void_with_value),
        JS_FUNCTION(call_void_error));
};

void call_void_test()
{
    CallVoidStruct voidStruct;
    std::string json_out;
    JS::DefaultCallFunctionContext context(call_void_json, json_out);
    context.callFunctions(voidStruct);

    if (context.error_context.getLatestError() != JS::Error::NoError)
        fprintf(stderr, "error %s\n", context.parse_context.tokenizer.makeErrorString().c_str());
    JS_ASSERT(context.error_context.getLatestError() == JS::Error::NoError);
    JS_ASSERT(voidStruct.executed_1);
    JS_ASSERT(voidStruct.executed_2);
    JS_ASSERT(voidStruct.executed_3);
    JS_ASSERT(voidStruct.executed_4);
    JS_ASSERT(voidStruct.executed_5);
    JS_ASSERT(voidStruct.executed_6);
    JS_ASSERT(context.execution_list.size() == 6);
}

const char call_error_check_json[] = R"json(
{
    "call_void" : [],
    "call_with_int" : 5,
    "call_another_void" : {},
    "call_with_object" : { "x" : 9 }
}
)json";

struct CallErrorCheckArg
{
	int x;
	JS_OBJECT(JS_MEMBER(x));
};
struct CallErrorCheck
{
	void call_void()
	{
		executed1 = true;
	}

	void call_with_int(int x, JS::CallFunctionErrorContext &context)
	{
		executed2 = true;
		context.setError(JS::Error::UserDefinedErrors, "CallWithIntCustomError problem with number");
	}

	void call_another_void()
	{
		executed3 = true;
	}

	std::string call_with_object(const CallErrorCheckArg &arg, JS::CallFunctionErrorContext &context)
	{
		executed4 = true;
		context.setError(JS::Error::UserDefinedErrors, "This functions should not serialize the string");
		return std::string("THIS SHOULD NOT BE SERIALIZED");
	}

	JS_FUNCTION_CONTAINER(
		JS_FUNCTION(call_void),
		JS_FUNCTION(call_with_int),
		JS_FUNCTION(call_another_void),
		JS_FUNCTION(call_with_object));

	bool executed1 = false;
	bool executed2 = false;
	bool executed3 = false;
	bool executed4 = false;
};

void call_error_check()
{
	CallErrorCheck errorCheck;
    std::string json_out;
    JS::DefaultCallFunctionContext context(call_error_check_json, json_out);
	context.stop_execute_on_fail = false;
    JS::Error error = context.callFunctions(errorCheck);

	JS_ASSERT(error == JS::Error::NoError);
	fprintf(stderr, "json out %s\n", json_out.c_str());
	JS_ASSERT(errorCheck.executed1);
	JS_ASSERT(errorCheck.executed2);
	JS_ASSERT(errorCheck.executed3);
	JS_ASSERT(errorCheck.executed4);
	JS_ASSERT(json_out.size() == 3);
}

const char json_alias[] = R"json(
{
    "execute_one" : 4,
    "execute_two" : 5,
    "execute_three": 6
}
)json";

struct JsonAlias
{
    bool executeOne = false;
    bool executeTwo = false;
    bool executeThree = false;

    void execute_one(int x)
    {
        JS_ASSERT(x == 4);
        executeOne = true;
    }

    void execute_two_primary(int x)
    {
        JS_ASSERT(x == 5);
        executeTwo = true;
    }

    void execute_three(int x)
    {
        JS_ASSERT(x == 6);
        executeThree = true;
    }

    JS_FUNCTION_CONTAINER(
        JS_FUNCTION(execute_one),
        JS_FUNCTION_ALIASES(execute_two_primary, "execute_two"),
        JS_FUNCTION(execute_three));
};

void call_json_alias()
{
    JsonAlias cont;
    std::string json_out;
    JS::DefaultCallFunctionContext context(json_alias, sizeof(json_alias), json_out);
    context.callFunctions(cont);

    JS_ASSERT(cont.executeOne);
    JS_ASSERT(cont.executeTwo);
    JS_ASSERT(cont.executeThree);

    if (context.parse_context.error != JS::Error::NoError)
        fprintf(stderr, "call_json_alias failed \n%s\n", context.parse_context.tokenizer.makeErrorString().c_str());
    JS_ASSERT(context.parse_context.error == JS::Error::NoError);
}

const char json_wrong_arg_type[] = R"json(
{
    "execute_one" : { "some_function_object": 1 },
    "execute_two" : { "more_members": false },
    "execute_three": { "last_member" : 44.50 } 
}
)json";

struct JsonWrongArgTypeExecThree
{
	double last_member;
	JS_OBJECT(JS_MEMBER(last_member));
};

struct JsonWrongArgType
{
    bool executeOne = false;
    bool executeTwo = false;
    bool executeThree = false;

    void execute_one(const std::string &foo)
    {
        executeOne = true;
    }

    void execute_two(const std::string &bar)
    {
        executeTwo = true;
    }

    void execute_three(const JsonWrongArgTypeExecThree &arg)
    {
		JS_ASSERT(arg.last_member == 44.50);
        executeThree = true;
    }

    JS_FUNCTION_CONTAINER(
        JS_FUNCTION(execute_one),
        JS_FUNCTION(execute_two),
        JS_FUNCTION(execute_three));
};

void call_json_wrong_arg_type()
{
    JsonWrongArgType cont;
    std::string json_out;
    JS::DefaultCallFunctionContext context(json_wrong_arg_type, sizeof(json_wrong_arg_type), json_out);
    context.callFunctions(cont);

    JS_ASSERT(cont.executeOne);
    JS_ASSERT(cont.executeTwo);
    JS_ASSERT(cont.executeThree);

    if (context.parse_context.error != JS::Error::NoError)
        fprintf(stderr, "call_json_alias failed \n%s\n", context.parse_context.tokenizer.makeErrorString().c_str());
    JS_ASSERT(context.parse_context.error == JS::Error::NoError);
}

int main()
{
    simpleTest();
    inheritanceTest();
    virtualFunctionTest();
    super_class_param_test();
    call_void_test();
	call_error_check();
    call_json_alias();
	call_json_wrong_arg_type();
}

