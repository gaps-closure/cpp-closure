#include "clang/Basic/FileManager.h"
#include "clang/Lex/MacroArgs.h"
#include "llvm/Support/raw_ostream.h"

#include "PPCallbacksClosure.h"
#include "Matcher.h"

namespace clang {
namespace pp_divider {

// Get a "file:line:column" source location string.
static std::string getSourceLocationString(Preprocessor &PP, SourceLocation Loc) 
{
    if (Loc.isInvalid())
        return std::string("(none)");

    if (Loc.isFileID()) {
        PresumedLoc PLoc = PP.getSourceManager().getPresumedLoc(Loc);

        if (PLoc.isInvalid()) {
            return std::string("(invalid)");
        }

        std::string Str;
        llvm::raw_string_ostream SS(Str);

        // The macro expansion and spelling pos is identical for file locs.
        SS << "\"" 
           << PLoc.getFilename() << ':' 
           << PLoc.getLine() << ':'
           << PLoc.getColumn() << "\"";

        std::string result = SS.str();
        // YAML treats backslash as escape, so use forward slashes.
        std::replace(result.begin(), result.end(), '\\', '/');

        return result;
    }
    return std::string("(nonfile)");
}

static const char *const FileChangeReasonStrings[] = {
    "EnterFile", "ExitFile", "SystemHeaderPragma", "RenameFile"
};

static const char *const CharacteristicKindStrings[] = { 
    "C_User", "C_System", "C_ExternCSystem" 
};

static const char *const MacroDirectiveKindStrings[] = {
    "MD_Define","MD_Undefine", "MD_Visibility"
};

static const char *const PragmaIntroducerKindStrings[] = {
    "PIK_HashPragma", "PIK__Pragma", "PIK___pragma" };

static const char *const PragmaMessageKindStrings[] = {
    "PMK_Message", "PMK_Warning", "PMK_Error"
};

static const char *const PragmaWarningSpecifierStrings[] = {
    "PWS_Default", "PWS_Disable", "PWS_Error",  "PWS_Once",   "PWS_Suppress",
    "PWS_Level1",  "PWS_Level2",  "PWS_Level3", "PWS_Level4",
};

static const char *const ConditionValueKindStrings[] = {
    "CVK_NotEvaluated", "CVK_False", "CVK_True"
};

static const char *const MappingStrings[] = { 
    "0", "MAP_IGNORE", "MAP_REMARK", "MAP_WARNING", "MAP_ERROR",  "MAP_FATAL" 
};

PPCallbacksClosure::PPCallbacksClosure(const FilterType &Filters,
                                       std::vector<CallbackCall> &CallbackCalls,
                                       Preprocessor &PP)
    : CallbackCalls(CallbackCalls), Filters(Filters), PP(PP) 
{
}

PPCallbacksClosure::~PPCallbacksClosure()
{
}

// Callback invoked whenever a source file is entered or exited.
void PPCallbacksClosure::FileChanged(SourceLocation Loc,
                                     PPCallbacks::FileChangeReason Reason,
                                     SrcMgr::CharacteristicKind FileType,
                                     FileID PrevFID) {
  beginCallback("FileChanged");
  appendArgument("Loc", Loc);
  appendArgument("Reason", Reason, FileChangeReasonStrings);
  appendArgument("FileType", FileType, CharacteristicKindStrings);
  appendArgument("PrevFID", PrevFID);
}

// Callback invoked whenever a source file is skipped as the result
// of header guard optimization.
void PPCallbacksClosure::FileSkipped(const FileEntryRef &SkippedFile,
                                     const Token &FilenameTok,
                                     SrcMgr::CharacteristicKind FileType) {
  beginCallback("FileSkipped");
  appendArgument("ParentFile", &SkippedFile.getFileEntry());
  appendArgument("FilenameTok", FilenameTok);
  appendArgument("FileType", FileType, CharacteristicKindStrings);
}

// Callback invoked whenever an inclusion directive results in a
// file-not-found error.
bool
PPCallbacksClosure::FileNotFound(llvm::StringRef FileName,
                                 llvm::SmallVectorImpl<char> &RecoveryPath) {
  beginCallback("FileNotFound");
  appendFilePathArgument("FileName", FileName);
  return false;
}

// Callback invoked whenever an inclusion directive of
// any kind (#include, #import, etc.) has been processed, regardless
// of whether the inclusion will actually result in an inclusion.
void PPCallbacksClosure::InclusionDirective(
    SourceLocation HashLoc, const Token &IncludeTok, llvm::StringRef FileName,
    bool IsAngled, CharSourceRange FilenameRange, const FileEntry *File,
    llvm::StringRef SearchPath, llvm::StringRef RelativePath,
    const Module *Imported, SrcMgr::CharacteristicKind FileType) {
  beginCallback("InclusionDirective");
  appendArgument("IncludeTok", IncludeTok);
  appendFilePathArgument("FileName", FileName);
  appendArgument("IsAngled", IsAngled);
  appendArgument("FilenameRange", FilenameRange);
  appendArgument("File", File);
  appendFilePathArgument("SearchPath", SearchPath);
  appendFilePathArgument("RelativePath", RelativePath);
  appendArgument("Imported", Imported);
}

// Callback invoked whenever there was an explicit module-import
// syntax.
void PPCallbacksClosure::moduleImport(SourceLocation ImportLoc,
                                      ModuleIdPath Path,
                                      const Module *Imported) {
  beginCallback("moduleImport");
  appendArgument("ImportLoc", ImportLoc);
  appendArgument("Path", Path);
  appendArgument("Imported", Imported);
}

// Callback invoked when the end of the main file is reached.
// No subsequent callbacks will be made.
void PPCallbacksClosure::EndOfMainFile() { beginCallback("EndOfMainFile"); }

// Callback invoked when a #ident or #sccs directive is read.
void PPCallbacksClosure::Ident(SourceLocation Loc, llvm::StringRef Str) {
  beginCallback("Ident");
  appendArgument("Loc", Loc);
  appendArgument("Str", Str);
}

// Callback invoked when start reading any pragma directive.
void PPCallbacksClosure::PragmaDirective(SourceLocation Loc,
                                         PragmaIntroducerKind Introducer)
{
    beginCallback("PragmaDirective");

    SourceManager &sm = PP.getSourceManager();
    unsigned line = sm.getExpansionLineNumber(Loc);
    SourceLocation line_after = sm.translateLineCol(sm.getMainFileID(), line + 1, 1);
    SourceRange range(Loc, line_after);
    CharSourceRange csr(range, false);

    std::string pragma = getSourceString(csr).str();
    if (pragma.rfind("#pragma cle begin ", 0) == 0) {
        Matcher::addCleRangeOpen(range, pragma);
    }
    else if (pragma.rfind("#pragma cle end ", 0) == 0) {
        Matcher::addCleRangeClose(range, pragma);
    }
}

// Callback invoked when a #pragma comment directive is read.
void PPCallbacksClosure::PragmaComment(SourceLocation Loc,
                                       const IdentifierInfo *Kind,
                                       llvm::StringRef Str) {
  beginCallback("PragmaComment");
  appendArgument("Loc", Loc);
  appendArgument("Kind", Kind);
  appendArgument("Str", Str);
}

// Callback invoked when a #pragma detect_mismatch directive is
// read.
void PPCallbacksClosure::PragmaDetectMismatch(SourceLocation Loc,
                                              llvm::StringRef Name,
                                              llvm::StringRef Value) {
  beginCallback("PragmaDetectMismatch");
  appendArgument("Loc", Loc);
  appendArgument("Name", Name);
  appendArgument("Value", Value);
}

// Callback invoked when a #pragma clang __debug directive is read.
void PPCallbacksClosure::PragmaDebug(SourceLocation Loc,
                                     llvm::StringRef DebugType) {
  beginCallback("PragmaDebug");
  appendArgument("Loc", Loc);
  appendArgument("DebugType", DebugType);
}

// Callback invoked when a #pragma message directive is read.
void PPCallbacksClosure::PragmaMessage(SourceLocation Loc,
                                       llvm::StringRef Namespace,
                                       PPCallbacks::PragmaMessageKind Kind,
                                       llvm::StringRef Str) {
  beginCallback("PragmaMessage");
  appendArgument("Loc", Loc);
  appendArgument("Namespace", Namespace);
  appendArgument("Kind", Kind, PragmaMessageKindStrings);
  appendArgument("Str", Str);
  exit(0);
}

// Callback invoked when a #pragma gcc diagnostic push directive
// is read.
void PPCallbacksClosure::PragmaDiagnosticPush(SourceLocation Loc,
                                              llvm::StringRef Namespace) {
  beginCallback("PragmaDiagnosticPush");
  appendArgument("Loc", Loc);
  appendArgument("Namespace", Namespace);
}

// Callback invoked when a #pragma gcc diagnostic pop directive
// is read.
void PPCallbacksClosure::PragmaDiagnosticPop(SourceLocation Loc,
                                             llvm::StringRef Namespace) {
  beginCallback("PragmaDiagnosticPop");
  appendArgument("Loc", Loc);
  appendArgument("Namespace", Namespace);
}

// Callback invoked when a #pragma gcc diagnostic directive is read.
void PPCallbacksClosure::PragmaDiagnostic(SourceLocation Loc,
                                          llvm::StringRef Namespace,
                                          diag::Severity Mapping,
                                          llvm::StringRef Str) {
  beginCallback("PragmaDiagnostic");
  appendArgument("Loc", Loc);
  appendArgument("Namespace", Namespace);
  appendArgument("Mapping", (unsigned)Mapping, MappingStrings);
  appendArgument("Str", Str);
}

// Called when an OpenCL extension is either disabled or
// enabled with a pragma.
void PPCallbacksClosure::PragmaOpenCLExtension(SourceLocation NameLoc,
                                               const IdentifierInfo *Name,
                                               SourceLocation StateLoc,
                                               unsigned State) {
  beginCallback("PragmaOpenCLExtension");
  appendArgument("NameLoc", NameLoc);
  appendArgument("Name", Name);
  appendArgument("StateLoc", StateLoc);
  appendArgument("State", (int)State);
}

// Callback invoked when a #pragma warning directive is read.
void PPCallbacksClosure::PragmaWarning(SourceLocation Loc,
                                       PragmaWarningSpecifier WarningSpec,
                                       llvm::ArrayRef<int> Ids) {
  beginCallback("PragmaWarning");
  appendArgument("Loc", Loc);
  appendArgument("WarningSpec", WarningSpec, PragmaWarningSpecifierStrings);

  std::string Str;
  llvm::raw_string_ostream SS(Str);
  SS << "[";
  for (int i = 0, e = Ids.size(); i != e; ++i) {
    if (i)
      SS << ", ";
    SS << Ids[i];
  }
  SS << "]";
  appendArgument("Ids", SS.str());
}

// Callback invoked when a #pragma warning(push) directive is read.
void PPCallbacksClosure::PragmaWarningPush(SourceLocation Loc, int Level) {
  beginCallback("PragmaWarningPush");
  appendArgument("Loc", Loc);
  appendArgument("Level", Level);
}

// Callback invoked when a #pragma warning(pop) directive is read.
void PPCallbacksClosure::PragmaWarningPop(SourceLocation Loc) {
  beginCallback("PragmaWarningPop");
  appendArgument("Loc", Loc);
}

// Callback invoked when a #pragma execution_character_set(push) directive
// is read.
void PPCallbacksClosure::PragmaExecCharsetPush(SourceLocation Loc,
                                               StringRef Str) {
  beginCallback("PragmaExecCharsetPush");
  appendArgument("Loc", Loc);
  appendArgument("Charset", Str);
}

// Callback invoked when a #pragma execution_character_set(pop) directive
// is read.
void PPCallbacksClosure::PragmaExecCharsetPop(SourceLocation Loc) {
  beginCallback("PragmaExecCharsetPop");
  appendArgument("Loc", Loc);
}

// Called by Preprocessor::HandleMacroExpandedIdentifier when a
// macro invocation is found.
void PPCallbacksClosure::MacroExpands(const Token &MacroNameTok,
                                      const MacroDefinition &MacroDefinition,
                                      SourceRange Range,
                                      const MacroArgs *Args) {
  beginCallback("MacroExpands");
  appendArgument("MacroNameTok", MacroNameTok);
  appendArgument("MacroDefinition", MacroDefinition);
  appendArgument("Range", Range);
  appendArgument("Args", Args);
}

// Hook called whenever a macro definition is seen.
void PPCallbacksClosure::MacroDefined(const Token &MacroNameTok,
                                      const MacroDirective *MacroDirective) {
  beginCallback("MacroDefined");
  appendArgument("MacroNameTok", MacroNameTok);
  appendArgument("MacroDirective", MacroDirective);
}

// Hook called whenever a macro #undef is seen.
void PPCallbacksClosure::MacroUndefined(const Token &MacroNameTok,
                                        const MacroDefinition &MacroDefinition,
                                        const MacroDirective *Undef) {
  beginCallback("MacroUndefined");
  appendArgument("MacroNameTok", MacroNameTok);
  appendArgument("MacroDefinition", MacroDefinition);
}

// Hook called whenever the 'defined' operator is seen.
void PPCallbacksClosure::Defined(const Token &MacroNameTok,
                                 const MacroDefinition &MacroDefinition,
                                 SourceRange Range) {
  beginCallback("Defined");
  appendArgument("MacroNameTok", MacroNameTok);
  appendArgument("MacroDefinition", MacroDefinition);
  appendArgument("Range", Range);
}

// Hook called when a source range is skipped.
void PPCallbacksClosure::SourceRangeSkipped(SourceRange Range,
                                            SourceLocation EndifLoc) {
  beginCallback("SourceRangeSkipped");
  appendArgument("Range", SourceRange(Range.getBegin(), EndifLoc));
}

// Hook called whenever an #if is seen.
void PPCallbacksClosure::If(SourceLocation Loc, SourceRange ConditionRange,
                            ConditionValueKind ConditionValue) {
  beginCallback("If");
  appendArgument("Loc", Loc);
  appendArgument("ConditionRange", ConditionRange);
  appendArgument("ConditionValue", ConditionValue, ConditionValueKindStrings);
}

// Hook called whenever an #elif is seen.
void PPCallbacksClosure::Elif(SourceLocation Loc, SourceRange ConditionRange,
                              ConditionValueKind ConditionValue,
                              SourceLocation IfLoc) {
  beginCallback("Elif");
  appendArgument("Loc", Loc);
  appendArgument("ConditionRange", ConditionRange);
  appendArgument("ConditionValue", ConditionValue, ConditionValueKindStrings);
  appendArgument("IfLoc", IfLoc);
}

// Hook called whenever an #ifdef is seen.
void PPCallbacksClosure::Ifdef(SourceLocation Loc, const Token &MacroNameTok,
                               const MacroDefinition &MacroDefinition) {
  beginCallback("Ifdef");
  appendArgument("Loc", Loc);
  appendArgument("MacroNameTok", MacroNameTok);
  appendArgument("MacroDefinition", MacroDefinition);
}

// Hook called whenever an #ifndef is seen.
void PPCallbacksClosure::Ifndef(SourceLocation Loc, const Token &MacroNameTok,
                                const MacroDefinition &MacroDefinition) {
  beginCallback("Ifndef");
  appendArgument("Loc", Loc);
  appendArgument("MacroNameTok", MacroNameTok);
  appendArgument("MacroDefinition", MacroDefinition);
}

// Hook called whenever an #else is seen.
void PPCallbacksClosure::Else(SourceLocation Loc, SourceLocation IfLoc) {
  beginCallback("Else");
  appendArgument("Loc", Loc);
  appendArgument("IfLoc", IfLoc);
}

// Hook called whenever an #endif is seen.
void PPCallbacksClosure::Endif(SourceLocation Loc, SourceLocation IfLoc) {
  beginCallback("Endif");
  appendArgument("Loc", Loc);
  appendArgument("IfLoc", IfLoc);
}

// Helper functions.

// Start a new callback.
void PPCallbacksClosure::beginCallback(const char *Name) 
{
    auto R = CallbackIsEnabled.try_emplace(Name, false);
    if (R.second) {
        llvm::StringRef N(Name);
        for (const std::pair<llvm::GlobPattern, bool> &Filter : Filters)
            if (Filter.first.match(N))
                R.first->second = Filter.second;
    }
    DisableTrace = !R.first->second;
    if (DisableTrace)
        return;
    CallbackCalls.push_back(CallbackCall(Name));
}

// Append a bool argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name, bool Value) {
  appendArgument(Name, (Value ? "true" : "false"));
}

// Append an int argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name, int Value) {
  std::string Str;
  llvm::raw_string_ostream SS(Str);
  SS << Value;
  appendArgument(Name, SS.str());
}

