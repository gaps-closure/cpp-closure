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
