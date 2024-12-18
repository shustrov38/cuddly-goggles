#include "generator.h"

#include <iostream>

namespace bp = boost::polygon;

namespace bg = boost::geometry;
namespace bgm = bg::model;

namespace generator {
Generator::Generator(Parameters const& params)
    : mGraph(params.numVertices)
{
    mPoints.resize(params.numVertices);
    mPoints.shrink_to_fit();
}

void Generator::Generate()
{
    double constexpr SCALE = 600;
    for (auto &p: mPoints) {
        bg::assign_values(p, rnd::mt19937::Uniform() * SCALE, rnd::mt19937::Uniform() * SCALE);
    }

    bp::voronoi_diagram<double> vd;
    bp::construct_voronoi(mPoints.begin(), mPoints.end(), &vd);

    for (auto &cell: vd.cells()) {       
        auto v = cell.source_index();
        auto *edge = cell.incident_edge();
        do {
            if (edge->is_primary()) {
                auto u = edge->twin()->cell()->source_index();
                mGraph.AddEdge(v, u);
            }
            edge = edge->next();
        } while (edge != cell.incident_edge());
    }

    double constexpr prop = 0.5;

    std::vector<int32_t> toRemove;
    for (auto v: mGraph.Vertices()) {
        toRemove.resize(0);
        for (auto u: mGraph.Neighbours(v)) {
            if (rnd::mt19937::Uniform() > prop) {
                toRemove.emplace_back(u);
            }
        }
        for (auto u: toRemove) {
            mGraph.RemoveEdge(v, u);
        }
    }
}

void Generator::ToSVG(std::ostream &svg) const
{
    bg::svg_mapper<Point> map(svg, 1200, 600); 

    using Segment = bgm::segment<Point>;
    std::vector<Segment> segments;

    for (auto v: mGraph.Vertices()) {
        for (auto u: mGraph.Neighbours(v)) {
            segments.emplace_back(mPoints[v], mPoints[u]);
            map.add(segments.back());
        }
    }

    for (auto const& segment: segments) {
        map.map(segment, R"(stroke:rgb(0, 0, 0);stroke-width:1)");
    }
}

void Generator::ToDIMACS(std::ostream &out) const
{
    out << "c SOURCE: Dmitriy Shustrov (shustrov38@gmail.com)" << std::endl;
    out << "c DESCRIPTION: Graph based on Voronoi Diagram with random edges removed." << std::endl;
    mGraph.ToDIMACS(out);
}
} // namespace generator