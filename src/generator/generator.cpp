#include "generator.h"

#include <ieee754.h>
typedef long double fpt80;

namespace bp = boost::polygon;

namespace bg = boost::geometry;
namespace bgm = bg::model;

struct UlpComparison {
    enum Result {
        LESS = -1,
        EQUAL = 0,
        MORE = 1
    };

    Result operator()(fpt80 a, fpt80 b, unsigned int maxUlps) const {
        if (a == b) {
            return EQUAL;
        }
        if (a > b) {
            Result res = operator()(b, a, maxUlps);
            if (res == EQUAL) {
                return res;
            }
            return (res == LESS) ? MORE : LESS;
        }
        ieee854_long_double lhs;
        lhs.d = a;

        ieee854_long_double rhs;
        rhs.d = b;
        
        if (lhs.ieee.negative ^ rhs.ieee.negative) {
            return lhs.ieee.negative ? LESS : MORE;
        }

        uint64_t le = lhs.ieee.exponent;
        le = (le << 32) + lhs.ieee.mantissa0;

        uint64_t re = rhs.ieee.exponent;
        re = (re << 32) + rhs.ieee.mantissa0;

        if (lhs.ieee.negative) {
            if (le - 1 > re) {
                return LESS;
            }
            le = (le == re) ? 0 : 1;
            le = (le << 32) + lhs.ieee.mantissa1;
            re = rhs.ieee.mantissa1;
            return (re + maxUlps < le) ? LESS : EQUAL;
        } else {
            if (le + 1 < re) {
                return LESS;
            }
            le = lhs.ieee.mantissa0;
            re = (le == re) ? 0 : 1;
            re = (re << 32) + rhs.ieee.mantissa1;
            return (le + maxUlps < re) ? LESS : EQUAL;
        }
    }
};

struct FptConverter {
    template <typename T>
    fpt80 operator()(const T& that) const {
        return static_cast<fpt80>(that);
    }

    template <std::size_t N>
    fpt80 operator()(const typename bp::detail::extended_int<N> &that) const {
        fpt80 result = 0.0;
        for (std::size_t i = 1; i <= (std::min)((std::size_t)3, that.size()); ++i) {
        if (i != 1)
            result *= static_cast<fpt80>(0x100000000ULL);
        result += that.chunks()[that.size() - i];
        }
        return (that.count() < 0) ? -result : result;
    }
};

struct VoronoiDiagramTraits {
    typedef fpt80 coordinate_type;
    typedef bp::voronoi_cell<coordinate_type> cell_type;
    typedef bp::voronoi_vertex<coordinate_type> vertex_type;
    typedef bp::voronoi_edge<coordinate_type> edge_type;
    typedef class VertexEqualityPredicate {
    public:
        enum { ULPS = 128 };
        bool operator()(const vertex_type &v1, const vertex_type &v2) const {
            return (
                ulp_cmp(v1.x(), v2.x(), ULPS) == UlpComparison::EQUAL &&
                ulp_cmp(v1.y(), v2.y(), ULPS) == UlpComparison::EQUAL
            );
        }
    private:
        UlpComparison ulp_cmp;
    } vertex_equality_predicate_type;
};

struct VoronoiCtypeTraits {
    typedef int64_t int_type;
    typedef bp::detail::extended_int<3> int_x2_type;
    typedef bp::detail::extended_int<3> uint_x2_type;
    typedef bp::detail::extended_int<128> big_int_type;
    typedef fpt80 fpt_type;
    typedef fpt80 efpt_type;
    typedef UlpComparison ulp_cmp_type;
    typedef FptConverter to_fpt_converter_type;
    typedef FptConverter to_efpt_converter_type;
};

