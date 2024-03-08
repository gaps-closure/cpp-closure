#ifndef PP_FRONTEND_H
#define PP_FRONTEND_H

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
    PPFrontendAction(const FilterType &Filters, raw_ostream &OS)
        : Filters(Filters), OS(OS) {}

protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                    StringRef InFile) override {
        Preprocessor &PP = CI.getPreprocessor();
        PP.addPPCallbacks(std::make_unique<PPCallbacksClosure>(Filters, PP));

        return std::make_unique<ASTConsumer>();
    }

    bool BeginSourceFileAction(CompilerInstance &ci) override {
        // std::unique_ptr<Find_Includes> find_includes_callback(new Find_Includes());

        // Preprocessor &pp = ci.getPreprocessor();
        // pp.addPPCallbacks(std::move(find_includes_callback));
        return true;
    }

    void EndSourceFileAction() override {
    }

private:
    const FilterType &Filters;
    raw_ostream &OS;
};

class PPFrontendActionFactory : public tooling::FrontendActionFactory 
{
public:
    PPFrontendActionFactory(const FilterType &Filters, raw_ostream &OS)
        : Filters(Filters), OS(OS) {}

    std::unique_ptr<FrontendAction> create() override {
        return std::make_unique<PPFrontendAction>(Filters, OS);
    }

private:
    const FilterType &Filters;
    raw_ostream &OS;
};
} // namespace
} // namespace pp_divider
} // namespace clang

#endif