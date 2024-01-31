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
    case STMT_COMPOUND:
        return "Stmt.Compound";
    case STMT_CALL:
        return "Stmt.Call";
    case STMT_CONSTRUCTOR:
        return "Stmt.Call";
    case STMT_IMPLICIT_DESTRUCTOR:
        return "Stmt.Call";
    case STMT_RETURN:
        return "Stmt.Return";
    case STMT_DECL:
        return "Stmt.Decl";
    case STMT_REF:
        return "Stmt.Ref";
    case STMT_OTHER:
        return "Stmt.Other";
    case STMT_THIS:
        return "Stmt.This";
    default:
        return "Unknown";
    }
}

std::string pgraph::edge_kind_name(EdgeKind kind) {
    switch(kind) {
    case RECORD_FIELD:
        return "Record.Field";
    case RECORD_METHOD:
        return "Record.Method";
    case RECORD_CONSTRUCTOR:
        return "Record.Constructor";
    case RECORD_DESTRUCTOR:
        return "Record.Method";
    case RECORD_INHERIT:
        return "Record.Inherit";
    case DATA_OBJECT:
        return "Data.Object";
    case DATA_PARAM:
        return "Data.Param";
    case DATA_DECL:
        return "Data.Decl";
    case DATA_DEFUSE:
        return "Data.DefUse";
    case DATA_ARGPASS:
        return "Data.ArgPass";
    case CONTROL_ENTRY:
        return "Control.Entry";
    case CONTROL_FUNCTION_INVOCATION:
        return "Control.FunctionInvocation";
    case CONTROL_CONSTRUCTOR_INVOCATION:
        return "Control.ConstructorInvocation";
    case CONTROL_DESTRUCTOR_INVOCATION:
        return "Control.DestructorInvocation";
    case CONTROL_METHOD_INVOCATION:
        return "Control.MethodInvocation";
    case CHILD:
        return "Child";
    default:
        return "Unknown";
    }
}


NodeID pgraph::Graph::add_node(Node&& node) {
    nodes.insert(std::pair<NodeID, Node>(cur_node_id, node));
    auto n = nodes.at(cur_node_id);
    auto decl = n.named_decl();
    clang_to_node_id[node.clang_node_id(ast_ctx)] = cur_node_id; 
    if(decl)
        named_decls[*decl] = cur_node_id; 
    cur_node_id++;
    return cur_node_id - 1;
}

NodeID pgraph::Graph::add_method_decl(CXXMethodDecl* decl, std::optional<clang::NamedDecl*> parent_decl) {
    return add_function_like<CXXMethodDecl, DeclMethod>(decl, parent_decl);
}

NodeID pgraph::Graph::add_field_decl(FieldDecl* decl, std::optional<clang::NamedDecl*> parent_decl) {
    return add_node(Node(DeclField(decl), parent_decl));
}

template<typename ClangDecl, typename CLENode> 
NodeID pgraph::Graph::add_function_like(ClangDecl* decl, std::optional<clang::NamedDecl*> parent_decl) {
    NodeID id;
    bool found_prev_decl = false;
    FunctionDecl* decl_with_body = nullptr;
    for(auto redecl : decl->redecls()) {
        if(redecl->hasBody()) {
            decl_with_body = redecl;
        }
        if(named_decls.find(redecl) != named_decls.end()) {
            id = named_decls[redecl];
            auto node = get_node(id);
            if(decl_with_body != nullptr) {
                replace_node(id, Node(CLENode(static_cast<ClangDecl*>(decl_with_body)), parent_decl));
                auto cid = add_stmt(decl_with_body->getBody(), static_cast<NamedDecl*>(decl_with_body));
                add_edge(Edge(id, cid, CONTROL_ENTRY));
            }
            found_prev_decl = true;
            break;
        }
    }

    if(!found_prev_decl) {
        id = add_node(Node(CLENode(decl), parent_decl));
        auto decl_p = get_node(id).decl_fun.decl;

        for(auto param : decl_p->parameters()) {
            auto pid = add_node(Node(DeclParam(param), static_cast<NamedDecl*>(decl)));
            add_edge(Edge(id, pid, DATA_PARAM));
        }

        if(decl->hasBody()) {

            auto cid = add_stmt(decl->getBody(), static_cast<NamedDecl*>(decl));
            add_implicit_destructors(decl, decl->getBody(),static_cast<NamedDecl*>(decl));
            add_edge(Edge(id, cid, CONTROL_ENTRY));
        }  
    }

    return id;

}

