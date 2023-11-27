#pragma once

#include <vector>
#include <map>
#include <optional>


namespace cle {

using NodeID = size_t;
using EdgeID = size_t;

template<typename Node, typename Edge>
class Graph {
public:

    Node& get_node(NodeID id) {
        return nodes.at(id);
    }

    Edge& get_edge(EdgeID id) {
        return edges.at(id);
    }

    virtual NodeID add_node(Node&& node) {
        nodes.insert(std::pair<NodeID, Node>(cur_node_id, node));
        cur_node_id++;
        return cur_node_id - 1;
    }

    void replace_node(NodeID id, Node node) {
        nodes.insert_or_assign(id, node);
    }

    void replace_edge(EdgeID id, Edge edge) {
        edges.insert_or_assign(id, edge);
    }

    EdgeID add_edge(Edge&& edge) {
        edges.insert(std::pair<EdgeID, Edge>(cur_edge_id, edge));
        cur_edge_id++;
        return cur_edge_id - 1;
    }

protected:
    NodeID cur_node_id = 0;
    EdgeID cur_edge_id = 0;
    std::map<NodeID, Node> nodes;
    std::map<EdgeID, Edge> edges;

};

};