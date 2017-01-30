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

    int execute_two(const double &data)
    {
        fprintf(stderr, "execute two executed %f\n", data);
        called_two = true;
        return 2;
    }

    int execute_three(const std::vector<double> &data)
    {
        fprintf(stderr, "execute three\n");
        for (auto x : data)
            fprintf(stderr, "\t%f\n", x);
        called_three = true;
        return 3;
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
    JT::CallFunctionContext<512> context(json);
    JT::callFunction(cont, context);

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

struct CallFunctionSuper : public CallFunctionSuperSuper
{
    void execute_two(const double &data)
    {
        fprintf(stderr, "execute two executed %f\n", data);
        called_two = true;
    }

    bool called_two = false;
    JT_FUNCTION_CONTAINER_WITH_SUPER(JT_SUPER_CLASSES(JT_SUPER_CLASS(CallFunctionSuperSuper)),
                                     JT_FUNCTION(execute_two));
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
struct CallFunctionSub : public CallFunctionSuper
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
    JT_FUNCTION_CONTAINER_WITH_SUPER(JT_SUPER_CLASSES(JT_SUPER_CLASS(CallFunctionSuper)),
                                     JT_FUNCTION(execute_three));
};

void inheritanceTest()
{
    CallFunctionSub cont;
    JT::CallFunctionContext<512> context(json);
    JT::callFunction(cont, context);

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
    CallFunctionVirtualOverload cont;
    JT::CallFunctionContext<512> context(json);
    JT::callFunction(cont, context);

    JT_ASSERT(cont.override_called);
    JT_ASSERT(!cont.called_one);
    JT_ASSERT(cont.called_two);
    JT_ASSERT(cont.called_three);

    fprintf(stderr, "return string\n%s\n", context.return_serializer.returnString().c_str());
    if (context.parse_context.error != JT::Error::NoError)
        fprintf(stderr, "callFunction failed \n%s\n", context.parse_context.tokenizer.makeErrorString().c_str());
    JT_ASSERT(context.parse_context.error == JT::Error::NoError);
}

int main()
{
    simpleTest();
    inheritanceTest();
    virtualFunctionTest();
}

