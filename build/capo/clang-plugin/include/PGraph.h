#pragma once

#include "Table.h"
#include "Graph.h"
#include "clang/AST/AST.h"

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
    DECL_DESTRUCTOR
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
};

struct DeclParam {
    clang::ParmVarDecl* decl;
};

struct DeclConstructor {
    clang::CXXConstructorDecl* decl;
};

struct DeclDestructor {
    clang::CXXDestructorDecl* decl;
};


struct Node {
    union {
        DeclVar decl_var;
        DeclFun decl_fun;
        DeclRecord decl_record;
        DeclField decl_field;
        DeclMethod decl_method;
        DeclParam decl_param;
        DeclConstructor decl_constructor;
        DeclDestructor decl_destructor;
    };
    NodeKind kind;

    Node(DeclVar decl_var) : decl_var(decl_var), kind(DECL_VAR) {}
    Node(DeclFun decl_fun) : decl_fun(decl_fun), kind(DECL_FUN) {}
    Node(DeclRecord decl_record) : decl_record(decl_record), kind(DECL_RECORD) {}
    Node(DeclField decl_field) : decl_field(decl_field), kind(DECL_FIELD) {}
};


class Edge {
private:
    Node& src;
    Node& dst;
public:
    Edge(Node& src, Node& dst) : src(src), dst(dst) {}
    Node& src_node() { return src; }
    Node& dst_node() { return dst; }
};

class Graph : public cle::Graph<Node, Edge> {
public:    
    Graph() {}
    void add_top_level_decl(Decl* decl); 
    void add_record_decl(CXXRecordDecl* decl); 
    void add_field_decl(FieldDecl* decl); 

    Table<NodeID, std::string, std::string> node_table();
};


};
};