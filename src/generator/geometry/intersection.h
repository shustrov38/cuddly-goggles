#pragma once 

#include <vector>
#include <set>

#include "constants.h"
#include "segment.h"
#include "point.h"

namespace geometry {
bool Intersect(Segment const& a, Segment const& b);

bool HasInersection(std::vector<Segment> const& segments); 
} // namespace geometry