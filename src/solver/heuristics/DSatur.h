#pragma once

#include <boost/graph/connected_components.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include <boost/range/iterator_range.hpp>

#include <boost/heap/fibonacci_heap.hpp>

#include "vertex_info.h"
#include "selectors.h"

#include "../config.h"

namespace solver::heuristics {
namespace impl {
template <typename VertexListGraph, class Order, class ColorMap, class Info>
typename boost::property_traits<ColorMap>::value_type
DSaturCore(VertexListGraph const& g, Order order, ColorMap color, typename ICandidateSelector<Info>::Ptr selector)
{
    using GraphTraits = boost::graph_traits<VertexListGraph>;
    using SizeType = GraphTraits::vertices_size_type;
    using ColorType = boost::property_traits<ColorMap>::value_type;

    ColorType maxColor = 0;
    auto const n = boost::num_vertices(g);

    std::vector<SizeType> mark(n, std::numeric_limits<ColorType>::max());

    std::vector<Info> vertexInfo(n);
    selector->Init(n, vertexInfo);

    for (auto v: boost::make_iterator_range(boost::vertices(g))) {
        SizeType i = order[v];

        auto& vi = vertexInfo[i];
        vi.index = i;
        vi.vertex = v;
        vi.degree = boost::out_degree(v, g);
        vi.saturation.clear();
        vi.colored = false;
        
        boost::put(color, v, 0);
        selector->Push(i);
    }

    for (SizeType _ = 0; _ < n; ++_) {
        auto i = selector->Pop();
        auto v = vertexInfo[i].vertex;

        for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
            auto j = order[u];
            if (vertexInfo[j].colored) {
                mark[boost::get(color, u)] = i;
            }
        }

        ColorType nextColor = 0;
        while (nextColor < maxColor && mark[nextColor] == i) {
            ++nextColor;
        }

        if (nextColor == maxColor) {
            ++maxColor;
        }

        boost::put(color, v, nextColor);
        vertexInfo[i].colored = true;

        for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
            auto j = order[u];
            if (!vertexInfo[j].colored) {
                vertexInfo[j].saturation.emplace(nextColor);
                selector->Update(j);
            }
        }
    }

    return maxColor;
}
} // namespace impl

template <typename VertexListGraph, class P, class T, class R>
auto DSaturSequentialVertexColoring(
    VertexListGraph const& g, boost::bgl_named_params<P, T, R> const& params, Config config
)
{
    auto color = boost::get_param(params, boost::vertex_color);
    auto order = boost::get_param(params, boost::vertex_index);

    using Info = VertexInfo<
        typename boost::graph_traits<VertexListGraph>::vertices_size_type,
        typename boost::graph_traits<VertexListGraph>::vertex_descriptor,
        typename boost::property_traits<decltype(color)>::value_type
    >;

    typename ICandidateSelector<Info>::Ptr selector;
    if (config == DSATUR) {
        selector = std::make_unique<DenseCandidateSelector<Info>>();
    } else if (config == DSATUR_FIBONACCI_HEAP) {
        selector = std::make_unique<SparseCandidateSelector<Info>>();
    } else {
        throw std::invalid_argument("unknown config parameter");
    }

    return impl::DSaturCore<VertexListGraph, decltype(order), decltype(color), Info>(
        g, order, color, std::move(selector)
    );
}
} // namespace solver::heuristics