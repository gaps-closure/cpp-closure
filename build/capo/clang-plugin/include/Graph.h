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
        return nodes[id];
    }

    Edge& get_edge(EdgeID id) {
        return edges[id];
    }

    NodeID add_node(Node&& node) {
        nodes.push_back(node);
        size_t index = nodes.size() - 1;
        return index; 
    }

    EdgeID add_edge(Edge&& edge) {
        edges.push_back(edge);
        size_t index = edges.size() - 1;
        assert(nodes.find(edge.src_node()) != nodes.end());
        assert(nodes.find(edge.dst_node()) != nodes.end());
        return index; 
    }

protected:
    std::vector<Node> nodes;
    std::vector<Edge> edges;

};

};