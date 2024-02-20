#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Sema/Sema.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/Sema/ParsedAttr.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "llvm/IR/Attributes.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Attributes.h"

#include "Table.h"
#include "Graph.h"
#include "PGraph.h"

using namespace clang;


namespace cle {

std::vector<std::tuple<std::string, std::string>> label_pairs;
void output_label_pairs() {
    std::ofstream node_csv;
    node_csv.open("collated.json");
    node_csv << "[";
    for(size_t i = 0; i < label_pairs.size(); i++) {
        auto [label, json] = label_pairs[i];
        node_csv << "{" << "\"cle-label\": \"" << label << "\","; 
        node_csv << "\"cle-json\":" << json << "}";
        if(i != label_pairs.size() - 1)
            node_csv << ",\n";
    }
    node_csv << "]";
    node_csv.close();
}
 
struct AttrInfo : public ParsedAttrInfo {
    AttrInfo() {
        NumArgs = 1;
        static constexpr Spelling S[] = {{ParsedAttr::AS_GNU, "cle_annotate"},
                                        {ParsedAttr::AS_C2x, "cle_annotate"},
                                        {ParsedAttr::AS_CXX11, "cle_annotate"},
                                        {ParsedAttr::AS_CXX11, "cle::annotate"}};
        Spellings = S;
    }

    AttrHandling handleDeclAttribute(Sema &sema, Decl *decl, const ParsedAttr &attr) const override {
        llvm::StringRef name;
        if(auto lit = dyn_cast<StringLiteral>(attr.getArgAsExpr(0))) {
            name = lit->getBytes();
        } else {
            unsigned id = sema.getDiagnostics().getCustomDiagID(
            DiagnosticsEngine::Error, "first argument to the 'cle_annotate'"
                                      "attribute must be a string literal");
            sema.Diag(attr.getLoc(), id);
            return AttributeNotApplied;
        }
        decl->addAttr(clang::AnnotateAttr::Create(sema.Context, name, AttributeCommonInfo(decl->getSourceRange())));
        return AttributeApplied;
    }
};

class Consumer : public ASTConsumer {
private:
    CompilerInstance& ci;
    cle::pgraph::Graph pg;
public:

    Consumer(CompilerInstance& ci) : 
        ci(ci), pg(&ci.getASTContext()) {}

    void HandleTranslationUnit(clang::ASTContext& ctx) override {
    }

    bool HandleTopLevelDecl(DeclGroupRef dg) override {
        for(auto decl : dg) {
            pg.add_decl(decl);
        }
        return true;
    }


    ~Consumer() {
        auto ntbl = pg.node_table();
        std::ofstream node_csv;
        node_csv.open("nodes.csv");
        ntbl.output_csv(node_csv, ",", "\n");
        node_csv.close();

        auto etbl = pg.edge_table();
        std::ofstream edge_csv;
        edge_csv.open("edges.csv");
        etbl.output_csv(edge_csv, ",", "\n");
        edge_csv.close();

        output_label_pairs();
    }
};
    

class Action : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& ci, llvm::StringRef) override {
        return std::make_unique<Consumer>(ci);
    }

    bool ParseArgs(const CompilerInstance &ci, const std::vector<std::string> &args) override {
        return true;
    }
};


class Handler : public PragmaHandler {
public:
  Handler() : PragmaHandler("cle") { }
  void HandlePragma(Preprocessor &pp, PragmaIntroducer intro, Token &tok) override {
    // Handle the pragma
    pp.Lex(tok);
    if(pp.getSpelling(tok) != "def") {
        pp.Diag(tok, diag::err_expected) << "unknown cle directive";
    }
    pp.Lex(tok);
    std::string label = pp.getSpelling(tok);
    pp.Lex(tok);
    std::string json;
    size_t count = 0;
    std::string tok_str = pp.getSpelling(tok);
    bool init = true;
    while(init || count > 0) {
        if(tok_str == "{")
            count++;
        if(tok_str == "}")
            count--;
        json += tok_str; 
        pp.Lex(tok);
        if(init)
            init = false;
        tok_str = pp.getSpelling(tok);
    }
    label_pairs.push_back(std::tuple(label, json));
  }
 
};


};
static PragmaHandlerRegistry::Add<cle::Handler> Z("cle", "cle pragma handler");

static ParsedAttrInfoRegistry::Add<cle::AttrInfo> Y("cle", "cle annotator");

static FrontendPluginRegistry::Add<cle::Action> X("cle", "cle annotation codegen");