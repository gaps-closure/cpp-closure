#pragma once

#include <string>

#include "Table.h"
#include "Graph.h"
#include "clang/AST/AST.h"
#include "clang/AST/Attr.h"
#include "clang/Analysis/CFG.h"
#include "clang/Basic/SourceManager.h"

namespace cle {
namespace pgraph {

using namespace clang;

enum NodeKind {
    DECL_VAR,
    DECL_FUN,
    DECL_RECORD,
    DECL_FIELD,
    DECL_METHOD,
    DECL_PARAM,
    DECL_CONSTRUCTOR,
    DECL_DESTRUCTOR,
    STMT_DECL,
    STMT_CALL,
    STMT_CONSTRUCTOR,
    STMT_IMPLICIT_DESTRUCTOR,
    STMT_COMPOUND,
    STMT_RETURN,
    STMT_REF,
    STMT_OTHER
};

std::string node_kind_name(NodeKind kind); 


struct DeclVar {
    clang::VarDecl* decl;
    DeclVar(clang::VarDecl* decl) : decl(decl) {}
};

struct DeclFun {
    clang::FunctionDecl* decl;
    DeclFun(clang::FunctionDecl* decl) : decl(decl) {}
};

struct DeclRecord {
    clang::CXXRecordDecl* decl;
    DeclRecord(clang::CXXRecordDecl* decl) : decl(decl) {}
};

struct DeclField {
    clang::FieldDecl* decl;
    DeclField(clang::FieldDecl* decl) : decl(decl) {}
};

struct DeclMethod {
    clang::CXXMethodDecl* decl;
    DeclMethod(clang::CXXMethodDecl* decl) : decl(decl) {}
};

struct DeclParam {
    clang::ParmVarDecl* decl;
    DeclParam(clang::ParmVarDecl* decl) : decl(decl) {}
};

struct DeclConstructor {
    clang::CXXConstructorDecl* decl;
    DeclConstructor(clang::CXXConstructorDecl* decl) : decl(decl) {}
};

struct DeclDestructor {
    clang::CXXDestructorDecl* decl;
    DeclDestructor(clang::CXXDestructorDecl* decl) : decl(decl) {}
};

struct StmtDecl {
    clang::DeclStmt* stmt;
    StmtDecl(clang::DeclStmt* stmt) : stmt(stmt) {}
};

struct StmtCall {
    clang::CallExpr* stmt;
    StmtCall(clang::CallExpr* stmt) : stmt(stmt) {}
};

struct StmtConstructor {
    clang::CXXConstructExpr* stmt;
    StmtConstructor(clang::CXXConstructExpr* stmt) : stmt(stmt) {}
};

struct StmtCompound {
    clang::CompoundStmt* stmt;
    StmtCompound(clang::CompoundStmt* stmt) : stmt(stmt) {}
};

struct StmtReturn {
    clang::ReturnStmt* stmt;
    StmtReturn(clang::ReturnStmt* stmt) : stmt(stmt) {}
};

struct StmtRef {
    clang::DeclRefExpr* stmt;
    StmtRef(clang::DeclRefExpr* stmt) : stmt(stmt) {}
};

struct StmtImplicitDestructor {
    clang::CFGAutomaticObjDtor dtor;
    StmtImplicitDestructor(clang::CFGAutomaticObjDtor dtor) : dtor(dtor) {}
};

struct StmtOther {
    clang::Stmt* stmt;
    StmtOther(clang::Stmt* stmt) : stmt(stmt) {}
};



class Node {
public:
    union {
        DeclVar decl_var;
        DeclFun decl_fun;
        DeclRecord decl_record;
        DeclField decl_field;
        DeclMethod decl_method;
        DeclParam decl_param;
        DeclConstructor decl_constructor;
        DeclDestructor decl_destructor;
        StmtDecl stmt_decl;
        StmtCall stmt_call;
        StmtConstructor stmt_cons;
        StmtImplicitDestructor stmt_implicit_dtor;
        StmtCompound stmt_compound;
        StmtReturn stmt_return;
        StmtRef stmt_ref;
        StmtOther stmt_other;
    };
    NodeKind kind;
    std::optional<NamedDecl*> parent_decl; 