// Append a string argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name, const char *Value) {
  if (DisableTrace)
    return;
  CallbackCalls.back().Arguments.push_back(Argument{Name, Value});
}

// Append a string object argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name,
                                        llvm::StringRef Value) {
  appendArgument(Name, Value.str());
}

// Append a string object argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name,
                                        const std::string &Value) {
  appendArgument(Name, Value.c_str());
}

// Append a token argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name, const Token &Value) {
  appendArgument(Name, PP.getSpelling(Value));
}

// Append an enum argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name, int Value,
                                        const char *const Strings[]) {
  appendArgument(Name, Strings[Value]);
}

// Append a FileID argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name, FileID Value) {
  if (Value.isInvalid()) {
    appendArgument(Name, "(invalid)");
    return;
  }
  const FileEntry *FileEntry = PP.getSourceManager().getFileEntryForID(Value);
  if (!FileEntry) {
    appendArgument(Name, "(getFileEntryForID failed)");
    return;
  }
  appendFilePathArgument(Name, FileEntry->getName());
}

// Append a FileEntry argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name,
                                        const FileEntry *Value) {
  if (!Value) {
    appendArgument(Name, "(null)");
    return;
  }
  appendFilePathArgument(Name, Value->getName());
}

// Append a SourceLocation argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name,
                                        SourceLocation Value) {
  if (Value.isInvalid()) {
    appendArgument(Name, "(invalid)");
    return;
  }
  appendArgument(Name, getSourceLocationString(PP, Value).c_str());
}

