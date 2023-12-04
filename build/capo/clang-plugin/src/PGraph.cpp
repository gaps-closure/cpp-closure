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

NodeID pgraph::Graph::add_method_decl(CXXMethodDecl* decl) {
    NodeID id;
    bool found_prev_decl = false;
    for(auto redecl : decl->redecls()) {
        if(named_decls.find(redecl) != named_decls.end()) {
            id = named_decls[redecl];
            auto node = get_node(id);
            if(!node.decl_fun.decl->hasBody() && decl->hasBody()) {
                replace_node(id, Node(DeclFun(decl)));
                auto cid = add_stmt(decl->getBody());
                add_edge(Edge(id, cid, CONTROL_ENTRY));
            }
            found_prev_decl = true;
            break;
        }
    }
    if(!found_prev_decl)
        id = add_node(Node(DeclMethod(decl)));

    return id;
}

NodeID pgraph::Graph::add_field_decl(FieldDecl* decl) {
    return add_node(Node(DeclField(decl)));
}

template<typename ClangDecl, typename CLENode> 
NodeID pgraph::Graph::add_function_like(ClangDecl* decl) {
    NodeID id;
    bool found_prev_decl = false;
    for(auto redecl : decl->redecls()) {
        if(named_decls.find(redecl) != named_decls.end()) {
            id = named_decls[redecl];
            auto node = get_node(id);
            if(!node.decl_fun.decl->hasBody() && decl->hasBody()) {
                replace_node(id, Node(CLENode(decl)));
                auto cid = add_stmt(decl->getBody());
                add_edge(Edge(id, cid, CONTROL_ENTRY));
            }
            found_prev_decl = true;
            break;
        }
    }

    if(!found_prev_decl) {
        id = add_node(Node(CLENode(decl)));
        auto decl_p = get_node(id).decl_fun.decl;

        for(auto param : decl_p->parameters()) {
            auto pid = add_node(Node(DeclParam(param)));
            add_edge(Edge(id, pid, DATA_PARAM));
        }

        if(decl->hasBody()) {

            auto cid = add_stmt(decl->getBody());
            add_implicit_destructors(decl, decl->getBody());
            add_edge(Edge(id, cid, CONTROL_ENTRY));
        }  
    }

    return id;

}

NodeID pgraph::Graph::add_constructor_decl(CXXConstructorDecl* decl) {
    return add_function_like<CXXConstructorDecl, DeclConstructor>(decl);
}

NodeID pgraph::Graph::add_destructor_decl(CXXDestructorDecl* decl) {
    return add_function_like<CXXDestructorDecl, DeclDestructor>(decl);
}

NodeID pgraph::Graph::add_compound_stmt(CompoundStmt* stmt) {
    auto id = add_node(Node(StmtCompound(stmt)));
    for(auto child : stmt->children()) {
        auto cid = add_stmt(child);
        add_edge(Edge(id, cid, CHILD));
    }
    return id;
}

NodeID pgraph::Graph::add_ref_stmt(DeclRefExpr* stmt) {
    auto id = add_node(Node(StmtRef(stmt)));
    if(named_decls.find(stmt->getFoundDecl()) != named_decls.end()) {
        auto did = named_decls[stmt->getFoundDecl()];
        add_edge(Edge(id, did, DATA_DEFUSE));
    }
    return id;
}


NodeID pgraph::Graph::add_decl_stmt(DeclStmt* stmt) {
    auto id = add_node(Node(StmtDecl(stmt)));
    for(auto decl : stmt->decls()) {
        auto did = add_decl(decl);
        add_edge(Edge(id, did, DATA_DECL));
    }
    return id;
}

NodeID pgraph::Graph::add_other_stmt(Stmt* stmt) {
    auto id = add_node(Node(StmtOther(stmt)));
    for(auto child : stmt->children()) {
        auto cid = add_stmt(child);
        add_edge(Edge(id, cid, CHILD));
    }
    return id;
}

NodeID pgraph::Graph::add_return_stmt(ReturnStmt* stmt) {
    auto id = add_node(Node(StmtReturn(stmt)));

    for(auto child : stmt->children()) {
        auto cid = add_stmt(child);
        add_edge(Edge(id, cid, CHILD));
    }
   
    return id;
}

