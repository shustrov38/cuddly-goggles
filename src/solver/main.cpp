#include <boost/graph/connected_components.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include <boost/program_options.hpp>

#include <boost/timer/timer.hpp>

#include <iostream>

#include "coloring.h"

struct VertexProperty {
    int32_t index;

    boost::default_color_type used { boost::white_color };
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

    // abscence of parallel edges
    std::set<std::pair<int32_t, int32_t>> edges;

    for (int32_t i = 0, v, u; i < m; ++i) {
        in >> ch >> v >> u;
        assert(ch == 'e');
        --v, --u;
        if (!edges.contains(std::minmax(v, u))) {
            boost::add_edge(vertices[v], vertices[u], g);
            edges.emplace(std::minmax(v, u));
        }
    }

    std::cout << boost::num_vertices(g) << ' ' << boost::num_edges(g) << std::endl;
}

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
    auto ncolors = solver::Process(
        g,
        boost::color_map(boost::get(&VertexProperty::color, g))
            .vertex_index_map(boost::get(&VertexProperty::index, g))
            .vertex_index1_map(boost::get(&VertexProperty::component, g)),
        static_cast<solver::Config>(std::stoi(argv[1]))
    );
    boost::timer::cpu_times times = t.elapsed();
    std::cout << boost::timer::format(times, 5, "%w") << 's' << std::endl;

    auto status = solver::Validate(g, boost::get(&VertexProperty::color, g));
    if (!status) {
        std::cout << "Bad coloring" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Found coloring N=" << ncolors << std::endl;




    // std::cout << std::endl;

    // auto order = boost::get(&VertexProperty::index, g);
    // auto color = boost::get(&VertexProperty::color, g);
    // auto order1 = boost::get(&VertexProperty::component, g);

    // for (auto v: boost::make_iterator_range(boost::vertices(g))) {
    //     std::cout << '(' << order[v] << ", {\'label\': \'" << color[v] << " | " << order1[v] << "\', \'degree\': " << boost::out_degree(v, g) << "}),\n";
    // }

    // std::cout << std::endl;

    // for (auto e: boost::make_iterator_range(boost::edges(g))) {
    //     auto v = boost::source(e, g);
    //     auto u = boost::target(e, g);
    //     std::cout << '(' << order[v] << ", " << order[u] << "),\n";
    // }

    return EXIT_SUCCESS;
}