// Append a SourceRange argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name, SourceRange Value) {
  if (DisableTrace)
    return;
  if (Value.isInvalid()) {
    appendArgument(Name, "(invalid)");
    return;
  }
  std::string Str;
  llvm::raw_string_ostream SS(Str);
  SS << "[" << getSourceLocationString(PP, Value.getBegin()) << ", "
     << getSourceLocationString(PP, Value.getEnd()) << "]";
  appendArgument(Name, SS.str());
}

// Append a CharSourceRange argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name,
                                        CharSourceRange Value) {
  if (Value.isInvalid()) {
    appendArgument(Name, "(invalid)");
    return;
  }
  appendArgument(Name, getSourceString(Value).str().c_str());
}

// Append a SourceLocation argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name, ModuleIdPath Value) {
  if (DisableTrace)
    return;
  std::string Str;
  llvm::raw_string_ostream SS(Str);
  SS << "[";
  for (int I = 0, E = Value.size(); I != E; ++I) {
    if (I)
      SS << ", ";
    SS << "{"
       << "Name: " << Value[I].first->getName() << ", "
       << "Loc: " << getSourceLocationString(PP, Value[I].second) << "}";
  }
  SS << "]";
  appendArgument(Name, SS.str());
}

// Append an IdentifierInfo argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name,
                                        const IdentifierInfo *Value) {
  if (!Value) {
    appendArgument(Name, "(null)");
    return;
  }
  appendArgument(Name, Value->getName().str().c_str());
}

