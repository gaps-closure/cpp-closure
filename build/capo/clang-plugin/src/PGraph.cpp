#include "PGraph.h"

using namespace clang;
using namespace cle;
using namespace cle::pgraph;


std::string pgraph::node_kind_name(NodeKind kind) {
    switch(kind) {
    case DECL_VAR:
        return "Decl.Var";
    case DECL_FUN:
        return "Decl.Function";
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
    case STMT_FIELD:
        return "Stmt.Field";
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
    case STRUCT_FIELD:
        return "Struct.Field";
    case STRUCT_METHOD:
        return "Struct.Method";
    case STRUCT_CONSTRUCTOR:
        return "Struct.Constructor";
    case STRUCT_DESTRUCTOR:
        return "Struct.Method";
    case STRUCT_INHERIT:
        return "Struct.Inherit";
    case STRUCT_PARAM:
        return "Struct.Param";
    case STRUCT_CHILD:
        return "Struct.Child";
    case DATA_OBJECT:
        return "Data.Object";
    case DATA_DECL:
        return "Data.Decl";
    case DATA_DEFUSE:
        return "Data.DefUse";
    case DATA_ARGPASS:
        return "Data.ArgPass";
    case DATA_RETURN:
        return "Data.Return";
    case DATA_FIELDACCESS:
        return "Data.FieldAccess";
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

NodeID pgraph::Graph::add_method_decl(CXXMethodDecl* decl, NodeCtx ctx) {
    return add_function_like<CXXMethodDecl, DeclMethod>(decl, ctx);
}

NodeID pgraph::Graph::add_field_decl(FieldDecl* decl, NodeCtx ctx) {
    return add_node(Node(DeclField(decl), ctx));
}

template<typename ClangDecl, typename CLENode> 
NodeID pgraph::Graph::add_function_like(ClangDecl* decl, NodeCtx ctx) {
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
                replace_node(id, Node(CLENode(static_cast<ClangDecl*>(decl_with_body)), ctx));
                auto cid = add_stmt(decl_with_body->getBody(), ctx.set_parent_function(decl_with_body));
                add_edge(Edge(id, cid, CONTROL_ENTRY));
            }
            found_prev_decl = true;
            break;
        }
    }

    if(!found_prev_decl) {
        id = add_node(Node(CLENode(decl), ctx));
        auto decl_p = get_node(id).decl_fun.decl;

        size_t idx = 0;
        for(auto param : decl_p->parameters()) {
            auto pid = add_node(Node(DeclParam(param), ctx.set_parent_function(decl), idx++));
            add_edge(Edge(id, pid, STRUCT_PARAM));
        }

        if(decl->hasBody()) {

            auto cid = add_stmt(decl->getBody(), ctx.set_parent_function(decl));
            add_implicit_destructors(decl, decl->getBody(), ctx.set_parent_function(decl));
            add_edge(Edge(id, cid, CONTROL_ENTRY));
        }  
    }

    return id;

}

NodeID pgraph::Graph::add_constructor_decl(CXXConstructorDecl* decl, NodeCtx ctx) {
    return add_function_like<CXXConstructorDecl, DeclConstructor>(decl, ctx);
}

NodeID pgraph::Graph::add_destructor_decl(CXXDestructorDecl* decl, NodeCtx ctx) {
    return add_function_like<CXXDestructorDecl, DeclDestructor>(decl, ctx);
}

NodeID pgraph::Graph::add_compound_stmt(CompoundStmt* stmt, NodeCtx ctx) {
    auto id = add_node(Node(StmtCompound(stmt), ctx));
    for(auto child : stmt->children()) {
        auto cid = add_stmt(child, ctx);
        add_edge(Edge(id, cid, STRUCT_CHILD));
    }
    return id;
}

NodeID pgraph::Graph::add_ref_stmt(DeclRefExpr* stmt, NodeCtx ctx) {
    auto id = add_node(Node(StmtRef(stmt), ctx));
    if(named_decls.find(stmt->getFoundDecl()) != named_decls.end()) {
        auto did = named_decls[stmt->getFoundDecl()];
        add_edge(Edge(id, did, DATA_DEFUSE));
    }
    return id;
}


NodeID pgraph::Graph::add_decl_stmt(DeclStmt* stmt, NodeCtx ctx) {
    auto id = add_node(Node(StmtDecl(stmt), ctx));
    for(auto decl : stmt->decls()) {
        auto did = add_decl(decl, ctx);
        add_edge(Edge(id, did, DATA_DECL));
    }
    return id;
}

NodeID pgraph::Graph::add_other_stmt(Stmt* stmt, NodeCtx ctx) {
    auto id = add_node(Node(StmtOther(stmt), ctx));
    for(auto child : stmt->children()) {
        auto cid = add_stmt(child, ctx);
        add_edge(Edge(id, cid, STRUCT_CHILD));
    }
    return id;
}

