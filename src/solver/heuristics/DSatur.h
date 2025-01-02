#pragma once

#include <boost/graph/connected_components.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include <boost/range/iterator_range.hpp>

#include <boost/heap/fibonacci_heap.hpp>

namespace heuristics {
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

    struct VertexInfo {
        SizeType i;
        Vertex v;

        SizeType degree;
        std::unordered_set<ColorType> saturation;
    };

    std::vector<VertexInfo> vertexInfo(n);
    std::unordered_set<SizeType> uncolored;
    for (auto v: boost::make_iterator_range(boost::vertices(g))) {
        SizeType i = order[v];
        vertexInfo[i] = {
            .i = i,
            .v = v,
            .degree = boost::out_degree(v, g),
            .saturation = {} 
        };
        boost::put(color, v, 0);
        uncolored.emplace(i);
    }

    auto findCandidate = [&vertexInfo, &uncolored]() {
        SizeType maxSaturation = 0;

        for (auto i: uncolored) {
            maxSaturation = std::max(maxSaturation, vertexInfo[i].saturation.size());
        }

        std::set<std::pair<SizeType, SizeType>> candidates; // pair (degree, index in vertexInfo)
        for (auto i: uncolored) {
            if (vertexInfo[i].saturation.size() == maxSaturation) {
                candidates.emplace(vertexInfo[i].degree, i);
            }
        }

        SizeType const i = candidates.rbegin()->second;
        uncolored.erase(i);
        return i;
    };

    while (!uncolored.empty()) {
        auto i = findCandidate();
        auto v = vertexInfo[i].v;

        for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
            mark[boost::get(color, u)] = i;
        }

        ColorType nextColor = 0;
        while (nextColor < maxColor && mark[nextColor] == i) {
            ++nextColor;
        }

        if (nextColor == maxColor) {
            ++maxColor;
        }

        boost::put(color, v, nextColor);

        for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
            auto j = order[u];
            vertexInfo[j].saturation.emplace(nextColor);
        }
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

    struct VertexInfo {
        SizeType i;
        Vertex v;

        SizeType degree;
        std::unordered_set<ColorType> saturation {};

        bool colored { false };
    };

    struct CompareVertexInfo {
        bool operator()(const VertexInfo *lhs, const VertexInfo *rhs) const {
            if (lhs->saturation.size() != rhs->saturation.size()) {
                return lhs->saturation.size() < rhs->saturation.size();
            }
            if (lhs->degree != rhs->degree) {
                return lhs->degree < rhs->degree;
            }
            return lhs->i < rhs->i;
        }
    };

    std::vector<VertexInfo> vertexInfo(n);

    using Heap = boost::heap::fibonacci_heap<VertexInfo *, boost::heap::compare<CompareVertexInfo>>;
    Heap uncolored;
    std::vector<typename Heap::handle_type> handles(n);

    for (auto v: boost::make_iterator_range(boost::vertices(g))) {
        SizeType i = order[v];
        vertexInfo[i] = {
            .i = i,
            .v = v,
            .degree = boost::out_degree(v, g),
            .saturation = {},
            .colored = false
        };
        boost::put(color, v, 0);
        handles[i] = uncolored.emplace(&vertexInfo[i]);
    }

    auto findCandidate = [&uncolored]() {
        auto i = uncolored.top()->i;
        uncolored.pop();
        return i;
    };

    while (!uncolored.empty()) {
        auto i = findCandidate();
        auto v = vertexInfo[i].v;

        for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
            mark[boost::get(color, u)] = i;
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
                uncolored.increase(handles[j], &vertexInfo[j]);
            }
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