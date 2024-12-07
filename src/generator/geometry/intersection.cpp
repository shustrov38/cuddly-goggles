#include "intersection.h"

namespace geometry {
namespace detail {
inline bool Intersect1D(double l1, double r1, double l2, double r2) {
	if (l1 > r1) {
        std::swap(l1, r1);
    }
	if (l2 > r2) {
        std::swap (l2, r2);
    }
	return std::max(l1, l2) <= std::min(r1, r2) + EPS;
}

inline int32_t Vec(Point const& a, Point const& b, Point const& c)
{
    double s = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    return std::abs(s) < EPS ? 0 : (s > 0 ? +1 : -1);
}
} // namespace detail

bool Intersect(Segment const& a, Segment const& b)
{
    return detail::Intersect1D(a.p.x, a.q.x, b.p.x, b.q.x)
        && detail::Intersect1D(a.p.y, a.q.y, b.p.y, b.q.y)
		&& detail::Vec(a.p, a.q, b.p) * detail::Vec(a.p, a.q, b.q) <= 0
		&& detail::Vec(b.p, b.q, a.p) * detail::Vec(b.p, b.q, a.q) <= 0;
}
} // namespace geometry
