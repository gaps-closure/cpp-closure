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

using namespace clang;

namespace {
struct CLEAttrInfo : public ParsedAttrInfo {
    CLEAttrInfo() {
        NumArgs = 1;
        static constexpr Spelling S[] = {{ParsedAttr::AS_GNU, "cle_annotate"},
                                        {ParsedAttr::AS_C2x, "cle_annotate"},
                                        {ParsedAttr::AS_CXX11, "cle_annotate"},
                                        {ParsedAttr::AS_CXX11, "cle::annotate"}};
        Spellings = S;

    }

    AttrHandling handleDeclAttribute(Sema &S, Decl *D,
                                   const ParsedAttr &Attr) const override {

        llvm::StringRef name;
        if(auto lit = dyn_cast<StringLiteral>(Attr.getArgAsExpr(0))) {
            name = lit->getBytes();
        } else {
            unsigned ID = S.getDiagnostics().getCustomDiagID(
            DiagnosticsEngine::Error, "first argument to the 'cle_annotate'"
                                      "attribute must be a string literal");
            S.Diag(Attr.getLoc(), ID);
            return AttributeNotApplied;
        }
        D->addAttr(clang::AnnotateAttr::Create(S.Context, name, AttributeCommonInfo(D->getSourceRange())));
        return AttributeApplied;
    }
};



class NamedClassVisitor : public RecursiveASTVisitor<NamedClassVisitor> {
private:
    CompilerInstance &CI;
public:

    NamedClassVisitor(CompilerInstance &CI) : CI(CI) {}

    bool VisitCXXRecordDecl(CXXRecordDecl *decl) {
        auto &sema = CI.getSema();
        auto &ctx = CI.getASTContext();
        auto thisType = ctx.getPointerType(ctx.getRecordType(decl));

        for(auto ctor : decl->ctors()) {
            if(!ctor->hasBody())
                continue;

            std::vector<Stmt*> stmts;

            stmts.push_back(ctor->getBody());

            auto loc = ctor->getLocation();
            auto thisExpr = sema.BuildCXXThisExpr(loc, thisType, false);

            for(auto field : decl->fields()) {
                
                auto access = 
                    sema.BuildFieldReferenceExpr(thisExpr, true, loc, 
                        CXXScopeSpec(), field, 
                        DeclAccessPair::make(decl, field->getAccess()), DeclarationNameInfo());

                stmts.push_back(access.get());

            }
            ctor->setBody(CompoundStmt::Create(ctx, stmts, loc, loc));
        }
        decl->dumpColor();        
        return true;
    }
};


class CLEConsumer : public ASTConsumer {
private:
    std::unique_ptr<llvm::LLVMContext> ctx; 
    CodeGenerator *cg; 
    NamedClassVisitor visitor;
public:

    CLEConsumer(CompilerInstance &CI) : 
        ctx(std::make_unique<llvm::LLVMContext>()),
        cg(CreateLLVMCodeGen(CI.getDiagnostics(), "cle", CI.getHeaderSearchOpts(),
            CI.getPreprocessorOpts(), CI.getCodeGenOpts(), *ctx)),
        visitor(CI) {}

    void Initialize(ASTContext &astCtx) override {
        cg->Initialize(astCtx);
    }

    void HandleTranslationUnit(clang::ASTContext &ctx) override {
        visitor.TraverseDecl(ctx.getTranslationUnitDecl());
        cg->HandleTranslationUnit(ctx);
    }

    bool HandleTopLevelDecl(DeclGroupRef DG) override {
        cg->HandleTopLevelDecl(DG);
        return true;
    }


    ~CLEConsumer() {
        cg->GetModule()->print(llvm::outs(), nullptr);
        delete cg;
    }
};
    
};

class CLEAction : public PluginASTAction {
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 llvm::StringRef) override {

    return std::make_unique<CLEConsumer>(CI);
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    return true;
  }
};


static ParsedAttrInfoRegistry::Add<CLEAttrInfo> Y("cle", "cle annotator");

static FrontendPluginRegistry::Add<CLEAction> X("cle", "cle annotation codegen");