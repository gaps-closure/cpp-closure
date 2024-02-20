#ifndef CLOSURE_DIVIDER_MATCHER_H
#define CLOSURE_DIVIDER_MATCHER_H

// #include <iostream>
// #include <filesystem>
// #include <fstream>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"

#include "clang/Basic/SourceManager.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/FixItRewriter.h"

#include "Topology.h"

using namespace clang;
using namespace ast_matchers;

class ClosureDividerMatcher
    : public clang::ast_matchers::MatchFinder::MatchCallback 
{
public:
    explicit ClosureDividerMatcher(clang::Rewriter &rewriter, Topology topology)
        : rewriter(rewriter), 
          topology(topology) {
    }
    
    void onEndOfTranslationUnit() override;
    void run(const clang::ast_matchers::MatchFinder::MatchResult &) override;
    bool isInFile(const clang::SourceManager &sm, const Decl *decl);

private:
    clang::Rewriter rewriter;
    Topology topology;
    vector<SourceRange> parentRanges;

    bool matchFunctionDecl(const clang::SourceManager &sm, const FunctionDecl *func);
    bool matchFunctionCall(const clang::SourceManager &sm, const CallExpr *expr);

    bool matchVarDecl(const clang::SourceManager &sm, const VarDecl *var);
    bool matchVarRef(const clang::SourceManager &sm, const DeclRefExpr *varRef);

    bool matchRecordDecl(const clang::SourceManager &sm, const CXXRecordDecl *record);

    void showLoc(string msg, const clang::SourceManager &sm, const Decl *decl);
    void showLoc(string msg, const clang::SourceManager &sm, const Expr *expr);
    void showLoc(string msg, const clang::SourceManager &sm, SourceLocation begin, SourceLocation end);
};

class ClosureMatcherASTConsumer : public clang::ASTConsumer 
{
public:
    explicit ClosureMatcherASTConsumer(clang::CompilerInstance &compiler, Topology &topology,
                                       bool mainFileOnly, clang::Rewriter &rewriter);
    void HandleTranslationUnit(clang::ASTContext &ctx) {
        finder.matchAST(ctx);
    }

private:
    clang::SourceManager &sm;
    // Should this plugin be only run on the main translation unit?
    bool mainTUOnly = true;

    clang::ast_matchers::MatchFinder finder;
    ClosureDividerMatcher matcherHandler;
};

#endif
