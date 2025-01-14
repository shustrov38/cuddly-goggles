#include "dsatur.h"

#include <memory>

namespace solver::heuristics {
namespace detail {
inline DSaturData *Data(DataMap dataMap, Vertex v)
{
    return static_cast<DSaturData *>(dataMap[v].get());
}

ColorType DSaturCore(Graph &g, selectors::ICandidateSelector::Ptr selector)
{
    ColorType maxColor = 0;
    auto const n = boost::num_vertices(g);
    
    auto dataMap = boost::get(&VertexProperty::data, g);
    auto colorMap = boost::get(&VertexProperty::color, g);
    selector->Init(n, dataMap);

    for (auto v: boost::make_iterator_range(boost::vertices(g))) {      
        dataMap[v] = std::make_shared<DSaturData>(v, boost::out_degree(v, g));
        colorMap[v] = 0;

        selector->Push(v);
    }

    for (SizeType _ = 0; _ < n; ++_) {
        auto v = selector->Pop();

        for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
            if (Data(dataMap, u)->colored) {
                Data(dataMap, v)->Mark(colorMap[u]);
            }
        }

        auto nextColor = Data(dataMap, v)->ColorMex();
        maxColor = std::max(maxColor, nextColor + 1);

        colorMap[v] = nextColor;
        Data(dataMap, v)->colored = true;

        for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
            if (!Data(dataMap, u)->colored) {
                Data(dataMap, u)->Mark(nextColor);
                selector->Update(u);
            }
        }
    }

    return maxColor;
}
} // namespace detail

ColorType DSatur(Graph &g, Config config)
{
    selectors::ICandidateSelector::Ptr selector;
    if (config == DSATUR) {
        selector = std::make_shared<selectors::DenseCandidateSelector>();
    } else if (config == DSATUR_BINARY_HEAP) {
        selector = std::make_shared<selectors::SparseCandidateSelectorBin>();
    } else if (config == DSATUR_FIBONACCI_HEAP) {
        selector = std::make_shared<selectors::SparseCandidateSelectorFib>();
    }

    return detail::DSaturCore(g, selector);
}
} // namespace solver::heuristics
