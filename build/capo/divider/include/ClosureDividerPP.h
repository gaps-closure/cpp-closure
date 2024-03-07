
#include "PPCallbacksTracker.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/Execution.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Option/Arg.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/GlobPattern.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/WithColor.h"
#include <string>
#include <vector>

using namespace llvm;

namespace clang {
namespace pp_trace {

namespace {

class PPTraceAction : public ASTFrontendAction {
public:
    PPTraceAction(const FilterType &Filters, raw_ostream &OS)
        : Filters(Filters), OS(OS) {}

protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                    StringRef InFile) override {
        Preprocessor &PP = CI.getPreprocessor();
        PP.addPPCallbacks(std::make_unique<PPCallbacksTracker>(Filters, CallbackCalls, PP));

        return std::make_unique<ASTConsumer>();
    }

    bool BeginSourceFileAction(CompilerInstance &ci) override {
        // std::unique_ptr<Find_Includes> find_includes_callback(new Find_Includes());

        // Preprocessor &pp = ci.getPreprocessor();
        // pp.addPPCallbacks(std::move(find_includes_callback));
        return true;
    }

    void EndSourceFileAction() override {
        OS << "---\n";
        for (const CallbackCall &Callback : CallbackCalls) {
            OS << "- Callback: " << Callback.Name << "\n";
            for (const Argument &Arg : Callback.Arguments)
                OS << "  " << Arg.Name << ": " << Arg.Value << "\n";
        }
        OS << "...\n";

        CallbackCalls.clear();
    }

private:
    const FilterType &Filters;
    raw_ostream &OS;
    std::vector<CallbackCall> CallbackCalls;
};

class PPTraceFrontendActionFactory : public tooling::FrontendActionFactory 
{
public:
    PPTraceFrontendActionFactory(const FilterType &Filters, raw_ostream &OS)
        : Filters(Filters), OS(OS) {}

    std::unique_ptr<FrontendAction> create() override {
        return std::make_unique<PPTraceAction>(Filters, OS);
    }

private:
    const FilterType &Filters;
    raw_ostream &OS;
};
} // namespace
} // namespace pp_trace
} // namespace clang