#ifndef PP_FRONTEND_ACTION_H
#define PP_FRONTEND_ACTION_H

#include <string>
#include <vector>

#include "PPCallbacksClosure.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Preprocessor.h"

using namespace llvm;

namespace clang {
namespace pp_divider {

namespace {

class PPFrontendAction : public ASTFrontendAction {
public:
    PPFrontendAction(const FilterType &filters, raw_ostream &OS)
        : filters(filters), OS(OS) {}

protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                    StringRef InFile) override {
        Preprocessor &PP = CI.getPreprocessor();
        PP.addPPCallbacks(std::make_unique<PPCallbacksClosure>(filters, PP));

        return std::make_unique<ASTConsumer>();
    }

    bool BeginSourceFileAction(CompilerInstance &ci) override {
        return true;
    }

    void EndSourceFileAction() override {
    }

private:
    const FilterType &filters;
    raw_ostream &OS;
};

class PPFrontendActionFactory : public tooling::FrontendActionFactory 
{
public:
    PPFrontendActionFactory(const FilterType &filters, raw_ostream &OS)
        : filters(filters), OS(OS) {}

    std::unique_ptr<FrontendAction> create() override {
        return std::make_unique<PPFrontendAction>(filters, OS);
    }

private:
    const FilterType &filters;
    raw_ostream &OS;
};
} // namespace
} // namespace pp_divider
} // namespace clang

#endif  // PP_FRONTEND_ACTION_H