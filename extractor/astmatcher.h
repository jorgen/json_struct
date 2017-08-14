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
    std::vector<JT::FunctionObject> function_objects;
    CompilerInstance *current_instance;
};

static void typeDefForQual(Extractor &extractor, clang::QualType type, JT::TypeDef &json_type);

std::string normalizeTypeName(const std::string &type)
{
    std::string return_type = type;
    if (return_type == "_Bool")
        return_type = "bool";
    else if (return_type == "basic_string")
        return_type = "string";
    return return_type;
}

template<typename T>
static std::string getLiteralText(const SourceManager &sm, const LangOptions &langOpts, T t)
{
    clang::SourceLocation b(t->getLocStart()), _e(t->getLocEnd());
    clang::SourceLocation e(clang::Lexer::getLocForEndOfToken(_e, 0, sm, langOpts));
    return std::string(sm.getCharacterData(b), sm.getCharacterData(e) - sm.getCharacterData(b));
}

static void commentForDecl(const Decl *decl, JT::OptionalChecked<JT::Comment> &comment)
{
    ASTContext& ctx = decl->getASTContext();
    ctx.getLangOpts();
    ctx.getSourceManager();
    comments::FullComment *fc = ctx.getCommentForDecl(decl, 0);


    if (!fc)
        return;

    for (auto &block : fc->getBlocks()) {
        if (clang::comments::ParagraphComment *paragraph = clang::dyn_cast<clang::comments::ParagraphComment>(block)) {
            comment.data.paragraphs.push_back(JT::Paragraph());
            JT::Paragraph &para = comment.data.paragraphs.back();
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

static void get_super_classes(const CXXMethodDecl *super_func, std::vector<const CXXRecordDecl *> &return_vector)
{
    if (!super_func)
        return;
    const LValueReferenceType *return_ref = clang::dyn_cast<LValueReferenceType>(super_func->getReturnType().getTypePtr());
    if (!return_ref)
        return;
    const DecltypeType *super_func_return_decltype = return_ref->getPointeeType()->getAs<DecltypeType>();
    if (!super_func_return_decltype)
        return;
    const TemplateSpecializationType *super_tuple = clang::dyn_cast<TemplateSpecializationType>(super_func_return_decltype->getUnderlyingType());
    if (!super_tuple)
        return;
    return_vector.reserve(super_tuple->template_arguments().size());
    
    for (const TemplateArgument &arg : super_tuple->template_arguments()) {
        const RecordType *record = clang::dyn_cast<RecordType>(arg.getAsType()->getUnqualifiedDesugaredType());
        const ClassTemplateSpecializationDecl *superInfoTemplate = clang::dyn_cast<ClassTemplateSpecializationDecl>(record->getDecl());
        const TemplateArgument &super_class_t_arg = superInfoTemplate->getTemplateArgs().get(0);
        const RecordType *super_record = clang::dyn_cast<RecordType>(super_class_t_arg.getAsType());
        const CXXRecordDecl *super_decl = clang::dyn_cast<CXXRecordDecl>(super_record->getDecl());
        return_vector.push_back(super_decl);
    }
}

class DefaultValueMatcher : public MatchFinder::MatchCallback
{
public:
    DefaultValueMatcher(JT::Member &member)
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
    JT::Member &member;
};

class MetaMemberMatcher : public MatchFinder::MatchCallback
{
public:
    MetaMemberMatcher(Extractor &extractor, JT::TypeDef &type_def)
        : extractor(extractor)
        , type_def(type_def)
    {}
    static StatementMatcher metaMatcher()
    {
        return callExpr(forEachDescendant(
            materializeTemporaryExpr(has(
                cxxBindTemporaryExpr(has(
                    callExpr(
                        has(stringLiteral(anything()).bind("string_name")),
                        has(unaryOperator(has(declRefExpr(hasDeclaration(fieldDecl(anything()).bind("member_ref"))))))
                    )))))));
    }
    void run(const MatchFinder::MatchResult &Result) override {
        const FieldDecl *field = Result.Nodes.getNodeAs<clang::FieldDecl>("member_ref");
        const StringLiteral *nameLiteral = Result.Nodes.getNodeAs<clang::StringLiteral>("string_name");
        assert(field && nameLiteral);
        if (!type_def.record_type.data)
            type_def.record_type.data.reset(new JT::Record());
        type_def.record_type.data->members.push_back(JT::Member());
        JT::Member &member = type_def.record_type.data->members.back();
        member.name = nameLiteral->getString().str();
        commentForDecl(field, member.comment);

        typeDefForQual(extractor, field->getType(), member.type);

        DefaultValueMatcher valueMatcher(member);
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(DefaultValueMatcher::metaMatcher(), &valueMatcher);
        finder.match(*field, field->getASTContext());

    }
    Extractor &extractor;
    JT::TypeDef &type_def;
};

static ClassTemplateSpecializationDecl *getJTMetaSpecialisation(Extractor &extractor, const CXXRecordDecl *parentClass, ClassTemplateDecl *metaTemplate)
{
    TemplateArgument t_args(QualType(parentClass->getTypeForDecl(), 0));
    auto template_arg = llvm::makeArrayRef(t_args);
    ClassTemplateSpecializationDecl *retval = ClassTemplateSpecializationDecl::Create(parentClass->getASTContext(), TTK_Class, const_cast<CXXRecordDecl *>(parentClass)->getDeclContext(), {}, {}, metaTemplate, template_arg, nullptr);
    bool is_incomplete = extractor.current_instance->getSema().RequireCompleteType({}, parentClass->getASTContext().getTypeDeclType(retval), 0);
    return is_incomplete ? nullptr : retval;
}

class ClassWithMetaMatcher : public MatchFinder::MatchCallback
{
public :
    ClassWithMetaMatcher(Extractor &extractor, JT::TypeDef &json_typedef)
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
                ).bind("func")),
                has(cxxMethodDecl(hasName("jt_static_meta_super_info")).bind("super_func"))
            ))
        ).bind("class_template"))).bind("parent");
    }

    void run(const MatchFinder::MatchResult &Result) override
    {
        const CXXRecordDecl *parent = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("parent");
        const CXXMethodDecl *super_func = Result.Nodes.getNodeAs<CXXMethodDecl>("super_func");

        std::vector<const CXXRecordDecl *> super_classes;
        get_super_classes(super_func, super_classes);
        if (super_classes.size()) {
            if (!json_typedef.record_type.data)
                json_typedef.record_type.data.reset(new JT::Record());
            json_typedef.record_type.data->super_classes.data.reserve(super_classes.size());
        }
        for (const CXXRecordDecl *super : super_classes)
        {
            json_typedef.record_type.data->super_classes.data.push_back(JT::TypeDef());
            JT::TypeDef &super_typedef = json_typedef.record_type.data->super_classes.data.back();
            typeDefForQual(extractor, QualType(super->getTypeForDecl(), 0), super_typedef);
        }

        ClassTemplateDecl *classTemplate = const_cast<ClassTemplateDecl *>(Result.Nodes.getNodeAs<clang::ClassTemplateDecl>("class_template"));
        ClassTemplateSpecializationDecl *meta_specialization = getJTMetaSpecialisation(extractor, parent, classTemplate);
        
        const DecltypeType *return_type = nullptr;
        for (auto *method : meta_specialization->methods()) {
            if (method->getName().str() == "jt_static_meta_data_info") {
                const LValueReferenceType *ref = method->getReturnType()->getAs<LValueReferenceType>();
                return_type = ref->getPointeeType()->getAs<DecltypeType>();
                break;
            }
        }
        MetaMemberMatcher memberMatcher(extractor, json_typedef);
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(MetaMemberMatcher::metaMatcher(), &memberMatcher);
        finder.match(*return_type->getUnderlyingExpr(), classTemplate->getASTContext());
    }

    Extractor &extractor;
    JT::TypeDef &json_typedef;
};

