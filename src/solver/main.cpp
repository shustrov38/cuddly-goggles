#include <boost/graph/connected_components.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include <boost/program_options.hpp>

#include <boost/timer/timer.hpp>

#include <iostream>

#include "heuristics/DSatur.h"

struct VertexProperty {
    int32_t index;

    boost::default_color_type used {boost::white_color};
    int32_t component { 0 };

    int32_t color { 0 };

    explicit VertexProperty(int32_t i)
        : index(i)
    {
    }
};

using Graph = boost::adjacency_list<
    boost::vecS, boost::hash_setS, boost::undirectedS,
    // vertex property
    VertexProperty,
    // edge property
    boost::no_property
>;
using GraphTraits = boost::graph_traits<Graph>;
using Edge = GraphTraits::edge_descriptor;
using Vertex = GraphTraits::vertex_descriptor;

namespace po = boost::program_options;

void FromDIMACS(std::istream &in, Graph &g, std::vector<Vertex> &vertices)
{
    char ch;
    std::string line;
    while (std::getline(in, line) && line.starts_with('c'));
    
    int32_t n;
    int32_t m;
    std::stringstream ss(line);
    ss >> ch >> line >> n >> m;
    assert(ch == 'p');

    vertices.clear();
    for (int32_t i = 0; i < n; ++i) {
        vertices.emplace_back(boost::add_vertex(VertexProperty(i), g));
    }

    for (int32_t i = 0, v, u; i < m; ++i) {
        in >> ch >> v >> u;
        assert(ch == 'e');
        boost::add_edge(vertices[v - 1], vertices[u - 1], g);
    }

    std::cout << boost::num_vertices(g) << ' ' << boost::num_edges(g) << std::endl;
}

class Coloring {
public:
    enum Config: uint16_t {
        DSATUR,                 // O(n^2)
        DSATUR_BINARY_HEAP,     // O((m + n)logn)
        DSATUR_FIBONACCI_HEAP,  // O(m + nlogn)

        __END
    };

    template <class VertexListGraph, class P, class T, class R>
    static auto Process(VertexListGraph const& g, boost::bgl_named_params<P, T, R> const& params, Config config)
    {
        switch (config) {
        case DSATUR: return heuristics::DSaturSequentialVertexColoring(g, params);
        case DSATUR_BINARY_HEAP: throw std::invalid_argument("DSATUR_BINARY_HEAP is not implemented");
        case DSATUR_FIBONACCI_HEAP: return heuristics::DSaturSparseSequentialVertexColoring(g, params);
        default: throw std::invalid_argument("unknown config parameter");;
        }
    }

    template <class VertexListGraph, class ColorMap>
    static bool Validate(VertexListGraph const& g, ColorMap color)
    {
        for (auto v: boost::make_iterator_range(boost::vertices(g))) {
            for (auto u: boost::make_iterator_range(boost::adjacent_vertices(v, g))) {
                if (color[v] == color[u]) {
                    return false;
                }
            }
        }
        return true;
    }
};

int32_t main(int32_t argc, char **argv)
{
    Graph g;
    std::vector<Vertex> vertices;

    FromDIMACS(std::cin, g, vertices);
    
    int32_t ncomps = boost::connected_components(
        g,
        boost::get(&VertexProperty::component, g),
        boost::color_map(boost::get(&VertexProperty::used, g))
            .vertex_index_map(boost::get(&VertexProperty::index, g))
    );

    std::cout << "Components: " << ncomps << std::endl;
    
    boost::timer::cpu_timer t;
    auto ncolors = Coloring::Process(
        g,
        boost::color_map(boost::get(&VertexProperty::color, g))
            .vertex_index_map(boost::get(&VertexProperty::index, g)),
        static_cast<Coloring::Config>(std::stoi(argv[1]))
    );
    boost::timer::cpu_times times = t.elapsed();
    std::cout << boost::timer::format(times, 5, "%w") << 's' << std::endl;

    auto status = Coloring::Validate(g, boost::get(&VertexProperty::color, g));
    if (!status) {
        std::cout << "Bad coloring" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Found coloring N=" << ncolors << std::endl;

    return EXIT_SUCCESS;
}