NodeID pgraph::Graph::add_constructor_decl(CXXConstructorDecl* decl, std::optional<clang::NamedDecl*> parent_decl) {
    return add_function_like<CXXConstructorDecl, DeclConstructor>(decl, parent_decl);
}

NodeID pgraph::Graph::add_destructor_decl(CXXDestructorDecl* decl, std::optional<clang::NamedDecl*> parent_decl) {
    return add_function_like<CXXDestructorDecl, DeclDestructor>(decl, parent_decl);
}

NodeID pgraph::Graph::add_compound_stmt(CompoundStmt* stmt, std::optional<clang::NamedDecl*> parent_decl) {
    auto id = add_node(Node(StmtCompound(stmt), parent_decl));
    for(auto child : stmt->children()) {
        auto cid = add_stmt(child, parent_decl);
        add_edge(Edge(id, cid, CHILD));
    }
    return id;
}

NodeID pgraph::Graph::add_ref_stmt(DeclRefExpr* stmt, std::optional<clang::NamedDecl*> parent_decl) {
    auto id = add_node(Node(StmtRef(stmt), parent_decl));
    if(named_decls.find(stmt->getFoundDecl()) != named_decls.end()) {
        auto did = named_decls[stmt->getFoundDecl()];
        add_edge(Edge(id, did, DATA_DEFUSE));
    }
    return id;
}


NodeID pgraph::Graph::add_decl_stmt(DeclStmt* stmt, std::optional<clang::NamedDecl*> parent_decl) {
    auto id = add_node(Node(StmtDecl(stmt), parent_decl));
    for(auto decl : stmt->decls()) {
        auto did = add_decl(decl, parent_decl);
        add_edge(Edge(id, did, DATA_DECL));
    }
    return id;
}

NodeID pgraph::Graph::add_other_stmt(Stmt* stmt, std::optional<clang::NamedDecl*> parent_decl) {
    auto id = add_node(Node(StmtOther(stmt), parent_decl));
    for(auto child : stmt->children()) {
        auto cid = add_stmt(child, parent_decl);
        add_edge(Edge(id, cid, CHILD));
    }
    return id;
}

NodeID pgraph::Graph::add_this_stmt(CXXThisExpr* stmt, std::optional<clang::NamedDecl*> parent_decl) {
    auto id = add_node(Node(StmtThis(stmt), parent_decl));
    return id;
}

NodeID pgraph::Graph::add_return_stmt(ReturnStmt* stmt, std::optional<clang::NamedDecl*> parent_decl) {
    auto id = add_node(Node(StmtReturn(stmt), parent_decl));

    for(auto child : stmt->children()) {
        auto cid = add_stmt(child, parent_decl);
        add_edge(Edge(id, cid, CHILD));
    }
   
    return id;
}

NodeID pgraph::Graph::add_call_stmt(CallExpr* stmt, std::optional<clang::NamedDecl*> parent_decl) {
    auto id = add_node(Node(StmtCall(stmt), parent_decl));

    if(auto decl = stmt->getDirectCallee()) {
        NodeID fid;
        if(named_decls.find(decl) != named_decls.end()) {
            fid = named_decls[decl];
        } else {
            fid = add_function_decl(decl, parent_decl);
        }
        add_edge(Edge(id, fid, CONTROL_FUNCTION_INVOCATION));
    }

    for(auto arg : stmt->arguments()) {
        auto aid = add_stmt(arg, parent_decl);
        add_edge(Edge(id, aid, DATA_ARGPASS));
    }

    return id;
}


NodeID pgraph::Graph::add_member_call_stmt(CXXMemberCallExpr* stmt, std::optional<clang::NamedDecl*> parent_decl) {
    auto id = add_node(Node(StmtCall(stmt), parent_decl));

    // stmt->getPreArg()
    if(auto decl = stmt->getMethodDecl()) {
        NodeID fid;
        if(named_decls.find(decl) != named_decls.end()) {
            fid = named_decls[decl];
        } else {
            fid = add_method_decl(decl, parent_decl);
        }
        auto node = get_node(fid);
        if(node.kind == DECL_DESTRUCTOR) {
            add_edge(Edge(id, fid, CONTROL_DESTRUCTOR_INVOCATION));
        } else {
            add_edge(Edge(id, fid, CONTROL_METHOD_INVOCATION));
        }
    }

    for(auto arg : stmt->arguments()) {
        auto aid = add_stmt(arg, parent_decl);
        add_edge(Edge(id, aid, DATA_ARGPASS));
    }

    if(auto obj = stmt->getImplicitObjectArgument()) {
        auto oid = add_stmt(obj, parent_decl);
        add_edge(Edge(id, oid, DATA_OBJECT));
    }

    return id;

}


