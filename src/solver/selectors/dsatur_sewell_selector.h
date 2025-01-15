#pragma once

#include "icandidate_selector.h"

#include <boost/range/iterator_range.hpp>

namespace solver::selectors {
class SewellCandidateSelector final: public ICandidateSelector {
public:
    using Info = std::pair<SizeType, SizeType>;
    struct CompareInfo {
        bool operator()(Info lhs, Info rhs) const {
            if (lhs.first != rhs.first) {
                return lhs.first < rhs.first;
            }
            return lhs.second > rhs.second;
        }
    };

    void Init(SizeType n, DataMap dataMap) override final
    {
        mDataMap = dataMap;
        mUncolored.reserve(n);
    }

    void Push(Vertex i) override final
    {
        mUncolored.insert(i);
    }

    Vertex Pop(Graph const& g) override final
    {
        SizeType maxSat = 0;
        for (auto i : mUncolored) {
            maxSat = std::max(maxSat, Data(i)->Saturation());
        }

        std::set<Info, CompareInfo> candidates;
        for (auto i : mUncolored) {
            if (Data(i)->Saturation() == maxSat) {
                candidates.emplace(Same(i, g), i);
            }
        }
        
        auto bestIt = candidates.rbegin();
        auto chosen = bestIt->second;
        mUncolored.erase(chosen);
        return chosen;
    }
    
    bool Empty() override final
    {
        return mUncolored.empty();
    }

private:
    SizeType Same(Vertex v, Graph const& g)
    {
        Data(v)->admissibleColors = 0;
        for (auto u: mUncolored) {
            if (u == v) {
                continue;
            }
            
            if (auto [_, hasEdge] = boost::edge(u, v, g); !hasEdge) {
                continue;
            }

            ColorType adm = (~(Data(v)->neighbourColors | Data(u)->neighbourColors)).count();
            adm = std::min(adm, Data(v)->maxColor + 1);

            Data(v)->admissibleColors += adm;
        }
        return Data(v)->admissibleColors;
    }

    DSaturData *Data(Vertex v)
    {
        return static_cast<DSaturData *>(mDataMap[v].get());
    }

    DataMap mDataMap;

    std::unordered_set<Vertex> mUncolored;
};
} // namespace solver::selectors