NodeID pgraph::Graph::add_this_stmt(CXXThisExpr* stmt, NodeCtx ctx) {
    auto id = add_node(Node(StmtThis(stmt), ctx));
    return id;
}

NodeID pgraph::Graph::add_return_stmt(ReturnStmt* stmt, NodeCtx ctx) {
    auto id = add_node(Node(StmtReturn(stmt), ctx));

    for(auto child : stmt->children()) {
        auto cid = add_stmt(child, ctx);
        add_edge(Edge(id, cid, STRUCT_CHILD));
    }

    return id;
}

NodeID pgraph::Graph::add_call_stmt(CallExpr* stmt, NodeCtx ctx) {
    auto id = add_node(Node(StmtCall(stmt), ctx));

    if(auto decl = stmt->getDirectCallee()) {
        NodeID fid;
        if(named_decls.find(decl) != named_decls.end()) {
            fid = named_decls[decl];
        } else {
            fid = add_function_decl(decl, ctx);
        }
        add_edge(Edge(id, fid, CONTROL_FUNCTION_INVOCATION));
    }

    size_t idx = 0;
    for(auto arg : stmt->arguments()) {
        auto aid = add_stmt(arg, ctx);
        nodes.find(aid)->second.param_idx = idx++;
        add_edge(Edge(id, aid, DATA_ARGPASS));
    }

    return id;
}


NodeID pgraph::Graph::add_member_call_stmt(CXXMemberCallExpr* stmt, NodeCtx ctx) {
    auto id = add_node(Node(StmtCall(stmt), ctx));

    // stmt->getPreArg()
    if(auto decl = stmt->getMethodDecl()) {
        NodeID fid;
        if(named_decls.find(decl) != named_decls.end()) {
            fid = named_decls[decl];
        } else {
            fid = add_method_decl(decl, ctx);
        }
        auto node = get_node(fid);
        if(node.kind == DECL_DESTRUCTOR) {
            add_edge(Edge(id, fid, CONTROL_DESTRUCTOR_INVOCATION));
        } else {
            add_edge(Edge(id, fid, CONTROL_METHOD_INVOCATION));
        }
    }

    size_t idx = 0;
    for(auto arg : stmt->arguments()) {
        auto aid = add_stmt(arg, ctx);
        nodes.find(aid)->second.param_idx = idx++;
        add_edge(Edge(id, aid, DATA_ARGPASS));
    }

    if(auto obj = stmt->getImplicitObjectArgument()) {
        auto oid = add_stmt(obj, ctx);
        add_edge(Edge(id, oid, DATA_OBJECT));
    }

    return id;

}


NodeID pgraph::Graph::add_constructor_call_stmt(CXXConstructExpr* stmt, NodeCtx ctx) {
    auto id = add_node(Node(StmtConstructor(stmt), ctx));

    NodeID cid;
    {
        auto decl = stmt->getConstructor();
        if(named_decls.find(decl) != named_decls.end()) {
            cid = named_decls[decl];
        } else {
            cid = add_constructor_decl(stmt->getConstructor(), ctx);
        }
    }

    add_edge(Edge(id, cid, CONTROL_CONSTRUCTOR_INVOCATION));

    size_t idx = 0; 
    for(auto arg : stmt->arguments()) {
        auto aid = add_stmt(arg, ctx);
        nodes.find(aid)->second.param_idx = idx++;
        add_edge(Edge(id, aid, DATA_ARGPASS));
    }

    return id;
}

NodeID pgraph::Graph::add_member_stmt(MemberExpr *stmt, NodeCtx ctx) {
    auto id = add_node(Node(StmtField(stmt), ctx));
    if(auto obj = stmt->getBase()) {
        auto oid = add_stmt(obj, ctx);
        add_edge(Edge(id, oid, DATA_OBJECT));
    }
    if(auto mem = dyn_cast<FieldDecl>(stmt->getMemberDecl())) {
        if(named_decls.find(mem) != named_decls.end()) {
            NodeID mid = named_decls[mem];
            add_edge(Edge(id, mid, DATA_FIELDACCESS));
        } else {
            llvm::errs() << "could not find member for field access" << "\n";
        } 
    }
    return id;
}