NodeID pgraph::Graph::add_constructor_call_stmt(CXXConstructExpr* stmt, std::optional<clang::NamedDecl*> parent_decl) {
    auto id = add_node(Node(StmtConstructor(stmt), parent_decl));

    NodeID cid;
    {
        auto decl = stmt->getConstructor();
        if(named_decls.find(decl) != named_decls.end()) {
            cid = named_decls[decl];
        } else {
            cid = add_constructor_decl(stmt->getConstructor(), parent_decl);
        }
    }

    add_edge(Edge(id, cid, CONTROL_CONSTRUCTOR_INVOCATION));

    for(auto arg : stmt->arguments()) {
        auto aid = add_stmt(arg, parent_decl);
        add_edge(Edge(id, aid, DATA_ARGPASS));
    }

    return id;
}


NodeID pgraph::Graph::add_stmt(Stmt* stmt, std::optional<clang::NamedDecl*> parent_decl) {
    switch(stmt->getStmtClass()) {
        case Stmt::CompoundStmtClass:
            return add_compound_stmt(dyn_cast<CompoundStmt>(stmt), parent_decl);
        case Stmt::DeclStmtClass:
            return add_decl_stmt(dyn_cast<DeclStmt>(stmt), parent_decl);
        case Stmt::ReturnStmtClass:
            return add_return_stmt(dyn_cast<ReturnStmt>(stmt), parent_decl);
        case Stmt::DeclRefExprClass: 
            return add_ref_stmt(dyn_cast<DeclRefExpr>(stmt), parent_decl);
        case Stmt::CallExprClass:
            return add_call_stmt(dyn_cast<CallExpr>(stmt), parent_decl);
        case Stmt::CXXConstructExprClass:
            return add_constructor_call_stmt(dyn_cast<CXXConstructExpr>(stmt), parent_decl);
        case Stmt::CXXMemberCallExprClass:
            return add_member_call_stmt(dyn_cast<CXXMemberCallExpr>(stmt), parent_decl);
        case Stmt::CXXThisExprClass:
            return add_this_stmt(dyn_cast<CXXThisExpr>(stmt), parent_decl);
        default:
            return add_other_stmt(stmt, parent_decl);
    }
}

void pgraph::Graph::add_implicit_destructors(Decl* decl, Stmt* body, std::optional<clang::NamedDecl*> parent_decl) {
    clang::CFG::BuildOptions opts;
    opts.AddImplicitDtors = true;
    opts.AddScopes = true;
    opts.AddTemporaryDtors = true;
    std::unique_ptr<clang::CFG> cfg(
        clang::CFG::buildCFG(decl, decl->getBody(), ast_ctx, opts)
    );
    // cfg->dump(LangOptions(), true);
    for(auto block : *cfg) {
        for(auto stmt : *block) {
            if (auto dtor = stmt.getAs<clang::CFGAutomaticObjDtor>()) {
                auto clang_id = dtor->getTriggerStmt()->getID(*ast_ctx);
                if(clang_to_node_id.find(clang_id) != clang_to_node_id.end()) {
                    auto id = clang_to_node_id[clang_id];
                    std::optional<NodeID> parent_id = std::nullopt; 
                    for(auto [eid, edge] : edges) {
                        if(edge.kind == EdgeKind::CHILD && edge.dst == id) {
                            parent_id = edge.src;
                            break;
                        }
                    }
                    if(parent_id) {
                        auto did = add_node(Node(StmtImplicitDestructor(*dtor), parent_decl));
                        add_edge(Edge(*parent_id, did, EdgeKind::CHILD));
                        add_edge(Edge(did, clang_to_node_id[dtor->getDestructorDecl(*ast_ctx)->getID()], EdgeKind::CONTROL_DESTRUCTOR_INVOCATION));
                    }
                }
            } 
        }
    }

}

NodeID pgraph::Graph::add_function_decl(FunctionDecl* decl, std::optional<clang::NamedDecl*> parent_decl) {
    return add_function_like<FunctionDecl, DeclFun>(decl, parent_decl);
}

