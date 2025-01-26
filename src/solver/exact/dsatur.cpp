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
    ColorType currentMaxColor;
    ColorType answer = 5;

    void PushColor(ColorType c)
    {
        maxColor.push(std::max(maxColor.top(), c + 1));
        UpdateMaxColor();
    }

    void PopColor()
    {
        maxColor.pop();
        UpdateMaxColor();
    }

    void UpdateMaxColor()
    {
        currentMaxColor = std::min(maxColor.top() + 1, answer - 1);
    }
};

void DSaturCore(
    Graph &g, selectors::ICandidateSelector::Ptr selector,
    Solution &solution, TimeLimitFuncCRef timeLimitFunctor
)
{
    if (timeLimitFunctor()) {
        return;
    }

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

    auto admissibleColors = Data(dataMap, v)->F();

    assert(!Data(dataMap, v)->colored);

    for (ColorType nextColor = 0; nextColor < solution.currentMaxColor; ++nextColor) {
        if (admissibleColors & (1 << nextColor)) {
            if (timeLimitFunctor()) {
                return;
            }

            solution.coloring[v] = nextColor;
            Data(dataMap, v)->colored = true;
            solution.PushColor(nextColor);

            using CacheData = std::pair<Vertex, decltype(DSaturData::neighbourColors)>;
            std::stack<CacheData> cache;

            for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
                if (!Data(dataMap, u)->colored) {
                    cache.emplace(u, Data(dataMap, u)->neighbourColors);
                    Data(dataMap, u)->Mark(nextColor);

                    if (Data(dataMap, u)->F()) {
                        continue;
                    }

                    // PRUNE if no colors left for vertex u in U'

                    while (!cache.empty()) {
                        auto [u, neighbourColors] = cache.top();
                        cache.pop();

                        Data(dataMap, u)->neighbourColors = neighbourColors;
                    }

                    Data(dataMap, v)->colored = false;
                    solution.PopColor();  
                    
                    selector->Push(v);
                    Data(dataMap, v)->neighbourColors = vNeighboursCache;
                    return;
                }
            }

            DSaturCore(g, selector, solution, timeLimitFunctor);

            while (!cache.empty()) {
                auto [u, neighbourColors] = cache.top();
                cache.pop();

                Data(dataMap, u)->neighbourColors = neighbourColors;
            }

            Data(dataMap, v)->colored = false;
            solution.PopColor();    
        }
    }

    selector->Push(v);
    Data(dataMap, v)->neighbourColors = vNeighboursCache;
}

ColorType BnB(Graph &g, selectors::ICandidateSelector::Ptr selector, TimeLimitFuncCRef timeLimitFunctor)
{
    auto const n = boost::num_vertices(g);
    
    auto dataMap = boost::get(&VertexProperty::data, g);
    auto colorMap = boost::get(&VertexProperty::color, g);
    selector->Init(n, dataMap);

    Solution solution;
    solution.coloring.assign(n, 0);

    for (auto v: boost::make_iterator_range(boost::vertices(g))) {      
        dataMap[v] = std::make_shared<DSaturData>(v, boost::out_degree(v, g), solution.currentMaxColor);
        colorMap[v] = 0;

        selector->Push(v);
    }

    solution.maxColor.push(1);
    solution.currentMaxColor = solution.maxColor.top();

    DSaturCore(g, selector, solution, timeLimitFunctor);

    return solution.answer;
}
}

ColorType DSatur(Graph &g, Config config, TimeLimitFuncCRef timeLimitFunctor)
{
    selectors::ICandidateSelector::Ptr selector;
    if (config == BNB_DSATUR) {
        selector = std::make_shared<selectors::DenseCandidateSelector>();
    } else if (config == BNB_DSATUR_SEWELL) {
        selector = std::make_shared<selectors::SewellCandidateSelector>();
    } else if (config == BNB_DSATUR_PASS) {
        selector = std::make_shared<selectors::PassCandidateSelector>();
    }

    return detail::BnB(g, selector, timeLimitFunctor);
}
} // namespace solver::exact
