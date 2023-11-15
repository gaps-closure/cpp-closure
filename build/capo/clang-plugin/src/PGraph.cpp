#include "PGraph.h"

using namespace clang;
using namespace cle;
using namespace cle::pgraph;

std::string pgraph::node_kind_name(NodeKind kind) {
    switch(kind) {
    case DECL_VAR:
        return "Decl.Var";
    case DECL_FUN:
        return "Decl.Fun";
    case DECL_RECORD:
        return "Decl.Record";
    case DECL_FIELD:
        return "Decl.Field";
    case DECL_METHOD:
        return "Decl.Method";
    case DECL_PARAM:
        return "Decl.Param";
    case DECL_CONSTRUCTOR:
        return "Decl.Constructor";
    case DECL_DESTRUCTOR:
        return "Decl.Destructor";
    default:
        return "";
    }
}

void pgraph::Graph::add_field_decl(FieldDecl* decl) {
    add_node(Node(DeclField(decl)));
}

void pgraph::Graph::add_record_decl(CXXRecordDecl* decl) {
    add_node(Node(DeclRecord(decl)));

    for(auto fdecl : decl->fields()) {
        add_field_decl(fdecl);
    }
}


void pgraph::Graph::add_top_level_decl(Decl* decl) {
    switch(decl->getKind()) {
        case Decl::Var:
            add_node(Node(DeclVar(dyn_cast<VarDecl>(decl))));
            return;
        case Decl::Function:
            add_node(Node(DeclFun(dyn_cast<FunctionDecl>(decl))));
            return;
        case Decl::CXXRecord:
            add_record_decl(dyn_cast<CXXRecordDecl>(decl));
        default:
            return;
    }
}

Table<NodeID, std::string, std::string> pgraph::Graph::node_table() {
    Table<NodeID, std::string, std::string> tbl;
    size_t index = 0;
    for(auto& node : nodes) {
        std::string name;

        switch(node.kind) {
            case NodeKind::DECL_VAR:
                name = node.decl_var.decl->getQualifiedNameAsString();
                break;
            case NodeKind::DECL_FUN:
                name = node.decl_fun.decl->getQualifiedNameAsString();
                break;
            case NodeKind::DECL_RECORD:
                name = node.decl_fun.decl->getQualifiedNameAsString();
                break;
            case NodeKind::DECL_FIELD:
                name = node.decl_field.decl->getQualifiedNameAsString();
            default:
                break;
        }

        std::string nk_name = node_kind_name(node.kind);

        HetList<cle::NodeID, std::string, std::string> row{index, nk_name, name}; 

        tbl << row;
        index++;
    }
    return tbl;
}