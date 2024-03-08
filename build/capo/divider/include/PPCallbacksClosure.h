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

    void FileChanged(SourceLocation Loc, PPCallbacks::FileChangeReason Reason,
                    SrcMgr::CharacteristicKind FileType,
                    FileID PrevFID = FileID()) override;
    void FileSkipped(const FileEntryRef &SkippedFile, const Token &FilenameTok,
                    SrcMgr::CharacteristicKind FileType) override;
    bool FileNotFound(llvm::StringRef FileName,
                        llvm::SmallVectorImpl<char> &RecoveryPath);// override;
    void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                            llvm::StringRef FileName, bool IsAngled,
                            CharSourceRange FilenameRange, const FileEntry *File,
                            llvm::StringRef SearchPath,
                            llvm::StringRef RelativePath, const Module *Imported,
                            SrcMgr::CharacteristicKind FileType); // override;
    void moduleImport(SourceLocation ImportLoc, ModuleIdPath Path,
                        const Module *Imported) override;
    void EndOfMainFile() override;
    void Ident(SourceLocation Loc, llvm::StringRef str) override;
    void PragmaDirective(SourceLocation Loc,
                        PragmaIntroducerKind Introducer) override;
    void PragmaComment(SourceLocation Loc, const IdentifierInfo *Kind,
                        llvm::StringRef Str) override;
    void PragmaDetectMismatch(SourceLocation Loc, llvm::StringRef Name,
                                llvm::StringRef Value) override;
    void PragmaDebug(SourceLocation Loc, llvm::StringRef DebugType) override;
    void PragmaMessage(SourceLocation Loc, llvm::StringRef Namespace,
                        PPCallbacks::PragmaMessageKind Kind,
                        llvm::StringRef Str) override;
    void PragmaDiagnosticPush(SourceLocation Loc,
                                llvm::StringRef Namespace) override;
    void PragmaDiagnosticPop(SourceLocation Loc,
                            llvm::StringRef Namespace) override;
    void PragmaDiagnostic(SourceLocation Loc, llvm::StringRef Namespace,
                            diag::Severity mapping, llvm::StringRef Str) override;
    void PragmaOpenCLExtension(SourceLocation NameLoc, const IdentifierInfo *Name,
                                SourceLocation StateLoc, unsigned State) override;
    void PragmaWarning(SourceLocation Loc, PragmaWarningSpecifier WarningSpec,
                        llvm::ArrayRef<int> Ids) override;
    void PragmaWarningPush(SourceLocation Loc, int Level) override;
    void PragmaWarningPop(SourceLocation Loc) override;
    void PragmaExecCharsetPush(SourceLocation Loc, StringRef Str) override;
    void PragmaExecCharsetPop(SourceLocation Loc) override;
    void MacroExpands(const Token &MacroNameTok, const MacroDefinition &MD,
                        SourceRange Range, const MacroArgs *Args) override;
    void MacroDefined(const Token &MacroNameTok,
                        const MacroDirective *MD) override;
    void MacroUndefined(const Token &MacroNameTok, const MacroDefinition &MD,
                        const MacroDirective *Undef) override;
    void Defined(const Token &MacroNameTok, const MacroDefinition &MD,
                SourceRange Range) override;
    void SourceRangeSkipped(SourceRange Range, SourceLocation EndifLoc) override;
    void If(SourceLocation Loc, SourceRange ConditionRange,
            ConditionValueKind ConditionValue) override;
    void Elif(SourceLocation Loc, SourceRange ConditionRange,
                ConditionValueKind ConditionValue, SourceLocation IfLoc) override;
    void Ifdef(SourceLocation Loc, const Token &MacroNameTok,
                const MacroDefinition &MD) override;
    void Ifndef(SourceLocation Loc, const Token &MacroNameTok,
                const MacroDefinition &MD) override;
    void Else(SourceLocation Loc, SourceLocation IfLoc) override;
    void Endif(SourceLocation Loc, SourceLocation IfLoc) override;

    // Helper functions.

    /// Start a new callback.
    void beginCallback(const char *Name);

    void append(const char *Str);
    void appendArgument(const char *Name, bool Value);
    void appendArgument(const char *Name, int Value);
    void appendArgument(const char *Name, const char *Value);
    void appendArgument(const char *Name, llvm::StringRef Value);
    void appendArgument(const char *Name, const std::string &Value);
    void appendArgument(const char *Name, const Token &Value);
    void appendArgument(const char *Name, int Value, const char *const Strings[]);
    void appendArgument(const char *Name, FileID Value);
    void appendArgument(const char *Name, const FileEntry *Value);
    void appendArgument(const char *Name, SourceLocation Value);
    void appendArgument(const char *Name, SourceRange Value);
    void appendArgument(const char *Name, CharSourceRange Value);
    void appendArgument(const char *Name, ModuleIdPath Value);
    void appendArgument(const char *Name, const IdentifierInfo *Value);
    void appendArgument(const char *Name, const MacroDirective *Value);
    void appendArgument(const char *Name, const MacroDefinition &Value);
    void appendArgument(const char *Name, const MacroArgs *Value);
    void appendArgument(const char *Name, const Module *Value);
    void appendQuotedArgument(const char *Name, const std::string &Value);
    void appendFilePathArgument(const char *Name, llvm::StringRef Value);

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