namespace generator {
void Generator::Generate(Parameters const& params)
{   
    Points points(params.numVertices);
    GenerateRandomPoints(points);
    BuildGraphUsingVoronoiDiagram(points);
    RemoveEdgesWithProp(params.removeProbability, params.connectivity);

    // remained edge indexing for further planar face traversal 
    auto edgeIndexMap = boost::get(&EdgeProperty::index, mGraph);

    int32_t index = 0;
    for (auto e: boost::make_iterator_range(boost::edges(mGraph))) {
        boost::put(edgeIndexMap, e, index++);
    }
}

Generator::Graph const& Generator::GetGraph() const
{
    return mGraph;
}

void Generator::GenerateRandomPoints(Points &points)
{
    static int64_t SCALE_POINT = std::sqrt(std::numeric_limits<int64_t>::max());

    static std::random_device rd {};
    static std::mt19937 gen { rd() };

    std::uniform_int_distribution<int64_t> dist(0, SCALE_POINT);

    int32_t index = 0;
    for (auto &p: points) {
        p.x = dist(gen), p.y = dist(gen);
        boost::add_vertex(VertexProperty(index++, p), mGraph);    
    }
}

void Generator::BuildGraphUsingVoronoiDiagram(Points const &points)
{
    bp::voronoi_builder<Point::Type, VoronoiCtypeTraits> vb;
    for (auto &[x, y]: points) {
        vb.insert_point(x, y);
    }

    bp::voronoi_diagram<fpt80, VoronoiDiagramTraits> vd;
    vb.construct(&vd);

    for (auto &cell: vd.cells()) {       
        auto v = cell.source_index();
        auto *edge = cell.incident_edge();
        if (!edge) {
            continue;
        }
        do {
            if (edge->is_primary()) {
                auto u = edge->twin()->cell()->source_index();
                boost::add_edge(v, u, {}, mGraph);
            }
            edge = edge->next();
        } while (edge != cell.incident_edge());
    }
}

void Generator::FindRandomMST()
{
    auto edgeWeightMap = boost::get(&EdgeProperty::weight, mGraph);
    auto edgeMstMap = boost::get(&EdgeProperty::mst, mGraph);
    
    int32_t constexpr SCALE_WEIGHT = 1e6;

    for (auto e: boost::make_iterator_range(boost::edges(mGraph))) {
        auto w = rnd::mt19937::Get(0, SCALE_WEIGHT);

        boost::put(edgeWeightMap, e, w);
        boost::put(edgeMstMap, e, false);
    }

    std::vector<Edge> mst;
    boost::kruskal_minimum_spanning_tree(
        mGraph, std::back_inserter(mst),
        boost::vertex_index_map(boost::get(&VertexProperty::index, mGraph))
            .weight_map(boost::get(&EdgeProperty::weight, mGraph))
    );

    auto isMstEdge = boost::get(&EdgeProperty::mst, mGraph);
    for (auto e: mst) {
        boost::put(isMstEdge, e, true);
    }
}

void Generator::RemoveEdgesWithProp(double prop, bool connectivity)
{
    if (connectivity) {
        FindRandomMST();
    }

    auto isMstEdge = boost::get(&EdgeProperty::mst, mGraph);

    std::vector<Edge> toRemove;
    for (auto e: boost::make_iterator_range(boost::edges(mGraph))) {
        if (isMstEdge[e]) {
            continue;
        }
        if (rnd::mt19937::Uniform() <= prop) {
            toRemove.emplace_back(e);
        }
    }
    for (auto e: toRemove) {
        boost::remove_edge(e, mGraph);
    }

    boost::connected_components(
        mGraph,
        boost::get(&VertexProperty::component, mGraph),
        boost::color_map(boost::get(&VertexProperty::used, mGraph))
            .vertex_index_map(boost::get(&VertexProperty::index, mGraph))
    );
}

void Generator::ToSVG(std::ostream &svg) const
{
    bg::svg_mapper<Point> map(svg, 600, 600); 

    using Segment = bgm::segment<Point>;
    std::vector<Segment> segments;

    auto vertexCoord = boost::get(&VertexProperty::coord, mGraph);
    for (auto e: boost::make_iterator_range(boost::edges(mGraph))) {
        Vertex v = boost::source(e, mGraph);
        Vertex u = boost::target(e, mGraph);
        segments.emplace_back(vertexCoord[v], vertexCoord[u]);
        map.add(segments.back());
    }

    for (auto const& segment: segments) {
        map.map(segment, R"(stroke:rgb(0, 0, 0);stroke-width:1)");
    }
}
} // namespace generator