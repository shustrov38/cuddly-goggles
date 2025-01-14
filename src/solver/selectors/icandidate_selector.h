#pragma once

#include <memory>

#include "../graph.h"

namespace solver::selectors {
struct ICandidateSelector {
    using Ptr = std::shared_ptr<ICandidateSelector>;

    virtual ~ICandidateSelector() = default;

    virtual void Init(SizeType, DataMap) = 0;
    virtual void Push(Vertex) = 0;
    virtual Vertex Pop() = 0;
    virtual void Update(Vertex) {};
};
} // namespace solver::selectors