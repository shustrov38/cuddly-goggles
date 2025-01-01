#pragma once

#include <boost/graph/connected_components.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include <boost/range/iterator_range.hpp>

#include <boost/heap/fibonacci_heap.hpp>

namespace heuristics {
namespace detail {

} // namespace detail

namespace impl {
template <typename VertexListGraph, class Order, class ColorMap>
typename boost::property_traits<ColorMap>::value_type
DSaturSequentialVertexColoring(VertexListGraph const& g, Order order, ColorMap color)
{
    using GraphTraits = boost::graph_traits<VertexListGraph>;
    using SizeType = GraphTraits::vertices_size_type;
    using ColorType = boost::property_traits<ColorMap>::value_type;
    using Vertex = GraphTraits::vertex_descriptor;

    ColorType maxColor = 0;
    auto const n = boost::num_vertices(g);

    std::vector<SizeType> mark(n, std::numeric_limits<ColorType>::max BOOST_PREVENT_MACRO_SUBSTITUTION ());

    std::unordered_map<SizeType, Vertex> vertices;

    std::vector<SizeType> degrees(n);

    std::unordered_set<SizeType> uncoloredVertices;
    std::vector<std::unordered_set<ColorType>> neighbourColors(n);

    for (auto v: boost::make_iterator_range(boost::vertices(g))) {
        auto i = order[v];

        vertices[i] = v;

        boost::put(color, v, 0);
        uncoloredVertices.emplace(i);

        degrees[i] = boost::out_degree(v, g);
    }

    std::vector<SizeType> saturation(n);
    for (auto v: boost::make_iterator_range(boost::vertices(g))) {
        auto i = order[v];
        saturation[i] = neighbourColors[i].size();
    }

    auto findMaxSaturation = [&uncoloredVertices, &saturation, &degrees, &vertices]() {
        SizeType maxSaturation = 0;

        for (auto i: uncoloredVertices) {
            if (saturation[i] > maxSaturation) {
                maxSaturation = saturation[i];
            }
        }

        std::set<std::pair<SizeType, SizeType>> tieBreaker; // pair (degree, index)
        for (auto i: uncoloredVertices) {
            if (saturation[i] == maxSaturation) {
                tieBreaker.emplace(degrees[i], i);
            }
        }

        return vertices[tieBreaker.rbegin()->second];
    };

    while (!uncoloredVertices.empty()) {
        auto cur = findMaxSaturation();

        auto i = order[cur];
        for (auto v: boost::make_iterator_range(boost::adjacent_vertices(cur, g))) {
            mark[boost::get(color, v)] = i;
        }

        ColorType nextColor = 0;
        while (nextColor < maxColor && mark[nextColor] == static_cast<SizeType>(i)) {
            ++nextColor;
        }

        if (nextColor == maxColor) {
            ++maxColor;
        }

        boost::put(color, cur, nextColor);
        for (auto v: boost::make_iterator_range(boost::adjacent_vertices(cur, g))) {
            auto j = order[v];

            neighbourColors[j].emplace(nextColor);
            saturation[j] = neighbourColors[j].size();
        }

        uncoloredVertices.erase(i);
    }

    return maxColor;
}

template <typename VertexListGraph, class Order, class ColorMap>
typename boost::property_traits<ColorMap>::value_type
DSaturSparseSequentialVertexColoring(VertexListGraph const& g, Order order, ColorMap color)
{
    using GraphTraits = boost::graph_traits<VertexListGraph>;
    using SizeType = GraphTraits::vertices_size_type;
    using ColorType = boost::property_traits<ColorMap>::value_type;
    using Vertex = GraphTraits::vertex_descriptor;

    ColorType maxColor = 0;
    auto const n = boost::num_vertices(g);

    std::vector<SizeType> mark(n, std::numeric_limits<ColorType>::max BOOST_PREVENT_MACRO_SUBSTITUTION ());

    std::unordered_map<SizeType, Vertex> vertices;

    std::vector<SizeType> degrees(n);
    std::vector<SizeType> saturation(n);
    std::vector<std::unordered_set<ColorType>> neighbourColors(n);

    using Node = std::tuple<SizeType, SizeType, SizeType>; // tuple [saturation, degree, index]
    using Heap = boost::heap::fibonacci_heap<Node, boost::heap::compare<std::greater<Node>>>;
    using Handle = Heap::handle_type; 
    Heap uncoloredVertices;
    std::vector<Handle> heapHandles(n);

    for (auto v: boost::make_iterator_range(boost::vertices(g))) {
        auto i = order[v];

        vertices[i] = v;

        boost::put(color, v, 0);

        degrees[i] = boost::out_degree(v, g);
        saturation[i] = 0;

        heapHandles[i] = uncoloredVertices.emplace(saturation[i], degrees[i], i);
    }

    std::vector<bool> used(n);
    while (!uncoloredVertices.empty()) {
        auto cur = vertices[std::get<2>(uncoloredVertices.top())];
        uncoloredVertices.pop();

        auto i = order[cur];
        used[i] = true;
        for (auto v: boost::make_iterator_range(boost::adjacent_vertices(cur, g))) {
            mark[boost::get(color, v)] = i;
        }

        ColorType nextColor = 0;
        while (nextColor < maxColor && mark[nextColor] == static_cast<SizeType>(i)) {
            ++nextColor;
        }

        if (nextColor == maxColor) {
            ++maxColor;
        }

        boost::put(color, cur, nextColor);
        for (auto v: boost::make_iterator_range(boost::adjacent_vertices(cur, g))) {
            auto j = order[v];

            if (used[j]) {
                continue;
            }

            neighbourColors[j].emplace(nextColor);
            saturation[j] = neighbourColors[j].size();

            std::get<0>(*heapHandles[j]) = saturation[j];
            uncoloredVertices.increase(heapHandles[j]);
        }
    }

    return maxColor;
}
} // namespace impl

template <typename VertexListGraph, class P, class T, class R>
auto DSaturSequentialVertexColoring(VertexListGraph const& g, boost::bgl_named_params<P, T, R> const& params)
{
    auto color = boost::get_param(params, boost::vertex_color);
    auto order = boost::get_param(params, boost::vertex_index);

    return impl::DSaturSequentialVertexColoring(g, order, color);
}

template <typename VertexListGraph, class P, class T, class R>
auto DSaturSparseSequentialVertexColoring(VertexListGraph const& g, boost::bgl_named_params<P, T, R> const& params)
{
    auto color = boost::get_param(params, boost::vertex_color);
    auto order = boost::get_param(params, boost::vertex_index);

    return impl::DSaturSparseSequentialVertexColoring(g, order, color);
}
} // namespace heuristics