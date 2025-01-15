#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include <bitset>
#include <memory>

namespace solver {
struct VertexProperty {
    int32_t index;
    int32_t color;

    std::shared_ptr<void> data;

    VertexProperty() = default;

    explicit VertexProperty(int32_t i)
        : index(i)
        , color(0)
    {
    }
};

struct EdgeProperty {
};

using Graph = boost::adjacency_list<
    boost::hash_setS, boost::vecS, boost::undirectedS,
    // vertex property
    VertexProperty,
    // edge property
    EdgeProperty
>;
using GraphTraits = boost::graph_traits<Graph>;

using Edge = GraphTraits::edge_descriptor;
using Vertex = GraphTraits::vertex_descriptor;

using SizeType = GraphTraits::vertices_size_type;

using ColorType = decltype(VertexProperty::color);
using ColorMap = boost::property_map<Graph, ColorType VertexProperty::*>::type;

using DataType = decltype(VertexProperty::data);
using DataMap = boost::property_map<Graph, DataType VertexProperty::*>::type;

struct DSaturData {
    SizeType index;

    bool colored;
    
    SizeType degree;

    std::bitset<10> neighbourColors;

    ColorType const& maxColor;
    SizeType admissibleColors;

    explicit DSaturData(SizeType index, SizeType degree, ColorType const& maxColor)
        : index(index)
        , colored(false)
        , degree(degree)
        , neighbourColors(0)
        , maxColor(maxColor)
    {
    }

    void Mark(ColorType c)
    {
        neighbourColors.set(c);
    }

    ColorType ColorMex() const
    {
        ColorType c = 0;
        while (neighbourColors.test(c) && c < static_cast<ColorType>(neighbourColors.size())) {
            ++c;
        }
        return c;
    }

    size_t Saturation() const noexcept
    {
        return neighbourColors.count();
    }
};
} // namespace solver
