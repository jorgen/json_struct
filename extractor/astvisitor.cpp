#include "astvisitor.h"
#include <clang/AST/DeclCXX.h>
#include <clang/Lex/Lexer.h>

ClassVisitor::ClassVisitor(clang::ASTContext *context)
	: clang::RecursiveASTVisitor<ClassVisitor>()
	, _context(context)
{
}

TemplateVisitor::TemplateVisitor(clang::ASTContext *context)
    : clang::RecursiveASTVisitor<TemplateVisitor>()
    , _context(context)
{
}

//bool ClassVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl *declaration)
//{
//	clang::FullSourceLoc fullLocation = _context->getFullLoc(declaration->getLocStart());
//    llvm::StringRef filename = _context->getSourceManager().getFilename(fullLocation);
//    if (filename.size() == 0)
//        return true;
//    if (_conf.file.compare(filename.str()))
//        return true;
//
//    for (auto decl : declaration->decls()) {
//        if (clang::NamedDecl *nDecl = clang::dyn_cast<clang::NamedDecl>(decl)) {
//            fprintf(stderr, "Named decl %s\n", nDecl->getNameAsString().c_str());
//            if (clang::ClassTemplateDecl *tDecl = clang::dyn_cast<clang::ClassTemplateDecl>(nDecl)) {
//                clang::CXXRecordDecl *jsonToolsBase = tDecl->getTemplatedDecl();
//                for(auto method : jsonToolsBase->methods()) {
//                    clang::CXXConstructorDecl *constuctor = clang::dyn_cast<clang::CXXConstructorDecl>(method);
//                    if (constuctor)
//                        fprintf(stderr, "constructor %s\n", method->getNameInfo().getAsString().c_str());
//                    clang::CXXDestructorDecl *destructor = clang::dyn_cast<clang::CXXDestructorDecl>(method);
//                    if (destructor)
//                        fprintf(stderr, "destructor %s\n", method->getNameInfo().getAsString().c_str());
//                    else {
//                        fprintf(stderr, "method: %s\n", method->getNameInfo().getAsString().c_str());
//                        if (method->getNameInfo().getAsString() == "_members") {
//                            fprintf(stderr, "**************\n");
//                            method->dump();
//                            fprintf(stderr, "--------------\n");
//                            if (const clang::DecltypeType *decType = clang::dyn_cast<clang::DecltypeType>(method->getReturnType().getTypePtr())) {
//                                fprintf(stderr, "\t**************\n");
//                                decType->dump();
//                                fprintf(stderr, "\t--------------\n");
//                                if (clang::CallExpr *callExpr = clang::dyn_cast<clang::CallExpr>(decType->getUnderlyingExpr()))
//                                {
////                                    for (auto expr : callExpr->getArgs()
//                                    for (int i = 0; i < callExpr->getNumArgs(); i++) {
//                                        clang::Expr *argExpr = callExpr->getArgs()[i];
//                                        fprintf(stderr, "arg: begin\n");
//                                        if (clang::CallExpr *argCall = clang::dyn_cast<clang::CallExpr>(argExpr))
//                                        {
////                                            argCall->dump();
////                                            argCall->getCallReturnType(*_context)->dump();
//                                        }
//                                        
//                                        fprintf(stderr, "arg: end\n");
//                                    }
//                                }
//                                TemplateVisitor templateVisitor(_conf, _context);
//                                templateVisitor.TraverseDecltypeType(const_cast<clang::DecltypeType *>(decType));
//
//                            }
//                        }
////                        auto qualType = method->getReturnType();
////                        qualType->dump();
//////                        qualType->getLocallyUnqualifiedSingleStepDesugaredType().dump();
////                        if (const clang::DecltypeType *declType = clang::dyn_cast<clang::DecltypeType>(qualType.getTypePtr())) {
////                            declType->getUnderlyingType().dump();
////                            fprintf(stderr, "underlying type %d\n", declType->getUnderlyingType()->isBuiltinType());
////                            if (const clang::BuiltinType *builtin = clang::dyn_cast<clang::BuiltinType>(declType->getUnderlyingType()))
////                            {
////                                fprintf(stderr, "builtin %s\n", builtin->getNameAsCString(this->_context->getPrintingPolicy()));
////                            }
////                        }
////                        fprintf(stderr, "type %p\n", clang::dyn_cast<clang::InjectedClassNameType>(qualType.getTypePtr()));
////                        qualType->getContainedAutoType()->dump();
////                        qualType->getArrayElementTypeNoTypeQual()->dump();
////                        fprintf(stderr, "typeclass %s ###\n%s\n", qualType.getAsString().c_str(),
//                    }
//                }
//                for (auto field : jsonToolsBase->fields()) {
//                    fprintf(stderr, "field %s\n", field->getNameAsString().c_str());
//                }
//                fprintf(stderr, "Template type %s\n", tDecl->getNameAsString().c_str());
//                if (clang::CXXRecordDecl *parent = clang::dyn_cast<clang::CXXRecordDecl>(tDecl->getDeclContext())) {
//                    fprintf(stderr, "HEEELELELEL %s\n", parent->getNameAsString	().c_str());
//                }
//                        
//                //fprintf(stderr, "cxxrecord %s\n", rDecl->getDeclContext()->getNameAsString().c_str());
//            }
//        }
//    }
//    for (auto field : declaration->fields()) {
//        fprintf(stderr, "field %s\n", field->getNameAsString().c_str());
//    }
//        for(auto method : declaration->methods()) {
//            clang::QualType qualfield = method->getType().getLocalUnqualifiedType().getTypePtr()->getPointeeType();
////            if (qualfield.isNull())
////                continue;
////            fprintf(stderr, "field2 %s and is record%d\n", qualfield.getAsString().c_str(), qualfield.getTypePtr()->isRecordType());;
////                const clang::RecordType *subRecordType  = qualfield->getAsStructureType();
////                if (subRecordType) {
////                    fprintf(stderr, "has subfield\n");
////                }
//            clang::CXXConstructorDecl *constuctor = clang::dyn_cast<clang::CXXConstructorDecl>(method);
//            if (constuctor)
//                fprintf(stderr, "constructor %s\n", method->getNameInfo().getAsString().c_str());
//            clang::CXXDestructorDecl *destructor = clang::dyn_cast<clang::CXXDestructorDecl>(method);
//            if (destructor)
//                fprintf(stderr, "destructor %s\n", method->getNameInfo().getAsString().c_str());
//            else	
//                fprintf(stderr, "method: %s\n", method->getNameInfo().getAsString().c_str());
//        }
//	return true;
//}

