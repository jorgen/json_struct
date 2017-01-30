#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"
#include "clang/AST/Comment.h"

#include <json_tools.h>
#include <extractor/extractor_api.h>

using namespace clang;
using namespace clang::ast_matchers;

std::string normalizeTypeName(const std::string &type)
{
    std::string return_type = type;
    if (return_type == "_Bool")
        return_type = "bool";
    return return_type;
}

template<typename T>
static std::string getLiteralText(const SourceManager &sm, const LangOptions &langOpts, T t)
{
    clang::SourceLocation b(t->getLocStart()), _e(t->getLocEnd());
    clang::SourceLocation e(clang::Lexer::getLocForEndOfToken(_e, 0, sm, langOpts));
    return std::string(sm.getCharacterData(b), sm.getCharacterData(e) - sm.getCharacterData(b));
}

static void commentForDecl(const Decl *decl, JT::OptionalChecked<Comment> &comment)
{
    ASTContext& ctx = decl->getASTContext();
    ctx.getLangOpts();
    ctx.getSourceManager();
    comments::FullComment *fc = ctx.getCommentForDecl(decl, 0);


    if (!fc)
        return;

    for (auto &block : fc->getBlocks()) {
        if (clang::comments::ParagraphComment *paragraph = clang::dyn_cast<clang::comments::ParagraphComment>(block)) {
            comment.data.paragraphs.push_back(Paragraph());
            Paragraph &para = comment.data.paragraphs.back();
            comment.assigned = true;
            auto child = paragraph->child_begin();
            bool first = true;
            while (child != paragraph->child_end()) {
                if (first) {
                    first = false;
                    clang::comments::InlineCommandComment *inlineCommand = clang::dyn_cast<clang::comments::InlineCommandComment>(*child);
                    if (inlineCommand) {
                        std::string commandName = Lexer::getSourceText(clang::CharSourceRange(inlineCommand->getCommandNameRange(), true), ctx.getSourceManager(), ctx.getLangOpts()).str();
                        commandName = commandName.substr(commandName.find('\\') + 1);
                        if (commandName.size()) {
                            para.command = commandName;
                            ++child;
                            continue;
                        }
                    }
                }
                clang::comments::TextComment *textComment = clang::dyn_cast<clang::comments::TextComment>(*child);
                if (textComment) {
                    comment.assigned = true;
                    para.lines.push_back(textComment->getText().str());
                }
                ++child;
            }
        }
    }
}

class DefaultValueMatcher : public MatchFinder::MatchCallback
{
public:
    DefaultValueMatcher(Member &member)
        : member(member)
    {}
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
                member.default_value = getLiteralText(sm, lo, str);
            } else if (const FloatingLiteral *f = Result.Nodes.getNodeAs<FloatingLiteral>("float")) {
                member.default_value = getLiteralText(sm, lo, f);
            } else if (const IntegerLiteral *i = Result.Nodes.getNodeAs<IntegerLiteral>("int")) {
                member.default_value = getLiteralText(sm, lo, i);
            } else if (const CXXBoolLiteralExpr *b = Result.Nodes.getNodeAs<CXXBoolLiteralExpr>("bool")) {
                member.default_value = getLiteralText(sm, lo, b);
            } else if (const CXXNullPtrLiteralExpr *ptr = Result.Nodes.getNodeAs<CXXNullPtrLiteralExpr>("ptr")) {
                member.default_value = getLiteralText(sm, lo, ptr);
            }
        }
    }
    Member &member;
};

class MetaMemberMatcher : public MatchFinder::MatchCallback
{
public:
    MetaMemberMatcher(JsonStruct &json_struct)
        : json_struct(json_struct)
    {}
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

        json_struct.members.push_back(Member());
        Member &member = json_struct.members.back();
        member.name = nameLiteral->getString().str();
        commentForDecl(field, member.comment);

        member.type = normalizeTypeName(field->getType().getAsString());

        DefaultValueMatcher valueMatcher(member);
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(DefaultValueMatcher::metaMatcher(), &valueMatcher);
        finder.match(*field, field->getASTContext());

    }
    JsonStruct &json_struct;
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
        const RecordDecl *parent = Result.Nodes.getNodeAs<clang::RecordDecl>("parent");
        const Decl *return_type = Result.Nodes.getNodeAs<clang::Decl>("return_type");
        assert(parent && return_type);
        JsonStruct json_struct;
        json_struct.type = parent->getName().str();
        MetaMemberMatcher memberMatcher(json_struct);
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(MetaMemberMatcher::metaMatcher(), &memberMatcher);
        finder.match(*return_type, return_type->getASTContext());
        fprintf(stderr, "found \n%s\n", JT::serializeStruct(json_struct).c_str());
    }
};

class MetaFunctionMatcher : public MatchFinder::MatchCallback
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
                                has(unaryOperator(has(declRefExpr(hasDeclaration(decl(anything()).bind("function_ref"))))))
                            ))
                        ))
                    ))
                ))
            ))
        ))).bind("root");
    }
    void run(const MatchFinder::MatchResult &Result) override {
        const CXXMethodDecl *function = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("function_ref");
        const StringLiteral *nameLiteral = Result.Nodes.getNodeAs<clang::StringLiteral>("string_name");
        assert(function && nameLiteral);
        fprintf(stderr, "Function meta match %s\n", nameLiteral->getString().str().c_str());
        fprintf(stderr, "Function emta name %s\n", function->getDeclName().getAsString().c_str());
        assert(function->param_size == 1);
        //const LValueReferenceType *param = clang::dyn_cast<const LValueReferenceType>(function->parameters().front()->getType());
        clang::QualType nonConst = function->parameters().front()->getType()->getPointeeType();
        nonConst.removeLocalConst();
        fprintf(stderr, "Name of argument %s\n", nonConst.getAsString().c_str());

        clang::QualType nonConstReturn = function->getReturnType();
        nonConstReturn.removeLocalConst();
        fprintf(stderr, "Name of return %s\n", nonConstReturn.getAsString().c_str());
    }
};

class ClassWithFunctionMetaMatcher : public MatchFinder::MatchCallback {
public:
    static DeclarationMatcher metaMatcher()
    {
        return cxxRecordDecl(hasName("JsonToolsFunctionContainer"),
            has(cxxMethodDecl(hasName("jt_static_meta_functions_info"),
                has(stmt(
                    has(declStmt(has(varDecl(anything()).bind("return_type"))))
                ))
            ).bind("func")),
            hasAncestor(recordDecl().bind("parent")),
            unless(hasAncestor(recordDecl(hasName("JsonToolsFunctionContainer")))),
            isTemplateInstantiation()).bind("meta");
    }

    void run(const MatchFinder::MatchResult &Result) override
    {
        const RecordDecl *parent = Result.Nodes.getNodeAs<clang::RecordDecl>("parent");
        const Decl *return_type = Result.Nodes.getNodeAs<clang::Decl>("return_type");
        assert(parent && return_type);
        fprintf(stderr, "found \n%s\n", parent->getName().str().c_str());
        //return_type->dump();
        MetaFunctionMatcher metaFunctionMatcher;
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(MetaFunctionMatcher::metaMatcher(), &metaFunctionMatcher);
        finder.match(*return_type, return_type->getASTContext());
    }
};