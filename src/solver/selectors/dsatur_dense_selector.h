#pragma once

#include "icandidate_selector.h"

#include <vector>

namespace solver::selectors {
class DenseCandidateSelector final: public ICandidateSelector {
public:
    using Info = std::pair<SizeType, SizeType>;
    struct CompareInfo {
        bool operator()(Info lhs, Info rhs) const
        {
            if (lhs.first != rhs.first) {
                return lhs.first < rhs.first;
            }
            return lhs.second < rhs.second;
        }
    };

    void Init(SizeType n, DataMap dataMap) override final
    {
        mDataMap = dataMap;
        mUncolored.reserve(n);
    }

    void Push(Vertex i) override final
    {
        mUncolored.push_back(i);
    }

    Vertex Pop(Graph const&) override final
    {
        SizeType maxSat = 0;
        for (size_t i = 0; i < mUncolored.size(); ++i) {
            SizeType v = mUncolored[i];
            maxSat = std::max(maxSat, Data(v)->Saturation());
        }

        int32_t bestIndex = -1;
        Info bestCandidate;
        for (size_t i = 0; i < mUncolored.size(); ++i) {
            SizeType v = mUncolored[i];
            if (Data(v)->Saturation() != maxSat) {
                continue;
            }
            Info info(Data(v)->degree, v);
            if (bestIndex == -1 || CompareInfo{}(bestCandidate, info)) {
                bestCandidate = info;
                bestIndex = i;
            }
        }

        std::swap(mUncolored[bestIndex], mUncolored.back());
        mUncolored.pop_back();
        return bestCandidate.second;
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

    std::vector<Vertex> mUncolored;
};
} // namespace solver::selectors