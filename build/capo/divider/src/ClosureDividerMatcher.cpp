#include <iostream>
#include <regex>
#include <fstream>

#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Sema/Sema.h"

#include "ClosureDividerAttr.h"
#include "ClosureDividerMatcher.h"
#include "Topology.h"

using namespace clang;
using namespace std;
using namespace ast_matchers;

ClosureMatcherASTConsumer::ClosureMatcherASTConsumer(
    clang::CompilerInstance &compiler,
    Topology &topology,
    bool mainFileOnly, 
    clang::Rewriter &rewriter)
    : sm(compiler.getSourceManager()), 
      mainTUOnly(mainFileOnly),
      matcherHandler(rewriter, topology) 
{
    const auto matcherForMemberAccess = cxxMemberCallExpr(
        callee(memberExpr(member(hasName("oldName"))).bind("MemberAccess")),
        thisPointerType(cxxRecordDecl(isSameOrDerivedFrom(hasName("className")))));
    finder.addMatcher(matcherForMemberAccess, &matcherHandler);

    const auto matcherForMemberDecl = cxxRecordDecl(
        allOf(isSameOrDerivedFrom(hasName("className")),
                hasMethod(decl(namedDecl(hasName("oldName"))).bind("MemberDecl"))));
    finder.addMatcher(matcherForMemberDecl, &matcherHandler);

    // const auto method = cxxMethodDecl(
    //     hasDescendant(decl(namedDecl(hasName(oldName))).bind("method")));
    // const auto method = cxxMethodDecl(hasName(oldName)).bind("method");   // ok
        // hasDescendant(cxxMethodDecl(hasName(oldName)))).bind("method");
    // const auto functionDecl(hasDescendant(callExpr().bind("functionCall")));

    // const auto funcDecl = functionDecl().bind("FunctionDecl");
    // finder.addMatcher(funcDecl, &matcherHandler);

    for (Annotation annotation: topology.getFunctions()) {
        // llvm::outs() << "#### " << annotation.getName() << "\n";
        string funcName = annotation.getName();
        const auto funcDecl = functionDecl(hasName(funcName)).bind("FunctionDecl");
        finder.addMatcher(funcDecl, &matcherHandler);

        const auto funcCall = callExpr(callee(functionDecl(hasName(funcName)))).bind("FunctionCall");
        finder.addMatcher(funcCall, &matcherHandler);
    }
    // const auto funcCall = callExpr(callee(functionDecl(hasName(annotation.getName()))),
    //                                    argumentCountIs(0)).bind("FunctionCall");

    for (Annotation annotation: topology.getGlobalScopedVars()) {
        string varName = annotation.getName();
        const auto vDecl = varDecl(hasName(varName), 
                                   hasGlobalStorage(),
                                   isDefinition()).bind("VarDecl");
        finder.addMatcher(vDecl, &matcherHandler);

        // const auto varRef = declRefExpr(varDecl(hasName(varName))).bind("varRef");
        const auto varRef =  declRefExpr(to(varDecl(hasName(varName)))).bind("VarRef");
        finder.addMatcher(varRef, &matcherHandler);
    }

    const auto fdDecl = fieldDecl().bind("FieldDecl");
    finder.addMatcher(fdDecl, &matcherHandler);

    const auto mdDecl = cxxMethodDecl().bind("CXXMethodDecl");
    finder.addMatcher(mdDecl, &matcherHandler);

    const auto recordDecl = cxxRecordDecl().bind("CXXRecordDecl");
    finder.addMatcher(recordDecl, &matcherHandler);
}

