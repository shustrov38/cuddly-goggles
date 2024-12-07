#pragma once 

#include "constants.h"
#include "segment.h"
#include "point.h"

namespace geometry {
bool Intersect(Segment const& a, Segment const& b);
} // namespace geometry