NodeID pgraph::Graph::add_record_decl(CXXRecordDecl* decl, std::optional<clang::NamedDecl*> parent_decl) {

    NodeID id; 
    bool found_prev_decl = false;
    for(auto redecl : decl->redecls()) {
        if(named_decls.find(redecl) != named_decls.end()) {
            id = named_decls[redecl];
            auto node = get_node(id);
            if(!node.decl_record.decl->hasDefinition() && decl->hasDefinition()) {
                replace_node(id, Node(DeclRecord(decl), parent_decl));
            }
            found_prev_decl = true;
            break;
        }
    }
    if(!found_prev_decl)
        id = add_node(Node(DeclRecord(decl), parent_decl));

    if(!decl->hasDefinition())
        return id;

    for(auto base : decl->bases()) {
        auto ty = base.getType().getTypePtr();
        if(record_definitions.find(ty) != record_definitions.end()) {
            add_edge(Edge(id, record_definitions[ty], RECORD_INHERIT));
        }
    }
    record_definitions.insert(std::pair(decl->getTypeForDecl(), id));

    for(auto fdecl : decl->fields()) {
        auto fid = add_field_decl(fdecl, static_cast<NamedDecl*>(decl));
        add_edge(Edge(id, fid, RECORD_FIELD));
    }

    for(auto mdecl : decl->methods()) {
        if(clang::isa<CXXConstructorDecl>(mdecl) || clang::isa<CXXDestructorDecl>(mdecl))
            continue;
        auto mid = add_method_decl(mdecl, static_cast<NamedDecl*>(decl));
        add_edge(Edge(id, mid, RECORD_METHOD));
    }

    for(auto cdecl : decl->ctors()) {
        auto cid = add_constructor_decl(cdecl, static_cast<NamedDecl*>(decl));
        add_edge(Edge(id, cid, RECORD_CONSTRUCTOR));
    }

    if(decl->hasUserDeclaredDestructor()) {
        auto ddecl = decl->getDestructor();
        auto did = add_destructor_decl(ddecl, static_cast<NamedDecl*>(decl));
        add_edge(Edge(id, did, RECORD_CONSTRUCTOR));
    }
    return id;
}

NodeID pgraph::Graph::add_decl(Decl* decl, std::optional<clang::NamedDecl*> parent_decl) {
    switch(decl->getKind()) {
        case Decl::Var: 
            return add_node(Node(DeclVar(dyn_cast<VarDecl>(decl)), parent_decl));
        case Decl::Function:
            return add_function_decl(dyn_cast<FunctionDecl>(decl), parent_decl);
        case Decl::CXXRecord:
            return add_record_decl(dyn_cast<CXXRecordDecl>(decl), parent_decl);
        case Decl::CXXMethod:
            return add_method_decl(dyn_cast<CXXMethodDecl>(decl), parent_decl);
        // case Decl::Field:
            // add_field_decl(dyn_cast<FieldDecl>(decl));
            // return;
        default:
            return 0;
    }
}

std::optional<std::string> pgraph::Node::qualified_name() {
    auto decl = named_decl();
    if(decl) {
        return (*decl)->getQualifiedNameAsString();
    } else {
        return std::nullopt;
    }
}

std::optional<NamedDecl*> pgraph::Node::named_decl() {
    switch(kind) {
        case NodeKind::DECL_VAR:
            return decl_var.decl;
        case NodeKind::DECL_FUN:
            return decl_fun.decl;
        case NodeKind::DECL_RECORD:
            return decl_fun.decl;
        case NodeKind::DECL_FIELD:
            return decl_field.decl;
        case NodeKind::DECL_METHOD:
            return decl_method.decl;
        case NodeKind::DECL_PARAM:
            return decl_param.decl;
        case NodeKind::DECL_CONSTRUCTOR:
            return decl_constructor.decl;
        case NodeKind::DECL_DESTRUCTOR:
            return decl_destructor.decl;
        default:
            return std::nullopt;
    }
}

