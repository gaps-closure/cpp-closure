#include "clang/Basic/FileManager.h"
#include "clang/Lex/MacroArgs.h"
#include "llvm/Support/raw_ostream.h"

#include "PPCallbacksClosure.h"
#include "Matcher.h"

namespace clang {
namespace pp_divider {

// Get a "file:line:column" source location string.
std::string getSourceLocationString(Preprocessor &PP, SourceLocation Loc) 
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

PPCallbacksClosure::PPCallbacksClosure(const FilterType &filters,
                                       Preprocessor &PP)
    : filters(filters), PP(PP) 
{
}

PPCallbacksClosure::~PPCallbacksClosure()
{
}

// invoked when start reading any pragma directive.
void PPCallbacksClosure::PragmaDirective(SourceLocation Loc,
                                         PragmaIntroducerKind Introducer)
{
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

// Get the raw source string of the range.
llvm::StringRef PPCallbacksClosure::getSourceString(CharSourceRange range) 
{
    const char *B = PP.getSourceManager().getCharacterData(range.getBegin());
    const char *E = PP.getSourceManager().getCharacterData(range.getEnd());

    return llvm::StringRef(B, E - B);
}

} // namespace pp_divider
} // namespace clang
