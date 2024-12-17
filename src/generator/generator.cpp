#include "generator.h"

#include <iostream>

namespace bp = boost::polygon;

namespace bg = boost::geometry;
namespace bgm = bg::model;

namespace generator {
Generator::Generator(Parameters const& params)
{
    mGraph.resize(params.numVertices);
    mGraph.shrink_to_fit();

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

    std::ofstream svg("/tmp/image.svg");
    bg::svg_mapper<Point> map(svg, 1200, 600); 

    using Segment = bgm::segment<Point>;
    std::vector<Segment> segments;

    for (auto &cell: vd.cells()) {       
        auto v = cell.source_index();
        auto *edge = cell.incident_edge();
        do {
            if (edge->is_primary()) {
                auto u = edge->twin()->cell()->source_index();
                mGraph[v].emplace(u);
            }
            edge = edge->next();
        } while (edge != cell.incident_edge());
    }

    double constexpr prop = 0.5;

    for (int32_t v = 0; v < static_cast<int32_t>(mGraph.size()); ++v) {
        std::unordered_set<int32_t> toRemove;
        for (auto u: mGraph[v]) {           
            if (rnd::mt19937::Uniform() > prop) {
                toRemove.emplace(u);
                continue;
            }

            segments.emplace_back(mPoints[v], mPoints[u]);
            map.add(segments.back());
        }
        for (auto u: toRemove) {
            mGraph[v].erase(u);
            mGraph[u].erase(v);
        }
    }

    for (auto const& segment: segments) {
        map.map(segment, R"(stroke:rgb(0, 0, 0);stroke-width:1)");
    }
}
} // namespace generator