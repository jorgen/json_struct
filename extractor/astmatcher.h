#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"

using namespace clang;
using namespace clang::ast_matchers;

template<typename T>
static std::string getLiteralText(const SourceManager &sm, const LangOptions &langOpts, T t)
{
    clang::SourceLocation b(t->getLocStart()), _e(t->getLocEnd());
    clang::SourceLocation e(clang::Lexer::getLocForEndOfToken(_e, 0, sm, langOpts));
    return std::string(sm.getCharacterData(b), sm.getCharacterData(e) - sm.getCharacterData(b));
}

static std::string commentForDecl(const Decl *decl)
{
    ASTContext& ctx = decl->getASTContext();
    const RawComment* rc = ctx.getRawCommentForDeclNoCache(decl);
    if (!rc)
        return std::string();

    return rc->getBriefText(ctx);
}

class DefaultValueMatcher : public MatchFinder::MatchCallback
{
public:
    static DeclarationMatcher metaMatcher()
    {
        return fieldDecl(anyOf(hasDescendant(stringLiteral(anything()).bind("string")),
                               hasDescendant(floatLiteral(anything()).bind("float")),
                               hasDescendant(integerLiteral(anything()).bind("int")),
                               hasDescendant(cxxBoolLiteral(anything()).bind("bool")),
                               hasDescendant(cxxNullPtrLiteralExpr(anything()).bind("ptr")))).bind("field");
    }

    void run(const MatchFinder::MatchResult &Result) override
    {
        if (const Decl *field = Result.Nodes.getNodeAs<clang::Decl>("field")) {
            SourceManager &sm = field->getASTContext().getSourceManager();
            const LangOptions &lo = field->getASTContext().getLangOpts();
            if (const StringLiteral *str = Result.Nodes.getNodeAs<StringLiteral>("string")) {
                fprintf(stderr, "string literal %s\n", getLiteralText(sm, lo, str).c_str());
            } else if (const FloatingLiteral *f = Result.Nodes.getNodeAs<FloatingLiteral>("float")) {
                fprintf(stderr, "float literal %s\n", getLiteralText(sm, lo, f).c_str());
            } else if (const IntegerLiteral *i = Result.Nodes.getNodeAs<IntegerLiteral>("int")) {
                fprintf(stderr, "int literal %s\n", getLiteralText(sm, lo, i).c_str());
            } else if (const CXXBoolLiteralExpr *b = Result.Nodes.getNodeAs<CXXBoolLiteralExpr>("bool")) {
                fprintf(stderr, "boolliteral %s\n", getLiteralText(sm, lo, b).c_str());
            } else if (const CXXNullPtrLiteralExpr *ptr = Result.Nodes.getNodeAs<CXXNullPtrLiteralExpr>("ptr")) {
                fprintf(stderr, "ptr literal %s\n", getLiteralText(sm, lo, ptr).c_str());
            }


        }
    }
};

class MetaMemberMatcher : public MatchFinder::MatchCallback
{
public:
    static DeclarationMatcher metaMatcher()
    {
        return varDecl(has(exprWithCleanups(
                   has(cxxConstructExpr(
                       has(materializeTemporaryExpr(
                           has(callExpr(
                               forEachDescendant(materializeTemporaryExpr(
                                                 has(callExpr(
                                                     has(stringLiteral(anything()).bind("string_name")),
                                                     has(unaryOperator(has(declRefExpr(hasDeclaration(fieldDecl(anything()).bind("member_ref"))))))
                                                     ))
                                             ))
                               ))
                           ))
                       ))
                   ))).bind("root");
    }
    void run(const MatchFinder::MatchResult &Result) override {
        const FieldDecl *field = Result.Nodes.getNodeAs<clang::FieldDecl>("member_ref");
        const StringLiteral *nameLiteral = Result.Nodes.getNodeAs<clang::StringLiteral>("string_name");
        assert(field && nameLiteral);

        DefaultValueMatcher valueMatcher;
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(DefaultValueMatcher::metaMatcher(), &valueMatcher);
        finder.match(*field, field->getASTContext());

        std::string comment = commentForDecl(field);
        std::string name = nameLiteral->getString().str();

        if (comment.size())
            fprintf(stderr, "comment for field %s\n %s\n", name.c_str(), comment.c_str());
    }
};

class ClassWithMetaMatcher : public MatchFinder::MatchCallback {
public :
    static DeclarationMatcher metaMatcher()
    {
        return cxxRecordDecl(hasName("JsonToolsBase"),
                             has(cxxMethodDecl(hasName("jt_static_meta_data_info"),
                                               has(stmt(
                                                   has(declStmt(has(varDecl(anything()).bind("return_type"))))
                                                   ))
                                               ).bind("func")),
                             hasAncestor(recordDecl().bind("parent")),
                             unless(hasAncestor(recordDecl(hasName("JsonToolsBase")))),
                             isTemplateInstantiation()).bind("meta");
    }

    void run(const MatchFinder::MatchResult &Result) override
    {
        if (const Decl *parent = Result.Nodes.getNodeAs<clang::Decl>("return_type")) {
            fprintf(stderr, "Finding members for node~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
            MetaMemberMatcher memberMatcher;
            clang::ast_matchers::MatchFinder finder;
            finder.addMatcher(MetaMemberMatcher::metaMatcher(), &memberMatcher);
            finder.match(*parent, parent->getASTContext());


        }
    }
};
