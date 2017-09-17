#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

#include "astvisitor.h"
#include "astmatcher.h"

#include <extractor/extractor_api.h>

using namespace clang::tooling;
using namespace llvm;

static llvm::cl::OptionCategory ExtractorExtraOptions("extractor options");

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::extrahelp MoreHelp("\nMore help text...\n");
static cl::opt<std::string>    OutputFile("o", cl::Optional, cl::ValueRequired, cl::desc("Output file name"), cl::cat(ExtractorExtraOptions));
static cl::opt<bool>           Linking("l", cl::Optional, cl::ValueDisallowed, cl::desc("Linking mode"), cl::cat(ExtractorExtraOptions));

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

int main(int argc, const char **argv)
{
    CommonOptionsParser OptionsParser(argc, argv, ExtractorExtraOptions);
    if (Linking)
    {
        fprintf(stderr, "Linking!!!!!\n");
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
            out_function_objects.insert(out_function_objects.end(), file_function_objects.begin(), file_function_objects.end());
        }
        FILE *output = stdout;
        if (OutputFile.size())
        {
            output = fopen(OutputFile.c_str(), "wb");
            if (!output)
            {
                fprintf(stderr, "Failed to write to output file %s\n", OutputFile.c_str());
                return 123;
            }
            
        }
        fprintf(output, "%s\n", JT::serializeStruct(out_function_objects).c_str());
        if (output != stdout)
            fclose(output);
        
    } else {
        ClangTool tool(OptionsParser.getCompilations(),
                       OptionsParser.getSourcePathList());
        int tool_run = tool.run(newFrontendActionFactory<RootFrontendAction>().get());
        FILE *output = stdout;
        if (OutputFile.size())
        {
            output = fopen(OutputFile.c_str(), "wb");
            if (!output)
            {
                fprintf(stderr, "Failed to write to output file %s\n", OutputFile.c_str());
                return 123;
            }
            
        }
        fprintf(output, "%s\n", JT::serializeStruct(global_extractor.function_objects).c_str());
        if (output != stdout)
            fclose(output);
    }
    return 0;
}