NodeID pgraph::Graph::add_stmt(Stmt* stmt, NodeCtx ctx) {
    switch(stmt->getStmtClass()) {
        case Stmt::CompoundStmtClass:
            return add_compound_stmt(dyn_cast<CompoundStmt>(stmt), ctx);
        case Stmt::DeclStmtClass:
            return add_decl_stmt(dyn_cast<DeclStmt>(stmt), ctx);
        case Stmt::ReturnStmtClass:
            return add_return_stmt(dyn_cast<ReturnStmt>(stmt), ctx);
        case Stmt::DeclRefExprClass: 
            return add_ref_stmt(dyn_cast<DeclRefExpr>(stmt), ctx);
        case Stmt::CallExprClass:
            return add_call_stmt(dyn_cast<CallExpr>(stmt), ctx);
        case Stmt::CXXConstructExprClass:
            return add_constructor_call_stmt(dyn_cast<CXXConstructExpr>(stmt), ctx);
        case Stmt::CXXMemberCallExprClass:
            return add_member_call_stmt(dyn_cast<CXXMemberCallExpr>(stmt), ctx);
        case Stmt::CXXThisExprClass:
            return add_this_stmt(dyn_cast<CXXThisExpr>(stmt), ctx);
        case Stmt::MemberExprClass:
            return add_member_stmt(dyn_cast<MemberExpr>(stmt), ctx);
        default:
            return add_other_stmt(stmt, ctx);
    }
}

void pgraph::Graph::add_implicit_destructors(Decl* decl, Stmt* body, NodeCtx ctx) {
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
                        if(edge.kind == EdgeKind::STRUCT_CHILD && edge.dst == id) {
                            parent_id = edge.src;
                            break;
                        }
                    }
                    if(parent_id) {
                        auto did = add_node(Node(StmtImplicitDestructor(*dtor), ctx));
                        add_edge(Edge(*parent_id, did, EdgeKind::STRUCT_CHILD));
                        add_edge(Edge(did, clang_to_node_id[dtor->getDestructorDecl(*ast_ctx)->getID()], EdgeKind::CONTROL_DESTRUCTOR_INVOCATION));
                    }
                }
            } 
        }
    }

}

NodeID pgraph::Graph::add_function_decl(FunctionDecl* decl, NodeCtx ctx) {
    return add_function_like<FunctionDecl, DeclFun>(decl, ctx);
}

NodeID pgraph::Graph::add_record_decl(CXXRecordDecl* decl, NodeCtx ctx) {

    NodeID id; 
    bool found_prev_decl = false;
    for(auto redecl : decl->redecls()) {
        if(named_decls.find(redecl) != named_decls.end()) {
            id = named_decls[redecl];
            auto node = get_node(id);
            if(!node.decl_record.decl->hasDefinition() && decl->hasDefinition()) {
                replace_node(id, Node(DeclRecord(decl), ctx));
            }
            found_prev_decl = true;
            break;
        }
    }
    if(!found_prev_decl)
        id = add_node(Node(DeclRecord(decl), ctx));

    if(!decl->hasDefinition())
        return id;

    for(auto base : decl->bases()) {
        auto ty = base.getType().getTypePtr();
        if(record_definitions.find(ty) != record_definitions.end()) {
            add_edge(Edge(id, record_definitions[ty], STRUCT_INHERIT));
        }
    }
    record_definitions.insert(std::pair(decl->getTypeForDecl(), id));

    for(auto fdecl : decl->fields()) {
        auto fid = add_field_decl(fdecl, ctx.set_parent_class(decl));
        add_edge(Edge(id, fid, STRUCT_FIELD));
    }

    for(auto mdecl : decl->methods()) {
        if(clang::isa<CXXConstructorDecl>(mdecl) || clang::isa<CXXDestructorDecl>(mdecl))
            continue;
        auto mid = add_method_decl(mdecl, ctx.set_parent_class(decl));
        add_edge(Edge(id, mid, STRUCT_METHOD));
    }

    for(auto cdecl : decl->ctors()) {
        auto cid = add_constructor_decl(cdecl, ctx.set_parent_class(decl));
        add_edge(Edge(id, cid, STRUCT_CONSTRUCTOR));
    }

    if(decl->hasUserDeclaredDestructor()) {
        auto ddecl = decl->getDestructor();
        auto did = add_destructor_decl(ddecl, ctx.set_parent_class(decl));
        add_edge(Edge(id, did, STRUCT_CONSTRUCTOR));
    }
    return id;
}

