#pragma once

#include <boost/graph/adjacency_list.hpp>

#include <boost/range/iterator_range.hpp>

namespace solver {
template <class VertexListGraph, class ColorMap>
static bool Validate(VertexListGraph const& g, ColorMap color)
{
    for (auto v: boost::make_iterator_range(boost::vertices(g))) {
        for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
            if (color[v] == color[u]) {
                return false;
            }
        }
    }
    return true;
}
} // namespace solver