void ClosureDividerMatcher::run(const MatchFinder::MatchResult &result) 
{
    const clang::SourceManager &sm = *result.SourceManager;

    const MemberExpr *memberAccess = result.Nodes.getNodeAs<clang::MemberExpr>("MemberAccess");
    if (memberAccess) {
        // printf("access............\n");
        SourceRange callExprSrcRange = memberAccess->getMemberLoc();
        rewriter.ReplaceText(callExprSrcRange, "XXX");
    }

    const NamedDecl *memberDecl = result.Nodes.getNodeAs<clang::NamedDecl>("MemberDecl");
    if (memberDecl && isInFile(sm, memberDecl)) {
        showLoc("decl", sm, memberDecl);

        SourceRange memberDeclSrcRange = memberDecl->getLocation();
        rewriter.ReplaceText(CharSourceRange::getTokenRange(memberDeclSrcRange), "XXX");
    }

    const FunctionDecl *func = result.Nodes.getNodeAs<clang::FunctionDecl>("FunctionDecl");
    if (func && isInFile(sm, func)) {  // func->hasAttrs() && 
        matchFunctionDecl(sm, func);
    }

    const CallExpr *call = result.Nodes.getNodeAs<clang::CallExpr>("FunctionCall");
    if (call) { // && isInFile(sm, call)) {  // func->hasAttrs() && 
        matchFunctionCall(sm, call);
    }

    const VarDecl *var = result.Nodes.getNodeAs<clang::VarDecl>("VarDecl");
    if (var && isInFile(sm, var) && !var->isLocalVarDecl()) { // && var->hasAttrs() ) {
        matchVarDecl(sm, var); 
    }

    const DeclRefExpr *varRef = result.Nodes.getNodeAs<clang::DeclRefExpr>("VarRef");
    if (varRef) { // && isInFile(sm, varRef)) { // && var->hasAttrs() ) {
        matchVarRef(sm, varRef); 
    }

    const FieldDecl *field = result.Nodes.getNodeAs<clang::FieldDecl>("FieldDecl");
    if (field && field->hasAttrs() && isInFile(sm, field)) {
        // showLoc("FieldDecl......", sm, field);
    }

    const CXXMethodDecl *method = result.Nodes.getNodeAs<clang::CXXMethodDecl>("CXXMethodDecl");
    if (method && method->hasAttrs() && isInFile(sm, method)) {
        showLoc("CXXMethodDecl......", sm, method);
        // SourceRange methodRange = method->getSourceRange();
        // rewriter.ReplaceText(methodRange, "XXX");
    }

    const CXXRecordDecl *record = result.Nodes.getNodeAs<clang::CXXRecordDecl>("CXXRecordDecl");
    if (record && isInFile(sm, record)) { // && record->hasAttrs()) {
        matchRecordDecl(sm, record); 
    }
}

bool ClosureDividerMatcher::matchFunctionDecl(const clang::SourceManager &sm, const FunctionDecl *func)
{
    string &level = topology.getLevelInProgress();
    string funcName = func->getNameInfo().getAsString();

    if (topology.isNameInLevel(funcName, level))
        return true;    // keep it

    // showLoc("FunctionDecl......", sm, func);
    replace(func->getSourceRange());

    return true;
}

bool ClosureDividerMatcher::matchFunctionCall(const clang::SourceManager &sm, const CallExpr *call)
{
    const FunctionDecl *callee = call->getDirectCallee();
    string funcName = callee->getNameInfo().getAsString();
    string &level = topology.getLevelInProgress();

    if (topology.isNameInLevel(funcName, level))
        return true;    // keep it

    SourceRange range = call->getSourceRange();
    // check if containing function has been erasded
    bool erased = false;
    for (SourceRange sr : parentRanges) {
        if (sr.fullyContains(range)) {
            erased = true;
            break;
        }
    }
    if (!erased) {
        // showLoc("FunctionCall......", sm, call);
        StringRef prefix("_err_handler_rpc_");
        rewriter.InsertTextBefore(call->getBeginLoc(), prefix);
    }
    return true;
}

bool ClosureDividerMatcher::matchVarDecl(const clang::SourceManager &sm, const VarDecl *var)
{
    string &level = topology.getLevelInProgress();
    string varName = var->getName().str();

    if (topology.isNameInLevel(varName, level))
        return true;    // keep it

    // showLoc("VarDecl......", sm, var);
    replace(var->getSourceRange());

    return true;
}