static void typeDefForQual(Extractor &extractor, clang::QualType type, JT::TypeDef &json_type)
{
    if (type->isReferenceType())
        type = type->getPointeeType();

    if (const clang::ElaboratedType *elaborateType = clang::dyn_cast<const clang::ElaboratedType>(type.getTypePtr()))
		type = elaborateType->getNamedType();

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
                json_type.template_parameters.data.push_back(JT::TypeDef());
                JT::TypeDef &template_arg_typedef = json_type.template_parameters.data.back();
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
        ClassWithMetaMatcher matcher(extractor, json_type);
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(ClassWithMetaMatcher::metaMatcher(), &matcher);
        finder.match(*record_decl, record_decl->getASTContext());
    }

}

class MetaFunctionMatcher : public MatchFinder::MatchCallback
{
public:
    MetaFunctionMatcher(Extractor &extractor, JT::FunctionObject &object)
        : extractor(extractor)
        , function_object(object)
    {}
    static StatementMatcher metaMatcher()
    {
        return callExpr(forEachDescendant(
            materializeTemporaryExpr(has(
                cxxBindTemporaryExpr(has(
                    callExpr(
                        has(stringLiteral(anything()).bind("string_name")),
                        has(unaryOperator(has(declRefExpr(hasDeclaration(decl(anything()).bind("function_ref"))))))
                    )))))));
    }
    void run(const MatchFinder::MatchResult &Result) override {
        const CXXMethodDecl *function = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("function_ref");
        const StringLiteral *nameLiteral = Result.Nodes.getNodeAs<clang::StringLiteral>("string_name");
        assert(function && nameLiteral);
		function_object.functions.push_back(JT::Function());
		JT::Function &func = function_object.functions.back();
		func.name = nameLiteral->getString().str();
		commentForDecl(function, func.comment);
		if (function->parameters().size()) {
			clang::QualType firstParameter = function->parameters().front()->getType();
			typeDefForQual(extractor, firstParameter, func.argument.data);
			func.argument.assigned = true;
		}
		typeDefForQual(extractor, function->getReturnType(), func.return_type);
    }

