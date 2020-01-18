#ifndef MINIMUM_SPANNING_TREE_EDGE_H
#define MINIMUM_SPANNING_TREE_EDGE_H

#include <tuple>

typedef unsigned long vertex;

struct edge {
    const vertex start;
    const vertex end;
    const unsigned long weight;

    constexpr edge(vertex start, vertex end, unsigned long weight) : start{start}, end{end}, weight{weight} {};
    constexpr edge(edge const &other) = default;

    constexpr inline bool operator<(edge const &e) const {
        return std::tie(weight, start, end) < std::tie(e.weight, e.start, e.end);
    }
    constexpr inline bool operator==(edge const &e) const {
        return (weight == e.weight) &&
               (((start == e.start) && (end == e.end)) || ((start == e.end) && (end == e.start)));
    }
};

#endif //MINIMUM_SPANNING_TREE_EDGE_H
