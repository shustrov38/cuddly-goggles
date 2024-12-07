#pragma once

#include "constants.h"
#include "point.h"

namespace geometry {
struct Segment {
    Point p;
    Point q;

    size_t id;

    double GetY(double x) const
    {
        if (std::abs(p.x - q.x) < EPS) {
            return p.y;
        }
        return p.y + (q.y - p.y) * (x - p.x) / (q.x - p.x);
    }

    friend bool operator <(Segment const& a, Segment const& b)
    {
        double x = std::max(std::min(a.p.x, a.q.x), std::min(b.p.x, b.q.x));
        return a.GetY(x) < b.GetY(x) - EPS;
    }
};
} // namespace geometry 
