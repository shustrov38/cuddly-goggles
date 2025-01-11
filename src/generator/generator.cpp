#include "generator.h"

#include <ieee754.h>
typedef long double fpt80;

namespace bp = boost::polygon;

namespace bg = boost::geometry;
namespace bgm = bg::model;

struct my_ulp_comparison {
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

struct my_fpt_converter {
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

struct my_voronoi_diagram_traits {
    typedef fpt80 coordinate_type;
    typedef bp::voronoi_cell<coordinate_type> cell_type;
    typedef bp::voronoi_vertex<coordinate_type> vertex_type;
    typedef bp::voronoi_edge<coordinate_type> edge_type;
    typedef class VertexEqualityPredicate {
    public:
        enum { ULPS = 128 };
        bool operator()(const vertex_type &v1, const vertex_type &v2) const {
            return (
                ulp_cmp(v1.x(), v2.x(), ULPS) == my_ulp_comparison::EQUAL &&
                ulp_cmp(v1.y(), v2.y(), ULPS) == my_ulp_comparison::EQUAL
            );
        }
    private:
        my_ulp_comparison ulp_cmp;
    } vertex_equality_predicate_type;
};

struct my_voronoi_ctype_traits {
    typedef int64_t int_type;
    typedef bp::detail::extended_int<3> int_x2_type;
    typedef bp::detail::extended_int<3> uint_x2_type;
    typedef bp::detail::extended_int<128> big_int_type;
    typedef fpt80 fpt_type;
    typedef fpt80 efpt_type;
    typedef my_ulp_comparison ulp_cmp_type;
    typedef my_fpt_converter to_fpt_converter_type;
    typedef my_fpt_converter to_efpt_converter_type;
};

namespace generator {
void Generator::Generate(Parameters const& params)
{   
    Points points(params.numVertices);
    GenerateRandomPoints(points);
    BuildGraphUsingVoronoiDiagram(points);
    RemoveEdgesWithProp(0.4, true);

    auto edgeIndexMap = boost::get(&EdgeProperty::index, mGraph);

    int32_t index = 0;
    for (auto e: boost::make_iterator_range(boost::edges(mGraph))) {
        boost::put(edgeIndexMap, e, index++);
    }
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
    bp::voronoi_builder<Point::Type, my_voronoi_ctype_traits> vb;
    for (auto &[x, y]: points) {
        vb.insert_point(x, y);
    }

    bp::voronoi_diagram<fpt80, my_voronoi_diagram_traits> vd;

    boost::timer::cpu_timer t;
    vb.construct(&vd);
    boost::timer::cpu_times times = t.elapsed();

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
    
    // prepare graph props
    for (auto e: boost::make_iterator_range(boost::edges(mGraph))) {
        auto w = rnd::mt19937::Get(0, SCALE_WEIGHT);

        boost::put(edgeWeightMap, e, w);
        boost::put(edgeMstMap, e, false);
    }

    // find MST
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

void Generator::ToDIMACS(std::ostream &out) const
{
    out << "c SOURCE: Dmitriy Shustrov (shustrov38@gmail.com)" << std::endl;
    out << "c DESCRIPTION: Graph based on Voronoi Diagram with random edges removed." << std::endl;

    AddStatsToDIMACS(out);

    auto numVerts = boost::num_vertices(mGraph);
    auto numEdges = boost::num_edges(mGraph);

    out << "p edge " << numVerts << " " << numEdges << std::endl;
    
    for (auto e: boost::make_iterator_range(boost::edges(mGraph))){
        Vertex v = boost::source(e, mGraph);
        Vertex u = boost::target(e, mGraph);

        out << "e " << v + 1 << " " << u + 1 << " " << boost::get(&EdgeProperty::index, mGraph, e) << std::endl;
    }
}

struct FaceCounter : public boost::planar_face_traversal_visitor
{
    std::map<size_t, size_t>& mFaceStats;
    size_t mCurrentFaceVerts { 0 };

    FaceCounter(std::map<size_t, size_t>& stats) : mFaceStats(stats) {}

    void begin_face() { mCurrentFaceVerts = 0; }

    template <typename Edge>
    void next_edge(Edge) { mCurrentFaceVerts++; }

    void end_face() { mFaceStats[mCurrentFaceVerts]++; }
};

struct output_visitor: public boost::planar_face_traversal_visitor
{
    void begin_face() { std::cout << "New face: "; }
    template <typename Vertex> void next_vertex(Vertex v) { std::cout << v << " "; }
    void end_face() { std::cout << std::endl; }
};

void Generator::ComputeFaceStats(std::map<size_t, size_t> &faceStats) const 
{
    std::vector<std::vector<Edge>> embedding(boost::num_vertices(mGraph));

    namespace P = boost::boyer_myrvold_params; 
    bool isPlanar = boost::boyer_myrvold_planarity_test(
        P::graph = mGraph,
        P::embedding = &embedding[0],
        P::vertex_index_map = boost::get(&VertexProperty::index, mGraph),
        P::edge_index_map = boost::get(&EdgeProperty::index, mGraph)
    );

    if (!isPlanar) {
        return;
    }

    FaceCounter visitor(faceStats);
    boost::planar_face_traversal(
        mGraph, &embedding[0], visitor,
        boost::get(&EdgeProperty::index, mGraph)
    );
}

void Generator::AddStatsToDIMACS(std::ostream &out) const
{
    auto numVerts = boost::num_vertices(mGraph);
    auto numEdges = boost::num_edges(mGraph);

    double avgDegree = 0;
    for (auto v: boost::make_iterator_range(boost::vertices(mGraph))) {
        avgDegree += boost::out_degree(v, mGraph);
    }
    avgDegree /= numVerts;

    double maxPossibleEdges = (numVerts * (numVerts - 1)) / 2.0;
    double edgeDensity = numEdges / maxPossibleEdges;

    // Calculate number of faces using Euler's formula for planar graphs
    int32_t numFaces = 2 - numVerts + numEdges;

    out << "c STATS: Average vertex degree = " << std::fixed << std::setprecision(2) << avgDegree << std::endl;
    out << "c STATS: Edge density = " << std::fixed << std::setprecision(4) << edgeDensity << std::endl;
    out << "c STATS: Number of faces = " << numFaces << std::endl;

    std::map<size_t, size_t> faceStats;
    ComputeFaceStats(faceStats);

    out << "c STATS: Face vertex count distribution:" << std::endl;
    for (const auto& [vertCount, faceCount] : faceStats) {
        out << "c        " << vertCount << " vertices: " << faceCount << " faces" << std::endl;
    }
}
} // namespace generator