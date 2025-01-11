#pragma once

#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/io/svg/svg_mapper.hpp>

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include <boost/range/iterator_range.hpp>

#include <boost/polygon/voronoi.hpp>

#include <filesystem>
#include <optional>

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

namespace generator {
struct Parameters {
    size_t numVertices { 0 };
    
    bool connectivity { false };
    double removePropability { 0.5 };

    std::optional<std::filesystem::path> svgPath { std::nullopt };
};

class Generator {
public:
    using Points = std::vector<Point>;

    struct VertexProperty {
        int32_t index;
        Point coord;

        // connected_component dependency
        boost::default_color_type used;
        int32_t component;

        VertexProperty() = default;

        explicit VertexProperty(int32_t index, Point const& coord)
            : index(index)
            , coord(coord)
            , used(boost::white_color)
            , component(0)
        {
        }
    };

    struct EdgeProperty {
        int32_t index;
        int32_t weight;
        bool mst;
    };

    using Graph = boost::adjacency_list<
        boost::hash_setS, boost::vecS, boost::undirectedS,
        // vertex property
        VertexProperty,
        // edge property
        EdgeProperty
    >;
    using GraphTraits = boost::graph_traits<Graph>;
    using Edge = GraphTraits::edge_descriptor;
    using Vertex = GraphTraits::vertex_descriptor;

    void Generate(Parameters const& params);
    Graph const& GetGraph() const;

    void ToSVG(std::ostream &svg) const;

private:
    void GenerateRandomPoints(Points &points);
    void BuildGraphUsingVoronoiDiagram(Points const &points);
    void FindRandomMST();
    void RemoveEdgesWithProp(double prop = 0.5, bool connectivity = true);

    Graph mGraph;
};
} // namespace generator