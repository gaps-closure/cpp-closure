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
        ci(ci) {}

    void HandleTranslationUnit(clang::ASTContext& ctx) override {
    }

    bool HandleTopLevelDecl(DeclGroupRef dg) override {
        for(auto decl : dg) {
            pg.add_top_level_decl(decl);
        }
        return true;
    }


    ~Consumer() {
        auto tbl = pg.node_table();
        std::ofstream node_csv;
        node_csv.open("nodes.csv");
        tbl.output_csv(node_csv, ",", "\n");
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

};

static ParsedAttrInfoRegistry::Add<cle::AttrInfo> Y("cle", "cle annotator");

static FrontendPluginRegistry::Add<cle::Action> X("cle", "cle annotation codegen");