    Extractor &extractor;
    JT::FunctionObject &function_object;
};

class ClassWithFunctionMetaMatcher : public MatchFinder::MatchCallback {
public:
    ClassWithFunctionMetaMatcher(JT::FunctionObject &functionObject, Extractor &extractor)
        : functionObject(functionObject)
        , extractor(extractor)
    {}
    static DeclarationMatcher metaMatcher()
    {
        return cxxRecordDecl(isSameOrDerivedFrom(cxxRecordDecl(has(classTemplateDecl(
            hasName("JsonToolsFunctionContainer"),
            has(cxxRecordDecl(
                hasName("JsonToolsFunctionContainer"),
                has(cxxMethodDecl(
                    hasName("jt_static_meta_functions_info"),
                    has(stmt(
                        has(declStmt(has(varDecl(anything()).bind("return_type"))))
                    ))).bind("func")),
                has(cxxMethodDecl(
                    hasName("jt_static_meta_super_info")).bind("super_func"))
            ))).bind("class_template"))))).bind("parent");
    }

    void run(const MatchFinder::MatchResult &Result) override
    {
        CXXRecordDecl *parent = const_cast<CXXRecordDecl *>(Result.Nodes.getNodeAs<clang::CXXRecordDecl>("parent"));
        ClassTemplateDecl *classTemplate = const_cast<ClassTemplateDecl *>(Result.Nodes.getNodeAs<clang::ClassTemplateDecl>("class_template"));
        const CXXMethodDecl *super_func = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("super_func");
        std::vector<const CXXRecordDecl *> super_classes;
        get_super_classes(super_func, super_classes);
        functionObject.super_classes.data.reserve(super_classes.size());
        for (const CXXRecordDecl *super_class : super_classes)
        {
            functionObject.super_classes.data.push_back(JT::FunctionObject());
            JT::FunctionObject &super_class_object = functionObject.super_classes.data.back();
            ClassWithFunctionMetaMatcher functionObjectMatcher(super_class_object, extractor);
            clang::ast_matchers::MatchFinder finder;
            finder.addMatcher(ClassWithFunctionMetaMatcher::metaMatcher(), &functionObjectMatcher);
            finder.match(*super_class, super_class->getASTContext());
        }

        ClassTemplateSpecializationDecl *meta_specialization = getJTMetaSpecialisation(extractor, parent, classTemplate);
        const DecltypeType *return_type = nullptr;
        for (auto *method : meta_specialization->methods()) {
            if (method->getName().str() == "jt_static_meta_functions_info") {
                const LValueReferenceType *ref = method->getReturnType()->getAs<LValueReferenceType>();
                return_type = ref->getPointeeType()->getAs<DecltypeType>();
            }
        }
        assert(return_type);
        functionObject.type = parent->getNameAsString();
        MetaFunctionMatcher metaFunctionMatcher(extractor, functionObject);
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(MetaFunctionMatcher::metaMatcher(), &metaFunctionMatcher);
        finder.match(*return_type->getUnderlyingExpr(), classTemplate->getASTContext());
    }

    JT::FunctionObject &functionObject;
    Extractor &extractor;
};

class FindFunctionCall : public MatchFinder::MatchCallback {
public:
    FindFunctionCall(Extractor &extractor)
        : extractor(extractor)
    {}
    static StatementMatcher metaMatcher()
    {
		return callExpr(callee(cxxMethodDecl(hasName("callFunctions"),
											parameterCountIs(1),
                                            isTemplateInstantiation(),
                                            hasDeclContext(namedDecl(hasName("CallFunctionContext"))),
                                            hasParameter(0, hasType(referenceType(pointee(hasDeclaration(cxxRecordDecl().bind("callObject"))))))
                ))).bind("func");
    }

    void run(const MatchFinder::MatchResult &Result) override
    {
        const CXXRecordDecl *callObject = Result.Nodes.getNodeAs<CXXRecordDecl>("callObject");
        extractor.function_objects.push_back(JT::FunctionObject());
        JT::FunctionObject &functionObject = extractor.function_objects.back();
        ClassWithFunctionMetaMatcher functionObjectMatcher(functionObject, extractor);
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(ClassWithFunctionMetaMatcher::metaMatcher(), &functionObjectMatcher);
        finder.match(*callObject, callObject->getASTContext());
    }

    Extractor &extractor;
};