// Append a MacroDirective argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name,
                                        const MacroDirective *Value) {
  if (!Value) {
    appendArgument(Name, "(null)");
    return;
  }
  appendArgument(Name, MacroDirectiveKindStrings[Value->getKind()]);
}

// Append a MacroDefinition argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name,
                                        const MacroDefinition &Value) {
  std::string Str;
  llvm::raw_string_ostream SS(Str);
  SS << "[";
  bool Any = false;
  if (Value.getLocalDirective()) {
    SS << "(local)";
    Any = true;
  }
  for (auto *MM : Value.getModuleMacros()) {
    if (Any) SS << ", ";
    SS << MM->getOwningModule()->getFullModuleName();
  }
  SS << "]";
  appendArgument(Name, SS.str());
}

// Append a MacroArgs argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name,
                                        const MacroArgs *Value) {
  if (!Value) {
    appendArgument(Name, "(null)");
    return;
  }
  std::string Str;
  llvm::raw_string_ostream SS(Str);
  SS << "[";

  // Each argument is is a series of contiguous Tokens, terminated by a eof.
  // Go through each argument printing tokens until we reach eof.
  for (unsigned I = 0; I < Value->getNumMacroArguments(); ++I) {
    const Token *Current = Value->getUnexpArgument(I);
    if (I)
      SS << ", ";
    bool First = true;
    while (Current->isNot(tok::eof)) {
      if (!First)
        SS << " ";
      // We need to be careful here because the arguments might not be legal in
      // YAML, so we use the token name for anything but identifiers and
      // numeric literals.
      if (Current->isAnyIdentifier() || Current->is(tok::numeric_constant)) {
        SS << PP.getSpelling(*Current);
      } else {
        SS << "<" << Current->getName() << ">";
      }
      ++Current;
      First = false;
    }
  }
  SS << "]";
  appendArgument(Name, SS.str());
}

