#pragma once

#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/io/svg/svg_mapper.hpp>

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include <boost/range/iterator_range.hpp>

#include <boost/polygon/voronoi.hpp>

#include <boost/timer/timer.hpp>

#include <unordered_set>
#include <filesystem>
#include <optional>
#include <fstream>

#include <random.h>

struct Point {
    typedef int64_t Type;

    Type x;
    Type y;

    Point() = default;
    Point(Type x, Type y)
        : x(x)
        , y(y)
    {}
};

BOOST_GEOMETRY_REGISTER_POINT_2D(Point, Point::Type, boost::geometry::cs::cartesian, x, y)

namespace boost::polygon {
template <>
struct geometry_concept<Point> {
    typedef point_concept type;
};

template <>
struct point_traits<Point> {
    typedef Point::Type coordinate_type;

    static inline coordinate_type get(const Point& point, orientation_2d orient)
    {
        return (orient == HORIZONTAL) ? point.x : point.y;
    }
};
} // namespace boost::polygon

namespace boost {
enum vertex_coord_t {
    vertex_coord
}; 
template <> 
struct property_kind<vertex_coord_t> {
    typedef vertex_property_tag type;
};

enum edge_mst_t {
    edge_mst
}; 
template <> 
struct property_kind<edge_mst_t> {
    typedef edge_property_tag type;
};
} // namespace boost

namespace generator {
struct Parameters {
    size_t numVertices {0};
    std::optional<std::filesystem::path> svgPath {std::nullopt};
};

class Generator {
    using Points = std::vector<Point>;

    using Graph = boost::adjacency_list<
        boost::vecS, boost::hash_setS, boost::undirectedS,
        // vertex property
        boost::property<boost::vertex_index_t, int32_t,
            boost::property<boost::vertex_coord_t, Point>
        >,
        // edge property
        boost::property<boost::edge_weight_t, int32_t,
            boost::property<boost::edge_mst_t, bool>
        >
    >;
    using GraphTraits = boost::graph_traits<Graph>;
    using Edge = GraphTraits::edge_descriptor;
    using Vertex = GraphTraits::vertex_descriptor;

public:
    void Generate(Parameters const& params);

    void ToSVG(std::ostream &svg) const;
    void ToDIMACS(std::ostream &out) const;

private:
    void GenerateRandomPoints(Points &points);
    void BuildGraphUsingVoronoiDiagram(Points const &points);
    void FindMST();
    void RemoveEdgesWithProp(double prop = 0.5, bool connectivity = true);


    Graph mGraph;
    std::vector<Vertex> mVertexes;
};
} // namespace generator