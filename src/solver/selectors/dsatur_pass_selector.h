#pragma once

#include "icandidate_selector.h"

#include <boost/range/iterator_range.hpp>

#include <vector>

namespace solver::selectors {
class PassCandidateSelector final: public ICandidateSelector {
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

    Vertex Pop(Graph const& g) override final
    {
        static std::vector<SizeType> T;
        T.resize(0);
        SizeType maxSat = 0;

        for (size_t i = 0; i < mUncolored.size(); ++i) {
            SizeType v = mUncolored[i];
            
            if (auto sat = Data(v)->Saturation(); sat > maxSat) {
                T.resize(0);
                maxSat = sat;
            }

            T.emplace_back(i);
        }

        // TODO: change method of getting currentMaxColor
        auto mu = Data(mUncolored[0])->currentMaxColor - maxSat;

        int32_t bestIndex = -1;
        Info bestCandidate;
        for (auto i: T) {
            SizeType v = mUncolored[i];
            if (Data(v)->Saturation() != maxSat) {
                continue;
            }
            
            Info info;

            if (mu <= TH) {
                info = {Same(v, maxSat, g), v}; // PASS
            } else {
                info = {Data(v)->degree, v}; // DSATUR
            }
            
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
    SizeType Same(Vertex v, SizeType maxSat, Graph const& g)
    {
        SizeType totalAdmissibleColors = 0;
        for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
            if (Data(u)->colored || Data(u)->Saturation() != maxSat) {
                continue;
            }
            totalAdmissibleColors += Data(v)->F() & Data(u)->F();
        }
        return totalAdmissibleColors;
    }

    DSaturData *Data(Vertex v)
    {
        return static_cast<DSaturData *>(mDataMap[v].get());
    }

    DataMap mDataMap;

    std::vector<Vertex> mUncolored;

    static size_t constexpr TH = 2; 
};
} // namespace solver::selectors