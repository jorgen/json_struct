#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Lexer.h"
#include "clang/AST/Comment.h"
#include <clang/Sema/Sema.h>

#include <json_tools.h>
#include <extractor/extractor_api.h>

using namespace clang;
using namespace clang::ast_matchers;

struct Extractor
{
    std::vector<FunctionObject> function_objects;
    CompilerInstance *current_instance;
};

static void typeDefForQual(Extractor &extractor, clang::QualType type, TypeDef &json_type);

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
    MetaMemberMatcher(Extractor &extractor, TypeDef &type_def)
        : extractor(extractor)
        , type_def(type_def)
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
        if (!type_def.record_type)
            type_def.record_type.reset(new Record());
        type_def.record_type->members.push_back(Member());
        Member &member = type_def.record_type->members.back();
        member.name = nameLiteral->getString().str();
        commentForDecl(field, member.comment);

        typeDefForQual(extractor, field->getType(), member.type);

        DefaultValueMatcher valueMatcher(member);
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(DefaultValueMatcher::metaMatcher(), &valueMatcher);
        finder.match(*field, field->getASTContext());

    }
    Extractor &extractor;
    TypeDef &type_def;
};

class TemplateSpecialisationMatcher : public MatchFinder::MatchCallback
{
public:
    TemplateSpecialisationMatcher(Extractor &extractor)
        : extractor(extractor)
        , return_type(nullptr)
    {}
    static DeclarationMatcher metaMatcher()
    {
        return classTemplateSpecializationDecl(
            has(cxxRecordDecl(hasName("JsonToolsBase"))),
            has(cxxMethodDecl(hasName("jt_static_meta_data_info"),
                has(stmt(
                    has(declStmt(has(varDecl(anything()).bind("return_type"))))
                ))
            ).bind("func")));
    }
    
    void run(const MatchFinder::MatchResult &Result) override
    {
        return_type = Result.Nodes.getNodeAs<const clang::VarDecl>("return_type");
    }

    Extractor &extractor;
    const VarDecl *return_type;
};

static ClassTemplateSpecializationDecl *getJTMetaSpecialisation(Extractor &extractor, CXXRecordDecl *parentClass, ClassTemplateDecl *metaTemplate)
{
    TemplateArgument t_args(QualType(parentClass->getTypeForDecl(), 0));
    auto template_arg = llvm::makeArrayRef(t_args);
    void *insertion_point;
    auto retval = metaTemplate->findSpecialization(template_arg, insertion_point);
    if (!retval) {
        retval = ClassTemplateSpecializationDecl::Create(parentClass->getASTContext(), TTK_Class, parentClass->getDeclContext(), {}, {}, metaTemplate, template_arg, nullptr);
        metaTemplate->AddSpecialization(retval, insertion_point);
    }
    bool is_incomplete = extractor.current_instance->getSema().RequireCompleteType({}, parentClass->getASTContext().getTypeDeclType(retval), 0);
    return is_incomplete ? nullptr : retval;
}

class ClassWithMetaMatcher : public MatchFinder::MatchCallback
{
public :
    ClassWithMetaMatcher(Extractor &extractor, TypeDef &json_typedef)
        : extractor(extractor)
        , json_typedef(json_typedef)
    {}
    static DeclarationMatcher metaMatcher()
    {
        return cxxRecordDecl(has(classTemplateDecl(hasName("JsonToolsBase"),
            has(cxxRecordDecl(hasName("JsonToolsBase"),
                has(cxxMethodDecl(hasName("jt_static_meta_data_info"),
                    has(stmt(
                        has(declStmt(has(varDecl(anything()).bind("return_type"))))
                    ))
                ).bind("func"))
            ))
        ).bind("class_template"))).bind("parent");
    }