clang::SourceRange pgraph::Node::source_range() {
    switch(kind) {
        case NodeKind::DECL_VAR:
            return decl_var.decl->getSourceRange();
        case NodeKind::DECL_FUN:
            return decl_fun.decl->getSourceRange();
        case NodeKind::DECL_RECORD:
            return decl_fun.decl->getSourceRange();
        case NodeKind::DECL_FIELD:
            return decl_field.decl->getSourceRange();
        case NodeKind::DECL_METHOD:
            return decl_method.decl->getSourceRange();
        case NodeKind::DECL_PARAM:
            return decl_param.decl->getSourceRange();
        case NodeKind::DECL_CONSTRUCTOR:
            return decl_constructor.decl->getSourceRange();
        case NodeKind::DECL_DESTRUCTOR:
            return decl_destructor.decl->getSourceRange();
        case NodeKind::STMT_DECL:
            return stmt_decl.stmt->getSourceRange();
        case NodeKind::STMT_CALL:
            return stmt_call.stmt->getSourceRange();
        case NodeKind::STMT_CONSTRUCTOR:
            return stmt_cons.stmt->getSourceRange();
        case NodeKind::STMT_IMPLICIT_DESTRUCTOR:
            return stmt_implicit_dtor.dtor.getTriggerStmt()->getSourceRange();
        case NodeKind::STMT_COMPOUND:
            return stmt_compound.stmt->getSourceRange();
        case NodeKind::STMT_RETURN:
            return stmt_return.stmt->getSourceRange();
        case NodeKind::STMT_REF:
            return stmt_ref.stmt->getSourceRange();
        case NodeKind::STMT_THIS:
            return stmt_this.stmt->getSourceRange();
        default:
            return stmt_other.stmt->getSourceRange();
    }
}

int64_t pgraph::Node::clang_node_id(ASTContext* ctx) {
    switch(kind) {
        case NodeKind::DECL_VAR:
            return decl_var.decl->getID();
        case NodeKind::DECL_FUN:
            return decl_fun.decl->getID();
        case NodeKind::DECL_RECORD:
            return decl_record.decl->getID();
        case NodeKind::DECL_FIELD:
            return decl_field.decl->getID();
        case NodeKind::DECL_METHOD:
            return decl_method.decl->getID();
        case NodeKind::DECL_PARAM:
            return decl_param.decl->getID();
        case NodeKind::DECL_CONSTRUCTOR:
            return decl_constructor.decl->getID();
        case NodeKind::DECL_DESTRUCTOR:
            return decl_destructor.decl->getID();
        case NodeKind::STMT_DECL:
            return stmt_decl.stmt->getID(*ctx);
        case NodeKind::STMT_CALL:
            return stmt_call.stmt->getID(*ctx);
        case NodeKind::STMT_CONSTRUCTOR:
            return stmt_cons.stmt->getID(*ctx);
        case NodeKind::STMT_COMPOUND:
            return stmt_compound.stmt->getID(*ctx);
        case NodeKind::STMT_RETURN:
            return stmt_return.stmt->getID(*ctx);
        case NodeKind::STMT_REF:
            return stmt_ref.stmt->getID(*ctx);
        case NodeKind::STMT_OTHER:
            return stmt_other.stmt->getID(*ctx);
        case NodeKind::STMT_THIS:
            return stmt_this.stmt->getID(*ctx);
        default:
            return -1;
    }
}

pgraph::Graph::NodeTable pgraph::Graph::node_table() {
    pgraph::Graph::NodeTable tbl;
    for(auto [id, node] : nodes) {
        std::string name = node.qualified_name().value_or("");
        std::string nk_name = node_kind_name(node.kind);
        std::string annotation; 
        if(node.named_decl()) {
            if(node.named_decl().value()->hasAttr<clang::AnnotateAttr>()) {
                auto attr = node.named_decl().value()->getAttr<clang::AnnotateAttr>();
                annotation = attr->getAnnotation();
            }
        }


        std::optional<NodeID> parent_id = std::nullopt; 
        if(node.parent_decl) {
            auto decl = *(node.parent_decl);
            if(named_decls.find(decl) != named_decls.end()) {
                parent_id = named_decls[decl];
            }
        }

        std::string parent_id_str;

        auto range = node.source_range();  
        auto start_loc = range.getBegin();
        auto &manager = ast_ctx->getSourceManager();
        auto filename = manager.getFilename(start_loc).str();
        auto start_off = manager.getFileOffset(start_loc);

        auto end_loc = range.getEnd();
        auto end_off = manager.getFileOffset(end_loc);
 
        if(parent_id)
            parent_id_str = std::to_string(*parent_id);
        HetList<cle::NodeID, std::string, std::string, std::string, std::string, 
            std::string, unsigned int, unsigned int> 
            row{id, nk_name, name, annotation, parent_id_str, 
                filename, start_off, end_off}; 

        tbl << row;
    }
    return tbl;
}


pgraph::Graph::EdgeTable pgraph::Graph::edge_table() {
    pgraph::Graph::EdgeTable tbl;
    for(auto [id, edge] : edges) {
        std::string ek_name = edge_kind_name(edge.kind);
        HetList<EdgeID, std::string, NodeID, NodeID> row{id, ek_name, edge.src, edge.dst};
        tbl << row;
    }
    return tbl;
}