NodeID pgraph::Graph::add_call_stmt(CallExpr* stmt) {
    auto id = add_node(Node(StmtCall(stmt)));

    if(auto decl = stmt->getDirectCallee()) {
        NodeID fid;
        if(named_decls.find(decl) != named_decls.end()) {
            fid = named_decls[decl];
        } else {
            fid = add_function_decl(decl);
        }
        add_edge(Edge(id, fid, CONTROL_FUNCTION_INVOCATION));
    }

    for(auto arg : stmt->arguments()) {
        auto aid = add_stmt(arg);
        add_edge(Edge(id, aid, DATA_ARGPASS));
    }

    return id;
}


NodeID pgraph::Graph::add_member_call_stmt(CXXMemberCallExpr* stmt) {
    auto id = add_node(Node(StmtCall(stmt)));

    // stmt->getPreArg()
    if(auto decl = stmt->getMethodDecl()) {
        NodeID fid;
        if(named_decls.find(decl) != named_decls.end()) {
            fid = named_decls[decl];
        } else {
            fid = add_method_decl(decl);
        }
        auto node = get_node(fid);
        if(node.kind == DECL_DESTRUCTOR) {
            add_edge(Edge(id, fid, CONTROL_DESTRUCTOR_INVOCATION));
        } else {
            add_edge(Edge(id, fid, CONTROL_METHOD_INVOCATION));
        }
    }

    for(auto arg : stmt->arguments()) {
        auto aid = add_stmt(arg);
        add_edge(Edge(id, aid, DATA_ARGPASS));
    }

    if(auto obj = stmt->getImplicitObjectArgument()) {
        auto oid = add_stmt(obj);
        add_edge(Edge(id, oid, DATA_OBJECT));
    }

    return id;

}


NodeID pgraph::Graph::add_constructor_call_stmt(CXXConstructExpr* stmt) {
    auto id = add_node(Node(StmtConstructor(stmt)));

    NodeID cid;
    {
        auto decl = stmt->getConstructor();
        if(named_decls.find(decl) != named_decls.end()) {
            cid = named_decls[decl];
        } else {
            cid = add_constructor_decl(stmt->getConstructor());
        }
    }

    add_edge(Edge(id, cid, CONTROL_CONSTRUCTOR_INVOCATION));

    for(auto arg : stmt->arguments()) {
        auto aid = add_stmt(arg);
        add_edge(Edge(id, aid, DATA_ARGPASS));
    }

    return id;
}


NodeID pgraph::Graph::add_stmt(Stmt* stmt) {
    switch(stmt->getStmtClass()) {
        case Stmt::CompoundStmtClass:
            return add_compound_stmt(dyn_cast<CompoundStmt>(stmt));
        case Stmt::DeclStmtClass:
            return add_decl_stmt(dyn_cast<DeclStmt>(stmt));
        case Stmt::ReturnStmtClass:
            return add_return_stmt(dyn_cast<ReturnStmt>(stmt));
        case Stmt::DeclRefExprClass: 
            return add_ref_stmt(dyn_cast<DeclRefExpr>(stmt));
        case Stmt::CallExprClass:
            return add_call_stmt(dyn_cast<CallExpr>(stmt));
        case Stmt::CXXConstructExprClass:
            return add_constructor_call_stmt(dyn_cast<CXXConstructExpr>(stmt));
        case Stmt::CXXMemberCallExprClass:
            return add_member_call_stmt(dyn_cast<CXXMemberCallExpr>(stmt));
        default:
            return add_other_stmt(stmt);
    }
}

void pgraph::Graph::add_implicit_destructors(Decl* decl, Stmt* body) {
    clang::CFG::BuildOptions opts;
    opts.AddImplicitDtors = true;
    opts.AddScopes = true;
    opts.AddTemporaryDtors = true;
    std::unique_ptr<clang::CFG> cfg(
        clang::CFG::buildCFG(decl, decl->getBody(), ast_ctx, opts)
    );
    cfg->dump(LangOptions(), true);
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
                        auto did = add_node(Node(StmtImplicitDestructor(*dtor)));
                        add_edge(Edge(*parent_id, did, EdgeKind::CHILD));
                        add_edge(Edge(did, clang_to_node_id[dtor->getDestructorDecl(*ast_ctx)->getID()], EdgeKind::CONTROL_DESTRUCTOR_INVOCATION));
                    }
                }
            } 
        }
    }

}

