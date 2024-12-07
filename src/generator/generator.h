#pragma once

#include <vector>

#include <random.h>

namespace generator {
struct Parameters {
    size_t numVertices;
};

class Generator {
    using Graph = std::vector<std::vector<int32_t>>;
    
    using Coordinate = std::pair<double, double>;
    using Coordinates = std::vector<Coordinate>; 
public:
    explicit Generator(Parameters const& params);

    void Generate();

private:
    Graph mGraph;
    Coordinates mCoords;
};
} // namespace generator