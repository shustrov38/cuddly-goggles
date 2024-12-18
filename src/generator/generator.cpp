#include "generator.h"

#include <iostream>

namespace bp = boost::polygon;

namespace bg = boost::geometry;
namespace bgm = bg::model;

namespace generator {
Generator::Generator(Parameters const& params)
{
    double constexpr SCALE = 600;
    
    Point p;
    for (int32_t i = 0; i < static_cast<int32_t>(params.numVertices); ++i) {
        bg::assign_values(p, rnd::mt19937::Uniform() * SCALE, rnd::mt19937::Uniform() * SCALE);
        mPoints.emplace_back(p);
        Vertex v = boost::add_vertex({i, p}, mGraph);
        mVertexes.emplace_back(v);
    }
}

void Generator::Generate()
{
    bp::voronoi_diagram<double> vd;
    bp::construct_voronoi(mPoints.begin(), mPoints.end(), &vd);

    double constexpr SCALE = 600;
    
    for (auto &cell: vd.cells()) {       
        auto v = cell.source_index();
        auto *edge = cell.incident_edge();
        do {
            if (edge->is_primary()) {
                auto u = edge->twin()->cell()->source_index();
                auto w = rnd::mt19937::Get(0, SCALE);
                boost::add_edge(mVertexes[v], mVertexes[u], {w, false}, mGraph);
            }
            edge = edge->next();
        } while (edge != cell.incident_edge());
    }

    std::vector<Edge> mst;
    boost::kruskal_minimum_spanning_tree(mGraph, std::back_inserter(mst));
    auto  isMstEdge = boost::get(boost::edge_mst, mGraph);
    for (auto &edge: mst) {
        isMstEdge[edge] = true;
    }

    double constexpr prop = 0.4;
    std::vector<Edge> toRemove;
    GraphTraits::edge_iterator ei, eiEnd;
    for (boost::tie(ei, eiEnd) = boost::edges(mGraph); ei != eiEnd; ++ei) {
        if (isMstEdge[*ei]) {
            continue;
        }
        if (rnd::mt19937::Uniform() > prop) {
            toRemove.emplace_back(*ei);
        }
    }
    std::cout << toRemove.size() << std::endl;
    for (auto &edge: toRemove) {
        boost::remove_edge(edge, mGraph);
    }
}

void Generator::ToSVG(std::ostream &svg) const
{
    bg::svg_mapper<Point> map(svg, 600, 600); 

    using Segment = bgm::segment<Point>;
    std::vector<Segment> segments;

    auto vertexCoord = boost::get(boost::vertex_coord, mGraph);
    GraphTraits::edge_iterator ei, eiEnd;
    for (boost::tie(ei, eiEnd) = boost::edges(mGraph); ei != eiEnd; ++ei) {
        Vertex v = boost::source(*ei, mGraph);
        Vertex u = boost::target(*ei, mGraph);
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
    
    auto numVerts = boost::num_vertices(mGraph);
    auto numEdges = boost::num_edges(mGraph);

    out << "p edge " << numVerts << " " << numEdges << std::endl;
    
    GraphTraits::edge_iterator ei, eiEnd;
    for (boost::tie(ei, eiEnd) = boost::edges(mGraph); ei != eiEnd; ++ei){
        Vertex src = boost::source(*ei, mGraph);
        Vertex tgt = boost::target(*ei, mGraph);
        int32_t v = boost::get(boost::vertex_index, mGraph, src);
        int32_t u = boost::get(boost::vertex_index, mGraph, tgt);
        out << "e " << v + 1 << " " << u + 1 << std::endl;
    }
}
} // namespace generator