// Append a Module argument to the top trace item.
void PPCallbacksClosure::appendArgument(const char *Name, const Module *Value) {
  if (!Value) {
    appendArgument(Name, "(null)");
    return;
  }
  appendArgument(Name, Value->Name.c_str());
}

// Append a double-quoted argument to the top trace item.
void PPCallbacksClosure::appendQuotedArgument(const char *Name,
                                              const std::string &Value) {
  std::string Str;
  llvm::raw_string_ostream SS(Str);
  SS << "\"" << Value << "\"";
  appendArgument(Name, SS.str());
}

// Append a double-quoted file path argument to the top trace item.
void PPCallbacksClosure::appendFilePathArgument(const char *Name,
                                                llvm::StringRef Value) {
  std::string Path(Value);
  // YAML treats backslash as escape, so use forward slashes.
  std::replace(Path.begin(), Path.end(), '\\', '/');
  appendQuotedArgument(Name, Path);
}

// Get the raw source string of the range.
llvm::StringRef PPCallbacksClosure::getSourceString(CharSourceRange Range) 
{
    const char *B = PP.getSourceManager().getCharacterData(Range.getBegin());
    const char *E = PP.getSourceManager().getCharacterData(Range.getEnd());
    
    return llvm::StringRef(B, E - B);
}

} // namespace pp_divider
} // namespace clang
