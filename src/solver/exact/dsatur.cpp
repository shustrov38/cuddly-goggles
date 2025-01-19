#include "dsatur.h"

namespace solver::exact {
namespace detail {
inline DSaturData *Data(DataMap dataMap, Vertex v)
{
    return static_cast<DSaturData *>(dataMap[v].get());
}

struct Solution {
    std::vector<ColorType> coloring;
    std::stack<ColorType> maxColor;
    ColorType answer = 5;
};

void DSaturCore(Graph &g, selectors::ICandidateSelector::Ptr selector, Solution &solution)
{
    if (solution.maxColor.top() >= solution.answer) {
        return;
    }

    if (selector->Empty()) {
        solution.answer = solution.maxColor.top();

        auto colorMap = boost::get(&VertexProperty::color, g);
        for (auto v: boost::make_iterator_range(boost::vertices(g))) {
            colorMap[v] = solution.coloring[v];
        }

        return;
    }

    auto dataMap = boost::get(&VertexProperty::data, g);

    auto v = selector->Pop(g);
    auto vNeighboursCache = Data(dataMap, v)->neighbourColors;

    for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
        if (Data(dataMap, u)->colored) {
            Data(dataMap, v)->Mark(solution.coloring[u]);
        }
    }

    ColorType colors = std::min(solution.maxColor.top() + 1, solution.answer - 1);
    auto admissibleColors = Data(dataMap, v)->F();

    assert(!Data(dataMap, v)->colored);

    for (ColorType nextColor = 0; nextColor < colors; ++nextColor) {
        if (admissibleColors & (1 << nextColor)) {
            solution.coloring[v] = nextColor;
            Data(dataMap, v)->colored = true;
            solution.maxColor.push(std::max(solution.maxColor.top(), nextColor + 1));

            using CacheData = std::pair<Vertex, decltype(DSaturData::neighbourColors)>;
            std::stack<CacheData> cache;

            for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
                if (!Data(dataMap, u)->colored) {
                    cache.emplace(u, Data(dataMap, u)->neighbourColors);
                    Data(dataMap, u)->Mark(nextColor);

                    if (std::popcount(Data(dataMap, u)->F())) {
                        continue;
                    }

                    // Prunning if no candidates left in U'

                    while (!cache.empty()) {
                        auto [u, neighbourColors] = cache.top();
                        cache.pop();

                        Data(dataMap, u)->neighbourColors = neighbourColors;
                    }

                    Data(dataMap, v)->colored = false;
                    solution.maxColor.pop();

                    selector->Push(v);
                    Data(dataMap, v)->neighbourColors = vNeighboursCache;
                    return;
                }
            }

            DSaturCore(g, selector, solution);

            while (!cache.empty()) {
                auto [u, neighbourColors] = cache.top();
                cache.pop();

                Data(dataMap, u)->neighbourColors = neighbourColors;
            }


            Data(dataMap, v)->colored = false;
            solution.maxColor.pop();
        }
    }

    selector->Push(v);
    Data(dataMap, v)->neighbourColors = vNeighboursCache;
}

ColorType BnB(Graph &g, selectors::ICandidateSelector::Ptr selector)
{
    auto const n = boost::num_vertices(g);
    
    auto dataMap = boost::get(&VertexProperty::data, g);
    auto colorMap = boost::get(&VertexProperty::color, g);
    selector->Init(n, dataMap);

    Solution solution;
    solution.coloring.assign(n, 0);

    for (auto v: boost::make_iterator_range(boost::vertices(g))) {      
        dataMap[v] = std::make_shared<DSaturData>(v, boost::out_degree(v, g), solution.answer);
        colorMap[v] = 0;

        selector->Push(v);
    }

    solution.maxColor.push(0);

    DSaturCore(g, selector, solution);

    return solution.answer;
}
}

ColorType DSatur(Graph &g, Config config)
{
    selectors::ICandidateSelector::Ptr selector;
    if (config == BNB_DSATUR) {
        selector = std::make_shared<selectors::DenseCandidateSelector>();
    } else if (config == BNB_DSATUR_SEWELL) {
        selector = std::make_shared<selectors::SewellCandidateSelector>();
    }

    return detail::BnB(g, selector);
}
} // namespace solver::exact