//bool ClassVisitor::TraverseCallExpr(clang::CallExpr *e)
//{
//    if (e->getDirectCallee()) {
//    clang::NamedDecl *decl = clang::dyn_cast<clang::NamedDecl>(e->getDirectCallee());
//        if (decl)
//            fprintf(stderr, "Named decl: %s\n", decl->getNameAsString().c_str());
//    }
//    e->dump();
//    fprintf(stderr, "dump begin\n");
    
    //if (clang::FunctionDecl *func = e->getDirectCallee()) {
//        if (func->getDeclName())
//    /  fprintf(stderr, "func: %s\n",func->getQualifiedNameAsString().c_str());
//        func->dump();.
//    }
//    if (e->getCalleeDecl()) {
//        e->getCalleeDecl()->dump();
//    }
//    clang::FunctionDecl *fdecl = e->getCalleeDecl()->getAsFunction();
//    if (fdecl) {
//        fdecl->dump();
//    }
//    }
//    e->getType()->dump();
//    e->dump();
//    fprintf(stderr, "dump end\n");
//    e->dump();
//    RecursiveASTVisitor<ClassVisitor>::TraverseCallExpr(e);
//    return true;
//}

AstConsumer::AstConsumer()
	: clang::ASTConsumer()
{

}

void AstConsumer::HandleTranslationUnit(clang::ASTContext &context)
{
	ClassVisitor visitor(&context);
    fprintf(stderr, "should visit template instanciation %d\n", visitor.shouldVisitTemplateInstantiations());
	visitor.TraverseDecl(context.getTranslationUnitDecl());
//    visitor.Traverse
//    visitor.TraverseCallExpr(context.getexpr    )
	//clang::CharSourceRange lexerRange = clang::Lexer::makeFileCharRange(range,context.getSourceManager(), context.LangOpts);

}
