#pragma once

#include "icandidate_selector.h"

#include <boost/heap/binomial_heap.hpp>
#include <boost/heap/fibonacci_heap.hpp>

namespace solver::selectors {
template<template <typename T, class... Options> class HeapType>
class SparseCandidateSelector final: public ICandidateSelector {
public:
    struct CompareInfo {
        bool operator()(DataType lhs, DataType rhs) const {
            if (Data(lhs)->Saturation() != Data(rhs)->Saturation()) {
                return Data(lhs)->Saturation() < Data(rhs)->Saturation();
            }
            if (Data(lhs)->degree != Data(rhs)->degree) {
                return Data(lhs)->degree < Data(rhs)->degree;
            }
            return Data(lhs)->index < Data(rhs)->index;
        }
    };

    using Heap = HeapType<DataType, boost::heap::compare<CompareInfo>>;
    using HandleType = typename Heap::handle_type;

    void Init(SizeType n, DataMap dataMap) override final
    {
        mDataMap = dataMap;
        mHandles.resize(n);
    }

    void Push(Vertex v) override final
    {
        mHandles[v] = mUncolored.push(mDataMap[v]);
    }

    Vertex Pop(Graph const&) override final
    {
        DataType chosen = mUncolored.top();
        mUncolored.pop();
        return Data(chosen)->index;
    }
    
    void Update(Vertex v) override final
    {
        mUncolored.increase(mHandles[v], mDataMap[v]);
    }

private:
    static DSaturData *Data(DataType data)
    {
        return static_cast<DSaturData *>(data.get());
    }

    DataMap mDataMap;

    Heap mUncolored;
    std::vector<HandleType> mHandles;
};

using SparseCandidateSelectorBin = SparseCandidateSelector<boost::heap::binomial_heap>;
using SparseCandidateSelectorFib = SparseCandidateSelector<boost::heap::fibonacci_heap>;
} // namespace solver::selectors