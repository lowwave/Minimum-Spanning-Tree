#include "graph.h"
#include <algorithm>
#include <iostream>

graph graph::calculate_minimum_spanning_tree() const {
    graph mst{};

    for (auto vertex : vertices()) mst.add_vertex(vertex); // populate the resulting MST with all vertices

    auto vertices_to_edges_map = get_edges_by_vertex();


    components_t components = mst.get_components(); // loads the set of components (a component is a set of vertices)

    auto vertices_to_components_map = mst.get_component_by_vertex(components);

    unsigned long last_components_size = 0;

    while (components.size() > 1) {
        if (components.size() == last_components_size)
            throw std::domain_error("MST could not be found!");
        else last_components_size = components.size();

        for (auto &component : components) {
            edge const *cheapest_edge = nullptr;

            for (auto vertex : component) { // searches for a minimal edge for each vertex of a component
                auto vertex_edges = vertices_to_edges_map[vertex];

                // removes all edges starting in a vertex which end in the same component
                for (auto iter = vertex_edges.begin(); iter != vertex_edges.end();) {
                    if (vertices_to_components_map[(*iter)->start] == vertices_to_components_map[(*iter)->end]) {
                        iter = vertex_edges.erase(iter);
                    } else {
                        ++iter;
                    }
                }

                // attempt to find the cheapest one of the remaining edges (which end in a different component)
                auto possible_cheapest_edge_iter = std::min_element(
                        vertex_edges.begin(),
                        vertex_edges.end(),
                        [](edge const *const &a, edge const *const &b) {
                            return (*a) < (*b);
                        });

                // if such an edge exists and it is cheaper than the cheapest one so far, mark it
                if (possible_cheapest_edge_iter != vertex_edges.end()) {
                    if (cheapest_edge == nullptr || **possible_cheapest_edge_iter < *cheapest_edge) {
                        cheapest_edge = *possible_cheapest_edge_iter;
                    }
                }
            }

            // after iterating through all vertices of a component, add the vertex to the MST
            if (cheapest_edge != nullptr) {
                auto x = edge(*cheapest_edge);
                mst.add_edge(x);
            }
        }

        // reload the list of components and the map of vertices to their component
        components = mst.get_components();
        vertices_to_components_map = mst.get_component_by_vertex(components);
    }

    return mst;
};

std::unique_ptr<graph> graph::calculate_minimum_spanning_tree_parallel() const {
    auto mst = std::make_unique<graph>();

    for (auto vertex : vertices()) { // populate the resulting MST with all vertices
        mst->add_vertex(vertex);
    }

    auto vertices_to_edges_map = get_edges_by_vertex();

    // loads the set of components (a component is a set of vertices)
    // this part is the most demanding in terms of performance, so we execute it in parallel as well
    auto components = mst->get_components_parallel();
    auto vertices_to_components_map = mst->get_component_by_vertex(components);

    unsigned long last_components_size = 0;

    while (components.size() > 1) {
        if (components.size() == last_components_size) {
            throw std::domain_error("MST could not be found!");
        } else {
            last_components_size = components.size();
        }

        std::vector<std::future<edge const *>> futures;
        futures.reserve(components.size());

        for (auto &component : components) {
            // execute the search for the cheapest edge in parallel, because the components do not rely on one another
            futures.push_back(std::async(
                    std::launch::async,
                    [&component, &mst, &vertices_to_edges_map, &vertices_to_components_map, this]() {
                        edge const *cheapest_edge = nullptr;

                        // attempts to find a minimal edge for each vertex of a component
                        for (auto &vertex : component) {
                            auto vertex_edges = vertices_to_edges_map[vertex];

                            // this removes all edges starting in a vertex which end in the same component
                            for (auto iter = vertex_edges.begin(); iter != vertex_edges.end();) {
                                if (vertices_to_components_map[(*iter)->start] ==
                                    vertices_to_components_map[(*iter)->end]) {
                                    iter = vertex_edges.erase(iter);
                                } else {
                                    ++iter;
                                }
                            }

                            // attempt to find the cheapest of the remaining edges (which end in a different component)
                            auto possible_cheapest_edge_iter = std::min_element(
                                    vertex_edges.begin(),
                                    vertex_edges.end(),
                                    [](edge const *const &a, edge const *const &b) {
                                        return (*a) < (*b);
                                    });

                            // if such an edge exists and it is cheaper than the cheapest one so far, mark it
                            if (possible_cheapest_edge_iter != vertex_edges.end()) {
                                if (cheapest_edge == nullptr || **possible_cheapest_edge_iter < *cheapest_edge) {
                                    cheapest_edge = *possible_cheapest_edge_iter;
                                }
                            }

                        }

                        // after iterating through all vertices of a component, return the cheapest edge
                        return cheapest_edge;
                    }));
        }

        // and add the cheapest edge for each component to the spanning tree
        for (auto &future : futures) {
            auto cheapest_edge = future.get();

            if (cheapest_edge != nullptr) {
                auto x = edge(*cheapest_edge);
                mst->add_edge(x);
            }
        }

        // reload the list of components and the map of vertices to their component
        components = mst->get_components_parallel();
        vertices_to_components_map = mst->get_component_by_vertex(components);
    }

    return mst;
};

vertex_to_edges_map const graph::get_edges_by_vertex() const {
    vertex_to_edges_map map{};

    for (auto &vertex: m_vertices)
        map[vertex] = {};

    for (const auto &m_edge : m_edges) {
        map[m_edge.start].push_back(&m_edge);
        map[m_edge.end].push_back(&m_edge);
    }

    return map;
};

components_t graph::get_components() const {
    components_t components{};

    auto edges_by_vertex = get_edges_by_vertex();

    for (auto &entry : edges_by_vertex) {
        vertices_t set{};
        populate_set_with_connected_vertices(edges_by_vertex, entry.first, set);
        components.insert(set);
    }

    return components;
}

std::map<vertex, vertices_t> const graph::get_component_by_vertex(components_t const &components) const {
    std::map<vertex, vertices_t> map;

    for (auto &component : components)
        for (auto vertex : component)
            map[vertex] = component;

    return map;
}

bool graph::has_edge(edge const &e) const {
    for (const auto &edge : m_edges)
        if ((e.start == edge.start && e.end == edge.end)
            || (e.start == edge.end && e.end == edge.start))
            return true;

    return false;
}

void graph::populate_set_with_connected_vertices(vertex_to_edges_map const &map, vertex v, vertices_t &ret) const {
    ret.insert(v);

    for (auto edge : map.at(v)) {
        if (ret.find(edge->start) == ret.end()) populate_set_with_connected_vertices(map, edge->start, ret);
        if (ret.find(edge->end) == ret.end()) populate_set_with_connected_vertices(map, edge->end, ret);
    }
}

components_t graph::get_components_parallel() const {
    components_t ret;

    auto edges_by_vertex = get_edges_by_vertex();
    std::vector<std::future<vertices_t>> futures;
    futures.reserve(edges_by_vertex.size());

    for (auto &entry : edges_by_vertex) {
        futures.push_back(std::async(
                std::launch::async,
                [&edges_by_vertex, &entry, this]() {
                    auto set = vertices_t();
                    populate_set_with_connected_vertices(edges_by_vertex, entry.first, set);
                    return set;
                }
        ));
    }

    for (auto &future : futures) {
        ret.insert(future.get());
    }

    return ret;
}
vertex graph::get_vertex_at(unsigned long index) const {
    auto start_it{m_vertices.begin()};
    std::advance(start_it, index);
    return *start_it;
}
