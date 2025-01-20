#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include <functional>
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

using TimeLimitFunc = std::function<bool()>;
using TimeLimitFuncCRef = TimeLimitFunc const&; 

struct DSaturData {
    SizeType index;
    SizeType degree;

    bool colored;
    
    uint32_t neighbourColors;
    ColorType const& currentMaxColor;

    explicit DSaturData(SizeType index, SizeType degree, ColorType const& currentMaxColor)
        : index(index)
        , degree(degree)
        , colored(false)
        , neighbourColors(0)
        , currentMaxColor(currentMaxColor)
    {
    }

    void Mark(ColorType c) noexcept
    {
        neighbourColors |= (1 << c);
    }

    uint32_t Filter(uint32_t val) const noexcept
    {
        return val & ((1u << currentMaxColor) - 1u);
    } 

    auto F() const noexcept
    {
        return Filter(~neighbourColors);
    }

    ColorType ColorMex() const noexcept
    {
        return std::countr_one(Filter(neighbourColors));
    }

    size_t Saturation() const noexcept
    {
        return std::popcount(neighbourColors);
    }
};
} // namespace solver
