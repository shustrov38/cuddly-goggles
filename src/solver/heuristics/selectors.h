#pragma once

#include <boost/heap/fibonacci_heap.hpp>
#include <boost/heap/binomial_heap.hpp>

#include "vertex_info.h"

namespace solver::heuristics {
template <typename Info>
struct ICandidateSelector {
    using Self = ICandidateSelector<Info>;
    using Ptr = std::unique_ptr<Self>;

    using VertexInfo = Info;
    using SizeType = Info::SizeType;

    virtual ~ICandidateSelector() = default;

    virtual void Init(SizeType, std::vector<Info> &) = 0;
    virtual void Push(SizeType) = 0;
    virtual SizeType Pop() = 0;
    virtual void Update(SizeType) = 0;
};

template <typename Info>
class DenseCandidateSelector final: public ICandidateSelector<Info> {
    using Parent = ICandidateSelector<Info>;

public:
    using SizeType = Parent::SizeType;
    using VertexInfo = Parent::VertexInfo;

    void Init(SizeType n, std::vector<Info> &vinfo) override final
    {
        mInfoPtr = &vinfo;
        mUncolored.reserve(n);
    }

    void Push(SizeType i) override final
    {
        mUncolored.insert(i);
    }

    SizeType Pop() override final
    {
        SizeType maxSat = 0;
        for (auto i : mUncolored) {
            maxSat = std::max(maxSat, mInfoPtr->at(i).saturation.size());
        }

        std::set<std::pair<SizeType, SizeType>> candidates;
        for (auto i : mUncolored) {
            if (mInfoPtr->at(i).saturation.size() == maxSat) {
                candidates.emplace(mInfoPtr->at(i).degree, i);
            }
        }
        
        auto bestIt = candidates.rbegin();
        SizeType chosen = bestIt->second;
        mUncolored.erase(chosen);
        return chosen;
    }

    void Update(SizeType) override final
    {
    }

private:
    std::vector<Info> *mInfoPtr { nullptr };
    std::unordered_set<SizeType> mUncolored;
};

template <typename Info, template <typename T, class... Options> class HeapType>
class SparseCandidateSelector final: public ICandidateSelector<Info> {
    using Parent = ICandidateSelector<Info>;

public:
    using SizeType = Parent::SizeType;
    using VertexInfo = Parent::VertexInfo;
    using InfoPtr = Info *;

    struct CompareInfo {
        bool operator()(InfoPtr lhs, InfoPtr rhs) const {
            if (lhs->saturation.size() != rhs->saturation.size()) {
                return lhs->saturation.size() < rhs->saturation.size();
            }
            if (lhs->degree != rhs->degree) {
                return lhs->degree < rhs->degree;
            }
            return lhs->index < rhs->index;
        }
    };

    using Heap = HeapType<InfoPtr, boost::heap::compare<CompareInfo>>;
    using HandleType = typename Heap::handle_type;

    void Init(SizeType n, std::vector<Info> &vinfo) override final
    {
        mInfoPtr = &vinfo;
        mHandles.resize(n);
    }

    void Push(SizeType i) override final
    {
        mHandles[i] = mUncolored.push(&mInfoPtr->at(i));
    }

    SizeType Pop() override final
    {
        Info* topInfo = mUncolored.top();
        mUncolored.pop();
        return topInfo->index;
    }

    void Update(SizeType i) override final
    {
        mUncolored.increase(mHandles[i], &mInfoPtr->at(i));
    }

private:
    std::vector<Info> *mInfoPtr { nullptr };
    Heap mUncolored;
    std::vector<HandleType> mHandles;
};

template <typename Info>
using SparseCandidateSelectorBin = SparseCandidateSelector<Info, boost::heap::binomial_heap>;

template <typename Info>
using SparseCandidateSelectorFib = SparseCandidateSelector<Info, boost::heap::fibonacci_heap>;
} // namespace solver::heuristics
