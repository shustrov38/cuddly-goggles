#pragma once 

#include <unordered_set>

namespace solver::heuristics {
template <typename SizeType_, typename Vertex, typename ColorType>
struct VertexInfo {
    using SizeType = SizeType_;

    SizeType index;
    Vertex vertex;
    
    SizeType degree;
    std::unordered_set<ColorType> saturation;
    
    bool colored;
};
} // namespace solver::heuristics
