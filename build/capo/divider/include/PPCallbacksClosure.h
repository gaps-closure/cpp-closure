#ifndef PP_CALLBACKS_CLOSURE_H
#define PP_CALLBACKS_CLOSURE_H

#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/GlobPattern.h"
#include <string>
#include <vector>

namespace clang {
namespace pp_divider {

// This struct represents one callback function argument by name and value.
struct Argument 
{
    std::string Name;
    std::string Value;
};

/// This class represents one callback call by name and an array of arguments.
class CallbackCall 
{
public:
    CallbackCall(llvm::StringRef Name) : Name(Name) {}
    CallbackCall() = default;

    std::string Name;
    std::vector<Argument> Arguments;
};

using FilterType = std::vector<std::pair<llvm::GlobPattern, bool>>;

class PPCallbacksClosure : public PPCallbacks 
{
public:
    PPCallbacksClosure(const FilterType &Filters,
                        std::vector<CallbackCall> &CallbackCalls,
                        Preprocessor &PP);
    ~PPCallbacksClosure() override;

    // Overridden callback functions.
    void PragmaDirective(SourceLocation Loc, PragmaIntroducerKind Introducer) override;

    // Helper functions.

    /// Get the raw source string of the range.
    llvm::StringRef getSourceString(CharSourceRange Range);

    /// Callback trace information.
    /// We use a reference so the trace will be preserved for the caller
    /// after this object is destructed.
    std::vector<CallbackCall> &CallbackCalls;

    // List of (Glob,Enabled) pairs used to filter callbacks.
    const FilterType &Filters;

    // Whether a callback should be printed.
    llvm::StringMap<bool> CallbackIsEnabled;

    /// Inhibit trace while this is set.
    bool DisableTrace;

    Preprocessor &PP;
};

} // namespace pp_divider
} // namespace clang

#endif // PP_CALLBACKS_H