bool ClosureDividerMatcher::matchVarRef(const clang::SourceManager &sm, const DeclRefExpr *varRef)
{
    return true;  // TODO

    string &level = topology.getLevelInProgress();
    string varName = varRef->getNameInfo().getAsString();

    showLoc("VarRef......", sm, varRef);
    if (topology.isNameInLevel(varName, level))
        return true;    // keep it

    SourceRange range = varRef->getSourceRange();
    LangOptions langOpts;
    // TODO: the following line leads to crash
    std::string original = rewriter.getRewrittenText(range);
    
    // TODO: not complete because of the crash above
    llvm::outs() << Lexer::getSourceText(CharSourceRange::getTokenRange(range), sm, langOpts).str() << "\n";   

    StringRef prefix("_err_handler_rpc_");
    rewriter.InsertTextBefore(varRef->getBeginLoc(), prefix);
    // rewriter.ReplaceText(CharSourceRange::getTokenRange(range), "XXX");
    return true;
}

bool ClosureDividerMatcher::matchRecordDecl(const clang::SourceManager &sm, const CXXRecordDecl *record)
{
    string &level = topology.getLevelInProgress();
    string className = record->getNameAsString();

    if (topology.isInEnclave(className, level) || !record->hasDefinition())
        return true;    // keep it

    showLoc("RecordDecl......", sm, record);
    replace(record->getSourceRange());

    return true;
}

void ClosureDividerMatcher::onEndOfTranslationUnit() 
{
    string file = topology.getOutputFile();
    llvm::outs() << "\t \t" << file << "\n";

    std::error_code error_code;
    llvm::raw_fd_ostream outFile(file, error_code, llvm::sys::fs::OF_None);
    rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(outFile);
    outFile.close();

    // rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID())
    //         .write(llvm::outs());
}

bool ClosureDividerMatcher::isInFile(const clang::SourceManager &sm, const Decl *decl)
{
    string file = sm.getFilename(decl->getLocation()).str();
    string &target = topology.getFileInProcess();

    return (file.length() >= target.length() &&
            !file.compare(file.length() - target.length(), target.length(), target));
}

void ClosureDividerMatcher::replace(SourceRange range)
{
    std::string original = rewriter.getRewrittenText(range);

    // erase all other than whitespace; 
    // keep source range line numbers intact for matchFunctionCall()
    std::regex non_ws("[^\\s]");
    string modified = std::regex_replace(original, non_ws, " ");
    rewriter.ReplaceText(CharSourceRange::getTokenRange(range), modified);

    parentRanges.push_back(range);
}

void ClosureDividerMatcher::showLoc(string msg, const clang::SourceManager &sm, const Decl *decl)
{
    showLoc(msg, sm, decl->getBeginLoc(), decl->getEndLoc());

    auto attr = decl->getAttr<clang::AnnotateAttr>();
    if (attr != nullptr)
        std::cout << attr->getAnnotation().str() << std::endl;
}

void ClosureDividerMatcher::showLoc(string msg, const clang::SourceManager &sm, const Expr *expr)
{
    showLoc(msg, sm, expr->getBeginLoc(), expr->getEndLoc());

    // auto attr = expr->getAttr<clang::AnnotateAttr>();
    // if (attr != nullptr)
    //     std::cout << attr->getAnnotation().str() << std::endl;    
}

void ClosureDividerMatcher::showLoc(string msg, const clang::SourceManager &sm, 
    SourceLocation begin, SourceLocation end)
{
    llvm::outs() << msg 
                << sm.getFilename(begin) << ":"
                << sm.getSpellingLineNumber(begin) << ":"
                << sm.getSpellingColumnNumber(begin) << "-"
                << sm.getSpellingLineNumber(end) << ":"
                << sm.getSpellingColumnNumber(end) << "\n";
}

static ParsedAttrInfoRegistry::Add<ClosureAttrInfo> Y("cle", "cle annotator");
