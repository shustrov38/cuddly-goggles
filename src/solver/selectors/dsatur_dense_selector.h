#pragma once

#include "icandidate_selector.h"

namespace solver::selectors {
class DenseCandidateSelector final: public ICandidateSelector {
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

    Vertex Pop(Graph const&) override final
    {
        SizeType maxSat = 0;
        for (auto i : mUncolored) {
            maxSat = std::max(maxSat, Data(i)->Saturation());
        }

        std::set<Info, CompareInfo> candidates;
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

    bool Empty() override final
    {
        return mUncolored.empty();
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