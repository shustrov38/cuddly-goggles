#pragma once

#include <boost/polygon/voronoi.hpp>

#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/io/svg/svg_mapper.hpp>

#include <unordered_set>
#include <filesystem>
#include <fstream>

#include <random.h>
#include <graph.h>

struct Point {
    double x;
    double y;

    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y)
    {}
};

BOOST_GEOMETRY_REGISTER_POINT_2D(Point, double, boost::geometry::cs::cartesian, x, y)

namespace boost::polygon {
template <>
struct geometry_concept<Point> {
    typedef point_concept type;
};

template <>
struct point_traits<Point> {
    typedef double coordinate_type;

    static inline coordinate_type get(const Point& point, orientation_2d orient)
    {
        return (orient == HORIZONTAL) ? point.x : point.y;
    }
};
} // namespace boost::polygon

namespace generator {
struct Parameters {
    size_t numVertices;
};

class Generator {
    using Points = std::vector<Point>;

public:
    explicit Generator(Parameters const& params);

    void Generate();

    void ToSVG(std::ostream &svg) const;
    void ToDIMACS(std::ostream &out) const;

private:
    Graph mGraph;
    
    Points mPoints;
};
} // namespace generator