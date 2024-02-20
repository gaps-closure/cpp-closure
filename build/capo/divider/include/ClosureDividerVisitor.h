// DESCRIPTION:
//    Declares the ClosureDivider visitor
//
#ifndef CLOSURE_DIVIDER_VISITOR_H
#define CLOSURE_DIVIDER_VISITOR_H

#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"

#include "Topology.h"

using namespace clang;

class ClosureDividerVisitor
    : public clang::RecursiveASTVisitor<ClosureDividerVisitor> 
{
public:
    explicit ClosureDividerVisitor(const clang::CompilerInstance &compiler,
                                   Topology &topology)
       : ctx(&compiler.getASTContext()), 
         langOpts(compiler.getLangOpts()),
         topology(topology) {
            
        init();
    }

    ~ClosureDividerVisitor() {
        finish();
    }

    bool VisitCXXRecordDecl(clang::CXXRecordDecl *decl);
    bool VisitFunctionDecl(clang::FunctionDecl *decl);
    bool VisitVarDecl(clang::VarDecl *decl);
    bool VisitFieldDecl(clang::FieldDecl *decl);

private:
    clang::ASTContext *ctx;
    clang::LangOptions langOpts;
    std::string outputDir;
    Topology topology;
    map<string, ofstream *> fds;
    
    void init();
    bool output(clang::Decl *decl);
    void finish();
};

#endif
