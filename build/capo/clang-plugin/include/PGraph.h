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
    STMT_REF,
    STMT_FIELD,
    STMT_THIS,
    STMT_RETURN,
    STMT_OTHER
};

std::string node_kind_name(NodeKind kind); 

struct NodeCtx {
    clang::NamedDecl* parent_decl;
    clang::CXXRecordDecl* parent_class;
    clang::FunctionDecl* parent_function;
    NodeCtx() : parent_decl(nullptr), parent_class(nullptr), parent_function(nullptr) {}
    NodeCtx(NamedDecl* parent_decl, CXXRecordDecl* parent_class, FunctionDecl* parent_function) : 
        parent_decl(parent_decl), parent_class(parent_class), parent_function(parent_function) {}
    NodeCtx set_parent_class(CXXRecordDecl* parent_class) {
        return NodeCtx(parent_class, parent_class, parent_function);
    }
    NodeCtx set_parent_function(FunctionDecl* parent_function) {
        return NodeCtx(parent_function, parent_class, parent_function);
    }
};


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

struct StmtThis {
    clang::CXXThisExpr* stmt;
    StmtThis(clang::CXXThisExpr* stmt) : stmt(stmt) {}
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
        StmtThis stmt_this;
    };
    NodeKind kind;
    NodeCtx ctx; 

    std::optional<size_t> param_idx = std::nullopt; 

    std::optional<std::string> qualified_name();

    std::optional<NamedDecl*> named_decl();
    clang::SourceRange source_range();

    int64_t clang_node_id(ASTContext*);

    Node(DeclVar decl_var, NodeCtx ctx = NodeCtx()) : decl_var(decl_var), kind(DECL_VAR), ctx(ctx) {}
    Node(DeclFun decl_fun, NodeCtx ctx = NodeCtx()) : decl_fun(decl_fun), kind(DECL_FUN), ctx(ctx) {}
    Node(DeclRecord decl_record, NodeCtx ctx = NodeCtx()) : decl_record(decl_record), kind(DECL_RECORD), ctx(ctx) {}
    Node(DeclField decl_field, NodeCtx ctx = NodeCtx()) : decl_field(decl_field), kind(DECL_FIELD), ctx(ctx) {}
    Node(DeclParam decl_param, NodeCtx ctx = NodeCtx(), size_t param_idx = 0) : decl_param(decl_param), kind(DECL_PARAM), ctx(ctx), param_idx(param_idx) {}
    Node(DeclMethod decl_method, NodeCtx ctx = NodeCtx()) : decl_method(decl_method), kind(DECL_METHOD), ctx(ctx) {}
    Node(DeclConstructor decl_constructor, NodeCtx ctx = NodeCtx()) : decl_constructor(decl_constructor), kind(DECL_CONSTRUCTOR), ctx(ctx) {}
    Node(DeclDestructor decl_destructor, NodeCtx ctx = NodeCtx()) : decl_destructor(decl_destructor), kind(DECL_DESTRUCTOR), ctx(ctx) {}
    Node(StmtDecl stmt_decl, NodeCtx ctx = NodeCtx()) : stmt_decl(stmt_decl), kind(STMT_DECL), ctx(ctx) {}
    Node(StmtCall stmt_call, NodeCtx ctx = NodeCtx()) : stmt_call(stmt_call), kind(STMT_CALL), ctx(ctx) {}
    Node(StmtConstructor stmt_cons, NodeCtx ctx = NodeCtx()) : stmt_cons(stmt_cons), kind(STMT_CONSTRUCTOR), ctx(ctx) {}
    Node(StmtImplicitDestructor stmt, NodeCtx ctx = NodeCtx()) : stmt_implicit_dtor(stmt), kind(STMT_IMPLICIT_DESTRUCTOR), ctx(ctx) {}
    Node(StmtCompound stmt_compound, NodeCtx ctx = NodeCtx()) : stmt_compound(stmt_compound), kind(STMT_COMPOUND), ctx(ctx) {}
    Node(StmtReturn stmt_return, NodeCtx ctx = NodeCtx()) : stmt_return(stmt_return), kind(STMT_RETURN), ctx(ctx) {}
    Node(StmtRef stmt_ref, NodeCtx ctx = NodeCtx()) : stmt_ref(stmt_ref), kind(STMT_REF), ctx(ctx) {}
    Node(StmtOther stmt_other, NodeCtx ctx = NodeCtx()) : stmt_other(stmt_other), kind(STMT_OTHER), ctx(ctx) {}
    Node(StmtThis stmt_this, NodeCtx ctx = NodeCtx()) : stmt_this(stmt_this), kind(STMT_THIS), ctx(ctx) {}
};


