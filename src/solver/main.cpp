#include <boost/graph/connected_components.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

#include <boost/program_options.hpp>

#include <iostream>

struct VertexProperty {
    int32_t index;

    boost::default_color_type used {boost::white_color};
    int32_t component { 0 };

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
    // for (auto v: vertices) {
    //     int32_t index = boost::get(&VertexProperty::vertexIndex, g, v);
    //     int32_t comp = boost::get(&VertexProperty::vertexComponent, g, v);
    //     std::cout << index << ' ' << comp << '\n';
    // }

    return EXIT_SUCCESS;
}