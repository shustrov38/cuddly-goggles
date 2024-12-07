#pragma once

#include <random.h>

#include "geometry/intersection.h"

namespace generator {
struct Parameters {
    size_t numVertices;
};

class Generator {
    using Graph = std::vector<std::vector<int32_t>>;
    
    using Coordinate = geometry::Point;
    using Coordinates = std::vector<Coordinate>;

    using Segment = geometry::Segment;
    using Segments = std::vector<Segment>;

public:
    explicit Generator(Parameters const& params);

    void Generate();

private:
    Graph mGraph;
    
    Coordinates mCoords;
    Segments mSegments;
};
} // namespace generator