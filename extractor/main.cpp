#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

#include "astvisitor.h"
#include "astmatcher.h"

#include <extractor/extractor_api.h>

#include <algorithm>

using namespace clang::tooling;
using namespace llvm;

static llvm::cl::OptionCategory ExtractorExtraOptions("extractor options");

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::extrahelp moreHelp("\nMore help text...\n");
static cl::opt<std::string>    outputFile("o", cl::Optional, cl::ValueRequired, cl::desc("Output file name"), cl::cat(ExtractorExtraOptions));
static cl::opt<bool>           linking("l", cl::Optional, cl::ValueDisallowed, cl::desc("Linking mode"), cl::cat(ExtractorExtraOptions));

static Extractor global_extractor;

class RootASTConsumer : public ASTConsumer {
public:
    RootASTConsumer()
        : findFunction(global_extractor)
    {
        Matcher.addMatcher(FindFunctionCall::metaMatcher(), &findFunction);
    }

    void HandleTranslationUnit(ASTContext &Context) override {
        Matcher.matchAST(Context);
    }

private:
    FindFunctionCall findFunction;
    MatchFinder Matcher;
}; 

class RootFrontendAction : public ASTFrontendAction {
public:
  RootFrontendAction() {}
  void EndSourceFileAction() override {
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef file) override {
      global_extractor.current_instance = &CI;
    return llvm::make_unique<RootASTConsumer>(astConsumer);
  }

private:
  RootASTConsumer astConsumer;
};

void sort_typedef(JT::TypeDef &typedef_obj)
{
    for (auto &template_type : typedef_obj.template_parameters.data)
    {
        if (template_type.name_space.data == "__1")
            template_type.name_space.data = "std";

        if (template_type.type == "string" && template_type.name_space.data == "std")
            template_type.template_parameters = {};
        sort_typedef(template_type);
    }
    std::sort(typedef_obj.template_parameters.data.begin(), typedef_obj.template_parameters.data.end(),
              [](JT::TypeDef &a, JT::TypeDef &b)
    {
        return strcmp(a.type.c_str(), b.type.c_str()) < 0;
    });

    if (typedef_obj.record_type.data)
    {
        JT::Record *record = typedef_obj.record_type.data.get();
        std::sort(record->members.begin(), record->members.end(), [](JT::Member &a, JT::Member &b)
        {
            return strcmp(a.name.c_str(), b.name.c_str()) < 0;
        });
        for (auto &member_typedef : record->members)
            sort_typedef(member_typedef.type);
        std::sort(record->super_classes.data.begin(), record->super_classes.data.end(), [](const JT::TypeDef &a, JT::TypeDef &b)
        {
            return strcmp(a.type.c_str(), b.type.c_str()) < 0;
        });
        for (auto &super : record->super_classes.data)
            sort_typedef(super);
    }

}

void sort_function_object(JT::FunctionObject &object)
{
    std::sort(object.functions.begin(), object.functions.end(), [](const JT::Function &a, const JT::Function &b)
    {
        return strcmp(a.name.c_str(), b.name.c_str()) < 0;
    });
    for (auto &function : object.functions)
    {
        if (function.argument.assigned)
                sort_typedef(function.argument.data);
        sort_typedef(function.return_type);
    }
    std::sort(object.super_classes.data.begin(), object.super_classes.data.end(), [](const JT::FunctionObject &a, const JT::FunctionObject &b)
    {
        return strcmp(a.type.c_str(), b.type.c_str()) < 0;
    });
    for (auto &super : object.super_classes.data)
        sort_function_object(super);
}

void add_function_object(std::vector<JT::FunctionObject> &objects, JT::FunctionObject &insert_object)
{
    auto found = std::find_if(objects.begin(), objects.end(), [&insert_object](const JT::FunctionObject &other)
    {
        return other.type == insert_object.type;
    });
    if (found != objects.end())
        return;
    sort_function_object(insert_object);
    objects.push_back(insert_object);
}

int main(int argc, const char **argv)
{
    CommonOptionsParser OptionsParser(argc, argv, ExtractorExtraOptions);
    if (linking)
    {
        std::vector<JT::FunctionObject> out_function_objects;

        for (auto &path : OptionsParser.getSourcePathList())
        {
            FILE *in_file = fopen(path.c_str(), "rb");
            if (!in_file)
            {
                fprintf(stderr, "Failed to read infile.\n");
                return 321;
            }
            
            fseek(in_file, 0, SEEK_END);
            size_t size = ftell(in_file);
            fseek(in_file, 0, SEEK_SET);
            char *data = new char[size];
            fread(data, 1, size, in_file);
            fclose(in_file);
            std::vector<JT::FunctionObject> file_function_objects;
            JT::ParseContext pc(data, size);
            pc.parseTo(file_function_objects);
            for (auto &local_function_object : file_function_objects)
                add_function_object(out_function_objects, local_function_object);
        }

        std::sort(out_function_objects.begin(), out_function_objects.end(), [](const JT::FunctionObject &a, JT::FunctionObject &b)
        {
            return strcmp(a.type.c_str(), b.type.c_str()) < 0;
        });
        FILE *output = stdout;
        if (outputFile.size())
        {
            output = fopen(outputFile.c_str(), "wb");
            if (!output)
            {
                fprintf(stderr, "Failed to write to output file %s\n", outputFile.c_str());
                return 123;
            }
            
        }
        std::string out_json;
        {
            JT::SerializerContext serializeContext(out_json);
            JT::SerializerOptions serializerOptions;
            serializerOptions.setShiftSize(2);
            serializeContext.serializer.setOptions(serializerOptions);
            JT::Token token;
            JT::TypeHandler<std::vector<JT::FunctionObject>>::serializeToken(out_function_objects, token, serializeContext.serializer);
            serializeContext.flush();
        }
        fprintf(output, "%s\n", out_json.c_str());
        if (output != stdout)
            fclose(output);
        
    } else {
        ClangTool tool(OptionsParser.getCompilations(),
                       OptionsParser.getSourcePathList());
        int tool_run = tool.run(newFrontendActionFactory<RootFrontendAction>().get());
        FILE *output = stdout;
        if (outputFile.size())
        {
            output = fopen(outputFile.c_str(), "wb");
            if (!output)
            {
                fprintf(stderr, "Failed to write to output file %s\n", outputFile.c_str());
                return 123;
            }
            
        }
        fprintf(output, "%s\n", JT::serializeStruct(global_extractor.function_objects).c_str());
        if (output != stdout)
            fclose(output);
    }
    return 0;
}
