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
    ClangTool tool(OptionsParser.getCompilations(),
                   OptionsParser.getSourcePathList());
    int tool_run = tool.run(newFrontendActionFactory<RootFrontendAction>().get());
    fprintf(stdout, "\n%s\n", JT::serializeStruct(global_extractor.function_objects).c_str());
    return 0;
}
