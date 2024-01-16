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

struct SDecl {
    bool top_level = false;
    SDecl(bool top_level) : top_level(top_level) {}
    SDecl() : top_level(false) {}
};

struct DeclVar : public SDecl {
    clang::VarDecl* decl;
    DeclVar(clang::VarDecl* decl, bool top_level = false) : SDecl(top_level), decl(decl) {}
};

struct DeclFun : public SDecl {
    clang::FunctionDecl* decl;
    DeclFun(clang::FunctionDecl* decl) : SDecl(true), decl(decl) {}
};

struct DeclRecord : public SDecl {
    clang::CXXRecordDecl* decl;
    DeclRecord(clang::CXXRecordDecl* decl, bool top_level = false) : SDecl(top_level), decl(decl) {}
};

struct DeclField : public SDecl {
    clang::FieldDecl* decl;
    DeclField(clang::FieldDecl* decl) : decl(decl) {}
};

struct DeclMethod : public SDecl {
    clang::CXXMethodDecl* decl;
    DeclMethod(clang::CXXMethodDecl* decl) : decl(decl) {}
};

struct DeclParam : public SDecl {
    clang::ParmVarDecl* decl;
    DeclParam(clang::ParmVarDecl* decl) : decl(decl) {}
};

struct DeclConstructor : public SDecl {
    clang::CXXConstructorDecl* decl;
    DeclConstructor(clang::CXXConstructorDecl* decl) : decl(decl) {}
};

struct DeclDestructor : public SDecl {
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

    std::optional<std::string> qualified_name();

    std::optional<SDecl*> as_sdecl();
    std::optional<NamedDecl*> named_decl();
    clang::SourceRange source_range();

    int64_t clang_node_id(ASTContext*);

    Node(DeclVar decl_var) : decl_var(decl_var), kind(DECL_VAR) {}
    Node(DeclFun decl_fun) : decl_fun(decl_fun), kind(DECL_FUN) {}
    Node(DeclRecord decl_record) : decl_record(decl_record), kind(DECL_RECORD) {}
    Node(DeclField decl_field) : decl_field(decl_field), kind(DECL_FIELD) {}
    Node(DeclParam decl_param) : decl_param(decl_param), kind(DECL_PARAM) {}
    Node(DeclMethod decl_method) : decl_method(decl_method), kind(DECL_METHOD) {}
    Node(DeclConstructor decl_constructor) : decl_constructor(decl_constructor), kind(DECL_CONSTRUCTOR) {}
    Node(DeclDestructor decl_destructor) : decl_destructor(decl_destructor), kind(DECL_DESTRUCTOR) {}
    Node(StmtDecl stmt_decl) : stmt_decl(stmt_decl), kind(STMT_DECL) {}
    Node(StmtCall stmt_call) : stmt_call(stmt_call), kind(STMT_CALL) {}
    Node(StmtConstructor stmt_cons) : stmt_cons(stmt_cons), kind(STMT_CONSTRUCTOR) {}
    Node(StmtImplicitDestructor stmt) : stmt_implicit_dtor(stmt), kind(STMT_IMPLICIT_DESTRUCTOR) {}
    Node(StmtCompound stmt_compound) : stmt_compound(stmt_compound), kind(STMT_COMPOUND) {}
    Node(StmtReturn stmt_return) : stmt_return(stmt_return), kind(STMT_RETURN) {}
    Node(StmtRef stmt_ref) : stmt_ref(stmt_ref), kind(STMT_REF) {}
    Node(StmtOther stmt_other) : stmt_other(stmt_other), kind(STMT_OTHER) {}
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
    NodeID add_decl(Decl* decl, bool top_level = false); 
    using NodeTable = Table<NodeID, std::string, std::string, std::string, std::string, 
        std::string, unsigned int, unsigned int>;
    using EdgeTable = Table<EdgeID, std::string, NodeID, NodeID>;
    NodeTable node_table();
    EdgeTable edge_table();

    NodeID add_node(Node&& node) override;
private:

    NodeID add_record_decl(CXXRecordDecl* decl, bool top_level = false); 
    NodeID add_function_decl(FunctionDecl* decl);
    NodeID add_field_decl(FieldDecl* decl); 
    NodeID add_method_decl(CXXMethodDecl* decl); 
    NodeID add_constructor_decl(CXXConstructorDecl* decl); 
    NodeID add_destructor_decl(CXXDestructorDecl* decl); 
    NodeID add_stmt(Stmt* stmt); 
    NodeID add_compound_stmt(CompoundStmt* stmt); 
    NodeID add_decl_stmt(DeclStmt* stmt); 
    NodeID add_return_stmt(ReturnStmt* stmt); 
    NodeID add_ref_stmt(DeclRefExpr* stmt); 
    NodeID add_call_stmt(CallExpr* stmt); 
    NodeID add_constructor_call_stmt(CXXConstructExpr* stmt);
    NodeID add_member_call_stmt(CXXMemberCallExpr* stmt); 
    NodeID add_other_stmt(Stmt* stmt); 

    template<typename ClangDecl, typename CLENode> 
    NodeID add_function_like(ClangDecl* decl);

    void add_implicit_destructors(Decl* decl, Stmt* body);

    std::map<const Type*, NodeID> record_definitions;
    std::map<NamedDecl*, NodeID> named_decls;
    std::map<int64_t, NodeID> clang_to_node_id; 

    ASTContext* ast_ctx;


};


};
};