    void run(const MatchFinder::MatchResult &Result) override
    {
        CXXRecordDecl *parent = const_cast<CXXRecordDecl *>(Result.Nodes.getNodeAs<clang::CXXRecordDecl>("parent"));
        assert(parent && return_type);
        assert(!json_typedef.assigned);
        ClassTemplateDecl *classTemplate = const_cast<ClassTemplateDecl *>(Result.Nodes.getNodeAs<clang::ClassTemplateDecl>("class_template"));
        ClassTemplateSpecializationDecl *meta_specialization = getJTMetaSpecialisation(extractor, parent, classTemplate);
        ast_matchers::MatchFinder returnFinder;
        TemplateSpecialisationMatcher returnMatcher(extractor);
        returnFinder.addMatcher(TemplateSpecialisationMatcher::metaMatcher(), &returnMatcher);
        returnFinder.match(*meta_specialization, classTemplate->getASTContext());

        MetaMemberMatcher memberMatcher(extractor, json_typedef);
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(MetaMemberMatcher::metaMatcher(), &memberMatcher);
        finder.match(*returnMatcher.return_type, classTemplate->getASTContext());
    }

    Extractor &extractor;
    TypeDef &json_typedef;
};

static void typeDefForQual(Extractor &extractor, clang::QualType type, TypeDef &json_type)
{
    if (type->isReferenceType())
        type = type->getPointeeType(); 

    type.removeLocalConst();
    if (TagDecl *tag_decl = type->getAsTagDecl()) {
        IdentifierInfo *info = tag_decl->getIdentifier();
        json_type.type = normalizeTypeName(info->getName().str());
        if (NamespaceDecl *ns = clang::dyn_cast<NamespaceDecl>(tag_decl->getDeclContext()))
            json_type.name_space = ns->getName().str();


        if (ClassTemplateSpecializationDecl *t_decl = clang::dyn_cast<ClassTemplateSpecializationDecl>(type->getAsTagDecl())) {
            for (unsigned n = 0; n < t_decl->getTemplateArgs().size(); n++) {
                const TemplateArgument &arg = t_decl->getTemplateArgs().get(n);
                if (arg.getKind() != TemplateArgument::Type)
                    continue;
                json_type.template_parameters.push_back(TypeDef());
                TypeDef &template_arg_typedef = json_type.template_parameters.back();
                typeDefForQual(extractor, arg.getAsType(), template_arg_typedef);
            }
        }
    }
    else {
        json_type.type = normalizeTypeName(type.getAsString());
    }

    const clang::RecordType *record = clang::dyn_cast<const RecordType>(type.getTypePtr());
    CXXRecordDecl *record_decl = 0;
    if (record)
        record_decl = clang::dyn_cast<CXXRecordDecl>(record->getDecl());
    if (record_decl) {
        bool isRef = record_decl->isReferenced();
        ClassWithMetaMatcher matcher(extractor, json_type);
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(ClassWithMetaMatcher::metaMatcher(), &matcher);
        finder.match(*record_decl, record_decl->getASTContext());
    }

}

class MetaFunctionMatcher : public MatchFinder::MatchCallback
{
public:
    MetaFunctionMatcher(Extractor &extractor, FunctionObject &object)
        : extractor(extractor)
        , function_object(object)
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
        assert(function->param_size == 1);
        clang::QualType firstParameter = function->parameters().front()->getType();
        function_object.functions.push_back(Functio());
        Functio &func = function_object.functions.back();
        func.name = nameLiteral->getString().str();
        commentForDecl(function, func.comment);
        typeDefForQual(extractor, firstParameter, func.argument);
        typeDefForQual(extractor, function->getReturnType(), func.return_type);
    }

    Extractor &extractor;
    FunctionObject &function_object;
};

class ClassWithFunctionMetaMatcher : public MatchFinder::MatchCallback {
public:
    ClassWithFunctionMetaMatcher(Extractor &extractor)
        : extractor(extractor)
    {}
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
        FunctionObject functionObject;
        MetaFunctionMatcher metaFunctionMatcher(extractor, functionObject);
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(MetaFunctionMatcher::metaMatcher(), &metaFunctionMatcher);
        finder.match(*return_type, return_type->getASTContext());
        fprintf(stderr, "Found\n%s\n", JT::serializeStruct(functionObject).c_str());
    }

    Extractor &extractor;
};