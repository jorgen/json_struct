#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

#include "astvisitor.h"
#include "astmatcher.h"

using namespace clang::tooling;
using namespace llvm;

static llvm::cl::OptionCategory MyToolCategory("extractor options");

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::extrahelp MoreHelp("\nMore help text...\n");

int main(int argc, const char **argv)
{
    CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
    ClangTool tool(OptionsParser.getCompilations(),
                   OptionsParser.getSourcePathList());
    ClassWithMetaMatcher matchCallback;
    clang::ast_matchers::MatchFinder finder;
    finder.addMatcher(ClassWithMetaMatcher::metaMatcher(), &matchCallback);
    int tool_run =  tool.run(newFrontendActionFactory(&finder).get());
    return 0;
}