enum EdgeKind {
    STRUCT_FIELD,
    STRUCT_METHOD,
    STRUCT_CONSTRUCTOR,
    STRUCT_DESTRUCTOR,
    STRUCT_INHERIT,
    STRUCT_PARAM,
    CONTROL_RETURN,
    CONTROL_ENTRY,
    CONTROL_FUNCTION_INVOCATION,
    CONTROL_METHOD_INVOCATION,
    CONTROL_CONSTRUCTOR_INVOCATION,
    CONTROL_DESTRUCTOR_INVOCATION,
    CHILD,
    DATA_DEFUSE,
    DATA_ARGPASS,
    DATA_OBJECT,
    DATA_FIELDACCESS,
    DATA_INSTANCEOF,
    DATA_DECL,
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
    NodeID add_decl(Decl* decl, NodeCtx ctx = NodeCtx()); 
    using NodeTable = Table<NodeID, 
        std::string, std::string, std::string,      // node type, debug name, annotation 
        std::string, std::string, std::string,      // parent decl, parent class, parent function
        std::string,                                // param idx
        std::string, unsigned int, unsigned int>;   // filename, start offset, end offset
    using EdgeTable = Table<EdgeID, std::string, NodeID, NodeID>;
    NodeTable node_table();
    EdgeTable edge_table();

    NodeID add_node(Node&& node) override;
private:

    NodeID add_record_decl(CXXRecordDecl* decl, NodeCtx ctx = NodeCtx()); 
    NodeID add_function_decl(FunctionDecl* decl, NodeCtx ctx = NodeCtx());
    NodeID add_field_decl(FieldDecl* decl, NodeCtx ctx = NodeCtx()); 
    NodeID add_method_decl(CXXMethodDecl* decl, NodeCtx ctx = NodeCtx()); 
    NodeID add_constructor_decl(CXXConstructorDecl* decl, NodeCtx ctx = NodeCtx()); 
    NodeID add_destructor_decl(CXXDestructorDecl* decl, NodeCtx ctx = NodeCtx()); 
    NodeID add_stmt(Stmt* stmt, NodeCtx ctx = NodeCtx()); 
    NodeID add_compound_stmt(CompoundStmt* stmt, NodeCtx ctx = NodeCtx()); 
    NodeID add_decl_stmt(DeclStmt* stmt, NodeCtx ctx = NodeCtx()); 
    NodeID add_return_stmt(ReturnStmt* stmt, NodeCtx ctx = NodeCtx()); 
    NodeID add_ref_stmt(DeclRefExpr* stmt, NodeCtx ctx = NodeCtx()); 
    NodeID add_call_stmt(CallExpr* stmt, NodeCtx ctx = NodeCtx()); 
    NodeID add_constructor_call_stmt(CXXConstructExpr* stmt, NodeCtx ctx = NodeCtx());
    NodeID add_member_call_stmt(CXXMemberCallExpr* stmt, NodeCtx ctx = NodeCtx()); 
    NodeID add_other_stmt(Stmt* stmt, NodeCtx ctx = NodeCtx()); 
    NodeID add_this_stmt(CXXThisExpr* stmt, NodeCtx ctx = NodeCtx());

    std::map<NodeID, NodeID> reorder_nodes();
    std::map<EdgeID, EdgeID> reorder_edges();
    std::optional<NodeID> find_node(NamedDecl* decl);

    template<typename ClangDecl, typename CLENode> 
    NodeID add_function_like(ClangDecl* decl, NodeCtx ctx = NodeCtx());

    void add_implicit_destructors(Decl* decl, Stmt* body, NodeCtx ctx = NodeCtx());

    std::map<const Type*, NodeID> record_definitions;
    std::map<NamedDecl*, NodeID> named_decls;
    std::map<int64_t, NodeID> clang_to_node_id; 

    ASTContext* ast_ctx;


};


};
};