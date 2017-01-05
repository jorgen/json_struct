#ifndef ASTVISITOR_H
#define ASTVISITOR_H
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

class ClassVisitor : public clang::RecursiveASTVisitor<ClassVisitor> {
public:
    ClassVisitor(clang::ASTContext *context);
    bool shouldVisitTemplateInstantiations() const { return true; }
    bool VisitStmt (clang::Stmt *s)
    {
        return true;
    }
    bool VisitType (clang::Type *t)
    {
        return true;
    }
    
    bool VisitDecl (clang::Decl *d)
    {
        return true;
    }

    bool VisitFunctionDecl(clang::FunctionDecl *decl)
    {
        if (decl->getIdentifier() && decl->getName() == llvm::StringRef("jt_static_meta_data_info")) {
            //clang::QualType qual = decl->getReturnType();
            const clang::DecltypeType *declRetType = clang::dyn_cast<const clang::DecltypeType>(decl->getReturnType().getNonReferenceType().getTypePtr());
            clang::CallExpr *callExpr = clang::dyn_cast<clang::CallExpr>(declRetType->getUnderlyingExpr());

            //fprintf(stderr, "callexpr %p \n", callExpr);
            callExpr->dump();

            for (int i = 0; i < callExpr->getNumArgs(); i++) {
                fprintf(stderr, "arg %d: ", i);
                callExpr->getArgs()[i]->dump();
            }
        }
        return true;
    }
    
    bool VisitCallExpr(clang::CallExpr *c)
    {
//        clang::FunctionDecl *decl = c->getDirectCallee();
//        if (decl && decl->getNameAsString() == "jt_static_meta_data_info") {
//            clang::QualType qual = decl->getReturnType().getNonReferenceType().getDesugaredType(*_context);
//            assert(qual.getTypePtr());
//            const clang::RecordDecl *meta_info_record = clang::dyn_cast<const clang::RecordType>(qual.getTypePtr())->getDecl();
//            assert(meta_info_record);
//            fprintf(stderr, "fields: \n");
//            for (auto field : meta_info_record->fields())
//            {
//                clang::QualType tupleQualType = field->getType().getNonReferenceType().getDesugaredType(*_context);
//                clang::ClassTemplateSpecializationDecl *meta_info_tuple = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(clang::dyn_cast<const clang::RecordType>(tupleQualType.getTypePtr())->getDecl());
//                //if (meta_info_tuple)
//
//                //for (auto tuple_field : meta_info_tuple->fields()) {
//                    //fprintf(stderr, "field\n");
//                    //tuple_field->dump();
//                //}
//                int field_index = 0;
//                for (auto base : meta_info_tuple->bases()) {
//                    fprintf(stderr, "field index %d\n", field_index++);
//                    const clang::TemplateSpecializationType *base_type = clang::dyn_cast<const clang::TemplateSpecializationType>(base.getType().getTypePtr());
//                    if (base_type) {
//                        const clang::ClassTemplateSpecializationDecl *record = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(clang::dyn_cast<const clang::RecordType>(base_type->desugar().getTypePtr())->getDecl());
//                        assert(record);
//                        for (auto record_field : record->fields()) {
//                            const clang::ClassTemplateSpecializationDecl *memberinfo = clang::dyn_cast<const clang::ClassTemplateSpecializationDecl>(clang::dyn_cast<const clang::RecordType>(record_field->getType().getDesugaredType(*_context).getTypePtr())->getDecl());
//                            memberinfo->dump();
//                        }
//                    }
//                }
//                //fprintf(stderr, "\tmemeber: %s\n", field->getType().getAsString().c_str());
//            }
            //const clang::DecltypeType *dtype = clang::dyn_cast<clang::DecltypeType>(qual.getTypePtr());
            //assert(dtype);

            //clang::QualType declQualType = dtype->getUnderlyingType();

            //fprintf(stderr, "Qual %s\n", declQualType.getAsString().c_str());

            //decl->dump();
        //}
        return true;
    }
private:
	clang::ASTContext *_context;
};


class TemplateVisitor : public clang::RecursiveASTVisitor<TemplateVisitor>
{
public:
    TemplateVisitor(clang::ASTContext *context);
    bool VisitTemplateTypeParmDecl(clang::TemplateTypeParmDecl *D);
    bool shouldVisitTemplateInstantiations() { return true; }
private:
    clang::ASTContext *_context;
};

class AstConsumer : public clang::ASTConsumer {
public:
	AstConsumer();
	virtual void HandleTranslationUnit(clang::ASTContext &context);
};

class Action : public clang::ASTFrontendAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer( clang::CompilerInstance &Compiler, llvm::StringRef InFile) override {
        return std::unique_ptr<clang::ASTConsumer>(new AstConsumer);
    }
};

#endif //ASTVISITOR_H

