#ifndef MINIMUM_SPANNING_TREE_GRAPH_H
#define MINIMUM_SPANNING_TREE_GRAPH_H

#include <future>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "edge.h"

typedef std::set<edge> edges_t;
typedef std::set<vertex> vertices_t;
typedef vertices_t component_t;
typedef std::set<component_t> components_t;

typedef std::map<vertex, std::vector<const edge *>> vertex_to_edges_map;

class graph {
    edges_t m_edges{};
    vertices_t m_vertices{};

    /**
     * Returns a set of all components in the graph. Component is a set of connected vertices,
     * i.e. there is a path from every vertex to any other vertex via graph edges
     * @return Set of components
     */
    components_t get_components() const;
    components_t get_components_parallel() const;

    std::map<vertex, vertices_t> const get_component_by_vertex(components_t const &components) const;

    /**
     * Returns all vertices connected to a certain vertex by looking in the edges_by_vertex map
     *
     * Note: because the search is recursive and repetitions and loops may occur,
     * we need to keep track of all the vertices. That's why we pass the resulting set as an argument
     * instead of having it as a return value.
     * @param map
     * @param v
     * @param ret
     */
    void populate_set_with_connected_vertices(vertex_to_edges_map const &map, vertex v, vertices_t &ret) const;

public:
    graph() = default;

    inline std::pair<edges_t::iterator, bool> add_edge(const edge &e) { return m_edges.insert(e); }
    inline std::pair<vertices_t::iterator, bool> add_vertex(vertex v) { return m_vertices.insert(v); }

    inline edges_t const &edges() const { return m_edges; }
    inline vertices_t const &vertices() const { return m_vertices; }

    /**
     * Gets vertex ad index. Note that vertices are stored in a set.
     * Function advances begin() iterator of vertices set at index positions and returns the vertex.
     * If index is higher than the total set size, behaviour is undefined.
     * @param index Advance count from the start
     * @return Vertex at position
     */
    vertex get_vertex_at(unsigned long index) const;

    /**
     * Gets a map of edges per vertices, i.e., the edge and all the edges, connected to a vertex
     * @return Map of vertex -> connected edges
     */
    vertex_to_edges_map const get_edges_by_vertex() const;

    /**
     * Checks if the graph is connected (i.e. if there are no separated vertices).
     * Gets graph components and checks if the size is equal to one.
     * @return True if the graph is connected
     */
    inline bool is_connected() const { return get_components().size() == 1; }
    bool has_edge(edge const &e) const;

    graph calculate_minimum_spanning_tree() const;
    std::unique_ptr<graph> calculate_minimum_spanning_tree_parallel() const;
};

#endif //MINIMUM_SPANNING_TREE_GRAPH_H