    std::optional<std::string> qualified_name();

    std::optional<NamedDecl*> named_decl();
    clang::SourceRange source_range();

    int64_t clang_node_id(ASTContext*);

    Node(DeclVar decl_var, std::optional<NamedDecl*> parent_decl = std::nullopt) : decl_var(decl_var), kind(DECL_VAR), parent_decl(parent_decl) {}
    Node(DeclFun decl_fun, std::optional<NamedDecl*> parent_decl = std::nullopt) : decl_fun(decl_fun), kind(DECL_FUN), parent_decl(parent_decl) {}
    Node(DeclRecord decl_record, std::optional<NamedDecl*> parent_decl = std::nullopt) : decl_record(decl_record), kind(DECL_RECORD), parent_decl(parent_decl) {}
    Node(DeclField decl_field, std::optional<NamedDecl*> parent_decl = std::nullopt) : decl_field(decl_field), kind(DECL_FIELD), parent_decl(parent_decl) {}
    Node(DeclParam decl_param, std::optional<NamedDecl*> parent_decl = std::nullopt) : decl_param(decl_param), kind(DECL_PARAM), parent_decl(parent_decl) {}
    Node(DeclMethod decl_method, std::optional<NamedDecl*> parent_decl = std::nullopt) : decl_method(decl_method), kind(DECL_METHOD), parent_decl(parent_decl) {}
    Node(DeclConstructor decl_constructor, std::optional<NamedDecl*> parent_decl = std::nullopt) : decl_constructor(decl_constructor), kind(DECL_CONSTRUCTOR), parent_decl(parent_decl) {}
    Node(DeclDestructor decl_destructor, std::optional<NamedDecl*> parent_decl = std::nullopt) : decl_destructor(decl_destructor), kind(DECL_DESTRUCTOR), parent_decl(parent_decl) {}
    Node(StmtDecl stmt_decl, std::optional<NamedDecl*> parent_decl = std::nullopt) : stmt_decl(stmt_decl), kind(STMT_DECL), parent_decl(parent_decl) {}
    Node(StmtCall stmt_call, std::optional<NamedDecl*> parent_decl = std::nullopt) : stmt_call(stmt_call), kind(STMT_CALL), parent_decl(parent_decl) {}
    Node(StmtConstructor stmt_cons, std::optional<NamedDecl*> parent_decl = std::nullopt) : stmt_cons(stmt_cons), kind(STMT_CONSTRUCTOR), parent_decl(parent_decl) {}
    Node(StmtImplicitDestructor stmt, std::optional<NamedDecl*> parent_decl = std::nullopt) : stmt_implicit_dtor(stmt), kind(STMT_IMPLICIT_DESTRUCTOR), parent_decl(parent_decl) {}
    Node(StmtCompound stmt_compound, std::optional<NamedDecl*> parent_decl = std::nullopt) : stmt_compound(stmt_compound), kind(STMT_COMPOUND), parent_decl(parent_decl) {}
    Node(StmtReturn stmt_return, std::optional<NamedDecl*> parent_decl = std::nullopt) : stmt_return(stmt_return), kind(STMT_RETURN), parent_decl(parent_decl) {}
    Node(StmtRef stmt_ref, std::optional<NamedDecl*> parent_decl = std::nullopt) : stmt_ref(stmt_ref), kind(STMT_REF), parent_decl(parent_decl) {}
    Node(StmtOther stmt_other, std::optional<NamedDecl*> parent_decl = std::nullopt) : stmt_other(stmt_other), kind(STMT_OTHER), parent_decl(parent_decl) {}
};


enum EdgeKind {
    RECORD_FIELD,
    RECORD_METHOD,
    RECORD_CONSTRUCTOR,
    RECORD_DESTRUCTOR,
    RECORD_INHERIT,
    CONTROL_ENTRY,
    CONTROL_FUNCTION_INVOCATION,
    CONTROL_CONSTRUCTOR_INVOCATION,
    CONTROL_DESTRUCTOR_INVOCATION,
    CONTROL_METHOD_INVOCATION,
    DATA_OBJECT,
    DATA_PARAM,
    DATA_DEFUSE,
    DATA_DECL,
    DATA_ARGPASS,
    CHILD
};

std::string edge_kind_name(EdgeKind kind); 

struct Edge {
    NodeID src;
    NodeID dst;
    EdgeKind kind;
    Edge(NodeID src, NodeID dst, EdgeKind kind) : src(src), dst(dst), kind(kind) {}
};

class Graph : public cle::Graph<Node, Edge> {
public:    
    Graph(ASTContext* ctx) : ast_ctx(ctx) { }
    NodeID add_decl(Decl* decl, std::optional<NamedDecl*> parent_decl = std::nullopt); 
    using NodeTable = Table<NodeID, std::string, std::string, std::string, std::string,  
        std::string, unsigned int, unsigned int>;
    using EdgeTable = Table<EdgeID, std::string, NodeID, NodeID>;
    NodeTable node_table();
    EdgeTable edge_table();

