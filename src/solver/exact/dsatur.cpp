#include "dsatur.h"

#include <limits>
#include <stack>
#include <unordered_map>
#include <iostream>

namespace solver::exact {
namespace detail {
inline DSaturData *Data(DataMap dataMap, Vertex v)
{
    return static_cast<DSaturData *>(dataMap[v].get());
}

struct Solution {
    std::stack<ColorType> maxColor;
    std::stack<Vertex> procesed;
    ColorType answer = std::numeric_limits<ColorType>::max();
};

void DSaturCore(Graph &g, selectors::ICandidateSelector::Ptr selector, Solution &solution)
{
    if (selector->Empty()) {
        solution.answer = std::min(solution.answer, solution.maxColor.top());
        return;
    }

    // std::string ofs;
    // for (int i = 0; i < solution.procesed.size(); ++i) {
    //     ofs += "    ";
    // }

    auto dataMap = boost::get(&VertexProperty::data, g);
    auto colorMap = boost::get(&VertexProperty::color, g);

    auto v = selector->Pop(g);
    // std::cout << ofs << "v=" << v + 1 << ' ';
    solution.procesed.push(v);

    for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
        if (Data(dataMap, u)->colored) {
            Data(dataMap, v)->Mark(colorMap[u]);
        }
    }

    auto admissibleColors = ~Data(dataMap, v)->neighbourColors;
    // std::cout << admissibleColors.to_string() << ' ' << solution.maxColor.top() << std::endl;
    for (ColorType nextColor = 0; nextColor <= solution.maxColor.top() && nextColor < admissibleColors.size() && nextColor < solution.answer - 1; ++nextColor) {
        if (admissibleColors.test(nextColor)) {
            // std::cout << ofs << "c=" << nextColor << std::endl;
            colorMap[v] = nextColor;
            Data(dataMap, v)->colored = true;
            solution.maxColor.push(std::max(solution.maxColor.top(), nextColor + 1));

            // for state restoration
            std::unordered_map<Vertex, decltype(DSaturData::neighbourColors)> cache;
            cache.reserve(boost::out_degree(v, g));

            for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
                if (!Data(dataMap, u)->colored) {
                    cache[u] = Data(dataMap, u)->neighbourColors;
                    Data(dataMap, u)->Mark(nextColor);
                }
            }

            DSaturCore(g, selector, solution);

            for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
                if (cache.contains(u)) {
                    Data(dataMap, u)->neighbourColors = cache[u];
                }
            }

            Data(dataMap, v)->colored = false;
            solution.maxColor.pop();
        }
    }

    solution.procesed.pop();
    selector->Push(v);
}

ColorType BnB(Graph &g, selectors::ICandidateSelector::Ptr selector)
{
    // prepare graph
    ColorType maxColor = 0;
    auto const n = boost::num_vertices(g);
    
    auto dataMap = boost::get(&VertexProperty::data, g);
    auto colorMap = boost::get(&VertexProperty::color, g);
    selector->Init(n, dataMap);

    for (auto v: boost::make_iterator_range(boost::vertices(g))) {      
        dataMap[v] = std::make_shared<DSaturData>(v, boost::out_degree(v, g), maxColor);
        colorMap[v] = 0;

        selector->Push(v);
    }

    Solution solution;
    solution.maxColor.push(0);

    DSaturCore(g, selector, solution);

    return solution.answer;
}
}

ColorType DSatur(Graph &g, Config config)
{
    selectors::ICandidateSelector::Ptr selector;
    if (config == DSATUR) {
        selector = std::make_shared<selectors::DenseCandidateSelector>();
    } else if (config == DSATUR_SEWELL) {
        selector = std::make_shared<selectors::SewellCandidateSelector>();
    }

    return detail::BnB(g, selector);
}
} // namespace solver::exact
