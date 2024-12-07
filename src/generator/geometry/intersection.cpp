#include "intersection.h"

namespace geometry {
namespace detail {
inline bool Intersect1D(double l1, double r1, double l2, double r2) {
	if (l1 > r1) {
        std::swap(l1, r1);
    }
	if (l2 > r2) {
        std::swap(l2, r2);
    }
	return std::max(l1, l2) <= std::min(r1, r2) + EPS;
}

inline int8_t Vec(Point const& a, Point const& b, Point const& c)
{
    double s = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    return std::abs(s) < EPS ? 0 : (s > 0 ? +1 : -1);
}

struct Event {
    double x;
    int32_t id;
    int8_t type;

    Event()
    {
    }

    Event(double x, int32_t id, int8_t type)
        : x(x)
        , id(id)
        , type(type)
    {
    }

	bool operator <(Event const& e) const {
		if (abs (x - e.x) > EPS) {
            return x < e.x;
        }
		return type > e.type;
	}
};
} // namespace detail

bool Intersect(Segment const& a, Segment const& b)
{
    return detail::Intersect1D(a.p.x, a.q.x, b.p.x, b.q.x)
        && detail::Intersect1D(a.p.y, a.q.y, b.p.y, b.q.y)
		&& detail::Vec(a.p, a.q, b.p) * detail::Vec(a.p, a.q, b.q) <= 0
		&& detail::Vec(b.p, b.q, a.p) * detail::Vec(b.p, b.q, a.q) <= 0;
}

bool HasInersection(std::vector<Segment> const& segments)
{
    static std::set<Segment> s;
    s.clear();

    static std::vector<decltype(s)::iterator> where;
    where.resize(segments.size());

    static std::vector<detail::Event> events;
    events.resize(0);
    for (int32_t i = 0; i < static_cast<int32_t>(events.size()); ++i) {
        events.emplace_back(std::min(segments[i].p.x, segments[i].q.x), i, +1);
        events.emplace_back(std::max(segments[i].p.x, segments[i].q.x), i, -1);
    }
    std::sort(events.begin(), events.end());

    for (auto const& [_, id, type]: events) {
        if (type == +1) {
            auto nxt = s.lower_bound(segments[id]);
            if (nxt != s.end() && Intersect(*nxt, segments[id])) {
                return true;
            }
            auto prv = std::prev(nxt);
            if (prv != s.end() && Intersect(*prv, segments[id])) {
                return true;
            }
            where[id] = s.insert(nxt, segments[id]);
        } else {
            auto nxt = std::next(where[id]);
            auto prv = std::prev(where[id]);
            if (nxt != s.end() && prv != s.end() && Intersect(*nxt, *prv)) {
                return true;
            }
            s.erase(where[id]);
        }
    }
    return false;
}
} // namespace geometry
