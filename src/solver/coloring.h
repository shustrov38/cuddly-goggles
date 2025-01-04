#pragma once

#include "config.h"
#include "heuristics/DSatur.h"

namespace solver {
template <class VertexListGraph, class P, class T, class R>
static auto Process(VertexListGraph const& g, boost::bgl_named_params<P, T, R> const& params, Config config)
{
    if (config < __DSATUR_BOUND) {
        return heuristics::DSaturSequentialVertexColoring(g, params, config);
    }
    throw std::invalid_argument("unknown config parameter");
}

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