NodeID pgraph::Graph::add_decl(Decl* decl, NodeCtx ctx) {
    switch(decl->getKind()) {
        case Decl::Var: 
            return add_node(Node(DeclVar(dyn_cast<VarDecl>(decl)), ctx));
        case Decl::Function:
            return add_function_decl(dyn_cast<FunctionDecl>(decl), ctx);
        case Decl::CXXRecord:
            return add_record_decl(dyn_cast<CXXRecordDecl>(decl), ctx);
        case Decl::CXXMethod:
            return add_method_decl(dyn_cast<CXXMethodDecl>(decl), ctx);
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

std::map<NodeID, NodeID> pgraph::Graph::reorder_nodes() {
    std::map<NodeID, NodeID> map;
    std::map<NodeKind, std::set<NodeID>> buckets;
    for(auto [id, node] : nodes) {
        buckets[node.kind].insert(id);
    }
    NodeID idx = 1;
    for(auto [kind, bucket] : buckets) {
        for(auto id : bucket) {
            map[id] = idx; 
            idx++;
        }
    }
    return map;
}

std::map<EdgeID, EdgeID> pgraph::Graph::reorder_edges() {
    std::map<EdgeID, EdgeID> map;
    std::map<EdgeKind, std::set<NodeID>> buckets;
    for(auto [id, edge] : edges) {
        buckets[edge.kind].insert(id);
    }
    EdgeID idx = 1;
    for(auto [kind, bucket] : buckets) {
        for(auto id : bucket) {
            map[id] = idx; 
            idx++;
        }
    }
    return map;
}


std::optional<NodeID> pgraph::Graph::find_node(NamedDecl* decl) {
    std::optional<NodeID> parent_id = std::nullopt; 
    if(decl) {
        if(named_decls.find(decl) != named_decls.end()) {
            parent_id = named_decls[decl];
        }
    }
    return parent_id;
}

pgraph::Graph::NodeTable pgraph::Graph::node_table() {
    pgraph::Graph::NodeTable tbl;
    std::map<NodeID, NodeID> node_id_map = reorder_nodes(); 
    std::vector<std::tuple<NodeID, NodeID>> reordered_nodes;
    for(auto [id, node] : nodes) {
        if(node_id_map[id] != 0)
            reordered_nodes.push_back(std::tuple<NodeID, NodeID>(node_id_map[id], id));
    }
    std::sort(reordered_nodes.begin(), reordered_nodes.end());
    for(auto [r_id, id] : reordered_nodes) {
        auto node = nodes.find(id)->second;
        std::string name = node.qualified_name().value_or("");
        std::string nk_name = node_kind_name(node.kind);
        std::string annotation; 
        if(node.named_decl()) {
            if(node.named_decl().value()->hasAttr<clang::AnnotateAttr>()) {
                auto attr = node.named_decl().value()->getAttr<clang::AnnotateAttr>();
                annotation = attr->getAnnotation();
            }
        }

        std::optional<NodeID> parent_id = find_node(node.ctx.parent_decl); 
        std::optional<NodeID> class_id = find_node(node.ctx.parent_class); 
        std::optional<NodeID> function_id = find_node(node.ctx.parent_function); 

        std::string parent_id_str, class_id_str, function_id_str, param_idx_str;

        auto range = node.source_range();  
        auto start_loc = range.getBegin();
        auto &manager = ast_ctx->getSourceManager();
        auto filename = manager.getFilename(start_loc).str();
        auto start_off = manager.getFileOffset(start_loc);

        auto end_loc = range.getEnd();
        auto end_off = manager.getFileOffset(end_loc);
 
        if(parent_id)
            parent_id_str = std::to_string(node_id_map[*parent_id]);
        if(class_id)
            class_id_str = std::to_string(node_id_map[*class_id]);
        if(function_id)
            function_id_str = std::to_string(node_id_map[*function_id]);
        if(node.param_idx)
            param_idx_str = std::to_string(*(node.param_idx));
        HetList<cle::NodeID, std::string, std::string, std::string, 
            std::string, std::string, std::string,
            std::string,
            std::string, unsigned int, unsigned int> 
            row{r_id, nk_name, name, annotation, 
                parent_id_str, class_id_str, function_id_str,
                param_idx_str,
                filename, start_off, end_off}; 

        tbl << row;
    }
    return tbl;
}


pgraph::Graph::EdgeTable pgraph::Graph::edge_table() {
    pgraph::Graph::EdgeTable tbl;
    std::map<NodeID, NodeID> node_id_map = reorder_nodes(); 
    std::map<EdgeID, EdgeID> edge_id_map = reorder_edges(); 

    std::vector<std::tuple<NodeID, NodeID>> reordered_edges;
    for(auto [id, edge] : edges) {
        if(edge_id_map[id] != 0)
            reordered_edges.push_back(std::tuple<NodeID, NodeID>(edge_id_map[id], id));
    }
    std::sort(reordered_edges.begin(), reordered_edges.end());
    
    for(auto [r_id, id] : reordered_edges) {
        auto edge = edges.find(id)->second;
        std::string ek_name = edge_kind_name(edge.kind);
        HetList<EdgeID, std::string, NodeID, NodeID> row{r_id, ek_name, node_id_map[edge.src], node_id_map[edge.dst]};
        tbl << row;
    }
    return tbl;
}