NodeID pgraph::Graph::add_function_decl(FunctionDecl* decl) {
    return add_function_like<FunctionDecl, DeclFun>(decl);
}

NodeID pgraph::Graph::add_record_decl(CXXRecordDecl* decl, bool top_level) {

    NodeID id; 
    bool found_prev_decl = false;
    for(auto redecl : decl->redecls()) {
        if(named_decls.find(redecl) != named_decls.end()) {
            id = named_decls[redecl];
            auto node = get_node(id);
            if(!node.decl_record.decl->hasDefinition() && decl->hasDefinition()) {
                replace_node(id, Node(DeclRecord(decl)));
            }
            found_prev_decl = true;
            break;
        }
    }
    if(!found_prev_decl)
        id = add_node(Node(DeclRecord(decl, top_level)));

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
        auto fid = add_field_decl(fdecl);
        add_edge(Edge(id, fid, RECORD_FIELD));
    }

    for(auto mdecl : decl->methods()) {
        if(clang::isa<CXXConstructorDecl>(mdecl) || clang::isa<CXXDestructorDecl>(mdecl))
            continue;
        auto mid = add_method_decl(mdecl);
        add_edge(Edge(id, mid, RECORD_METHOD));
    }

    for(auto cdecl : decl->ctors()) {
        auto cid = add_constructor_decl(cdecl);
        add_edge(Edge(id, cid, RECORD_CONSTRUCTOR));
    }

    if(decl->hasUserDeclaredDestructor()) {
        auto ddecl = decl->getDestructor();
        auto did = add_destructor_decl(ddecl);
        add_edge(Edge(id, did, RECORD_CONSTRUCTOR));
    }
    return id;
}

NodeID pgraph::Graph::add_decl(Decl* decl, bool top_level) {
    switch(decl->getKind()) {
        case Decl::Var: 
            return add_node(Node(DeclVar(dyn_cast<VarDecl>(decl), top_level)));
        case Decl::Function:
            return add_function_decl(dyn_cast<FunctionDecl>(decl));
        case Decl::CXXRecord:
            return add_record_decl(dyn_cast<CXXRecordDecl>(decl));
        case Decl::CXXMethod:
            return add_method_decl(dyn_cast<CXXMethodDecl>(decl));
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
        default:
            return -1;
    }
}


std::optional<SDecl*> pgraph::Node::as_sdecl() {
    switch(kind) {
        case NodeKind::DECL_VAR:
            return &decl_var;
        case NodeKind::DECL_FUN:
            return &decl_fun;
        case NodeKind::DECL_RECORD:
            return &decl_record;
        case NodeKind::DECL_FIELD:
            return &decl_field;
        case NodeKind::DECL_METHOD:
            return &decl_method;
        case NodeKind::DECL_PARAM:
            return &decl_param;
        case NodeKind::DECL_CONSTRUCTOR:
            return &decl_constructor;
        case NodeKind::DECL_DESTRUCTOR:
            return &decl_destructor;
        default:
            return std::nullopt;
    }
}

Table<NodeID, std::string, std::string, std::string, std::string> pgraph::Graph::node_table() {
    Table<NodeID, std::string, std::string, std::string, std::string> tbl;
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

        std::optional<NodeID> ctx_id = std::nullopt;
        if(auto decl = node.named_decl()) {
            auto ctx = (*decl)->getDeclContext();
            if(auto named_ctx = dyn_cast<NamedDecl>(ctx)) {
                if(named_decls.find(named_ctx) != named_decls.end()) {
                    ctx_id = named_decls[named_ctx];
                }
            }
        }
        std::string ctx_id_str;
        if(ctx_id)
            ctx_id_str = std::to_string(*ctx_id);
        HetList<cle::NodeID, std::string, std::string, std::string, std::string> row{id, nk_name, name, annotation, ctx_id_str}; 

        tbl << row;
    }
    return tbl;
}


Table<EdgeID, std::string, NodeID, NodeID> pgraph::Graph::edge_table() {
    Table<EdgeID, std::string, NodeID, NodeID> tbl;
    for(auto [id, edge] : edges) {
        std::string ek_name = edge_kind_name(edge.kind);
        HetList<EdgeID, std::string, NodeID, NodeID> row{id, ek_name, edge.src, edge.dst};
        tbl << row;
    }
    return tbl;
}