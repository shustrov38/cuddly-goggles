#pragma once

#include "icandidate_selector.h"

namespace solver::selectors {
class DenseCandidateSelector final: public ICandidateSelector {
public:
    void Init(SizeType n, DataMap dataMap) override final
    {
        mDataMap = dataMap;
        mUncolored.reserve(n);
    }

    void Push(Vertex i) override final
    {
        mUncolored.insert(i);
    }

    Vertex Pop() override final
    {
        SizeType maxSat = 0;
        for (auto i : mUncolored) {
            maxSat = std::max(maxSat, Data(i)->Saturation());
        }

        std::set<std::pair<SizeType, SizeType>> candidates;
        for (auto i : mUncolored) {
            if (Data(i)->Saturation() == maxSat) {
                candidates.emplace(Data(i)->degree, i);
            }
        }
        
        auto bestIt = candidates.rbegin();
        auto chosen = bestIt->second;
        mUncolored.erase(chosen);
        return chosen;
    }

private:
    DSaturData *Data(Vertex v)
    {
        return static_cast<DSaturData *>(mDataMap[v].get());
    }

    DataMap mDataMap;

    std::unordered_set<Vertex> mUncolored;
};
} // namespace solver::selectors