    NodeID add_node(Node&& node) override;
private:

    NodeID add_record_decl(CXXRecordDecl* decl, std::optional<NamedDecl*> parent_decl = std::nullopt); 
    NodeID add_function_decl(FunctionDecl* decl, std::optional<NamedDecl*> parent_decl = std::nullopt);
    NodeID add_field_decl(FieldDecl* decl, std::optional<NamedDecl*> parent_decl = std::nullopt); 
    NodeID add_method_decl(CXXMethodDecl* decl, std::optional<NamedDecl*> parent_decl = std::nullopt); 
    NodeID add_constructor_decl(CXXConstructorDecl* decl, std::optional<NamedDecl*> parent_decl = std::nullopt); 
    NodeID add_destructor_decl(CXXDestructorDecl* decl, std::optional<NamedDecl*> parent_decl = std::nullopt); 
    NodeID add_stmt(Stmt* stmt, std::optional<NamedDecl*> parent_decl = std::nullopt); 
    NodeID add_compound_stmt(CompoundStmt* stmt, std::optional<NamedDecl*> parent_decl = std::nullopt); 
    NodeID add_decl_stmt(DeclStmt* stmt, std::optional<NamedDecl*> parent_decl = std::nullopt); 
    NodeID add_return_stmt(ReturnStmt* stmt, std::optional<NamedDecl*> parent_decl = std::nullopt); 
    NodeID add_ref_stmt(DeclRefExpr* stmt, std::optional<NamedDecl*> parent_decl = std::nullopt); 
    NodeID add_call_stmt(CallExpr* stmt, std::optional<NamedDecl*> parent_decl = std::nullopt); 
    NodeID add_constructor_call_stmt(CXXConstructExpr* stmt, std::optional<NamedDecl*> parent_decl = std::nullopt);
    NodeID add_member_call_stmt(CXXMemberCallExpr* stmt, std::optional<NamedDecl*> parent_decl = std::nullopt); 
    NodeID add_other_stmt(Stmt* stmt, std::optional<NamedDecl*> parent_decl = std::nullopt); 

    template<typename ClangDecl, typename CLENode> 
    NodeID add_function_like(ClangDecl* decl, std::optional<NamedDecl*> parent_decl = std::nullopt);

    void add_implicit_destructors(Decl* decl, Stmt* body, std::optional<NamedDecl*> parent_decl = std::nullopt);

    std::map<const Type*, NodeID> record_definitions;
    std::map<NamedDecl*, NodeID> named_decls;
    std::map<int64_t, NodeID> clang_to_node_id; 

    ASTContext* ast_ctx;


};


};
};