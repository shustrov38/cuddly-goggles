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
    double constexpr SCALE = 100;
    for (auto &p: mPoints) {
        bg::assign_values(p, rnd::mt19937::Uniform() * SCALE, rnd::mt19937::Uniform() * SCALE);
    }

    bp::voronoi_diagram<double> vd;
    bp::construct_voronoi(mPoints.begin(), mPoints.end(), &vd);

    std::ofstream svg("/tmp/image.svg");
    bg::svg_mapper<Point> map(svg, 1200, 600); 

    using Segment = bgm::segment<Point>;
    std::vector<Segment> segments;

    int32_t color = 0;
    for (auto &vert: vd.vertices()) {
        vert.color(++color);
    }

    double constexpr prop = 0.7;

    color = 0;
    for (auto &edge: vd.edges()) {
        if (edge.is_finite() && !edge.color()) {
            edge.color(++color);

            auto v = edge.vertex0()->color();
            auto u = edge.vertex1()->color();

            mGraph[v - 1].emplace(u - 1);
            mGraph[u - 1].emplace(v - 1);

            segments.emplace_back(*edge.vertex0(), *edge.vertex1());
            map.add(segments.back());
        }
    }

    for (auto const& segment: segments) {
        map.map(segment, R"(stroke:rgb(0, 0, 0);stroke-width:1)");
    }
}
} // namespace generator