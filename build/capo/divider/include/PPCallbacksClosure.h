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

using FilterType = std::vector<std::pair<llvm::GlobPattern, bool>>;

class PPCallbacksClosure : public PPCallbacks 
{
public:
    PPCallbacksClosure(const FilterType &filters, Preprocessor &PP);
    ~PPCallbacksClosure() override;

    // callback
    void PragmaDirective(SourceLocation Loc, PragmaIntroducerKind Introducer) override;

    llvm::StringRef getSourceString(CharSourceRange Range);

    // List of (Glob,Enabled) pairs used to filter callbacks.
    const FilterType &filters;

    Preprocessor &PP;
};

} // namespace pp_divider
} // namespace clang

#endif // PP_CALLBACKS_H
