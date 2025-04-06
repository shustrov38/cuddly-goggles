#pragma once
#include "boost/graph/subgraph.hpp"
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <boost/range/iterator_range.hpp>

#include <ostream>
#include <iomanip>
#include <string>

namespace utils {
template<typename VertexListGraph>
class DimacsColoringIO {
public:
    using GraphTraits = boost::graph_traits<VertexListGraph>;
    
    using Vertex = typename GraphTraits::vertex_descriptor;
    using VertexSizeType = typename GraphTraits::vertices_size_type;
    
    using Edge = typename GraphTraits::edge_descriptor;
    using EdgeSizeType = typename GraphTraits::edges_size_type;

    struct Comments {
        static void Description(std::ostream &out, VertexListGraph const&) noexcept
        {
            out << "c SOURCE: Dmitriy Shustrov (shustrov38@gmail.com)" << std::endl;
            out << "c DESCRIPTION: Planar graph based on Voronoi Diagram with random edges removed." << std::endl;
        }

        static void Density(std::ostream &out, VertexListGraph const& g) noexcept
        {
            auto numVerts = boost::num_vertices(g);
            auto numEdges = boost::num_edges(g);

            double avgDegree = 0;
            for (auto v: boost::make_iterator_range(boost::vertices(g))) {
                avgDegree += boost::out_degree(v, g);
            }
            avgDegree /= numVerts;

            double maxPossibleEdges = (numVerts * (numVerts - 1)) / 2.0;
            double edgeDensity = numEdges / maxPossibleEdges;

            out << "c STATS: Average vertex degree = " << std::fixed << std::setprecision(2) << avgDegree << std::endl;
            out << "c STATS: Edge density = " << std::fixed << std::setprecision(4) << edgeDensity << std::endl;
        }

        static void Separator(std::ostream &out, VertexListGraph const&) noexcept
        {
            out << "c " << std::endl;
        }
    };

    template <class... Commenters>
    static void Write(VertexListGraph const& g, std::ostream &out, Commenters&&... comments) noexcept 
    {
        // Write problem comments
        // Format: c comments
        (comments(out, g), ...);

        auto numVerts = boost::num_vertices(g);
        auto numEdges = boost::num_edges(g);

        // Write problem line
        // Format: p edge <vertices> <edges>
        out << "p edge " << numVerts << " " << numEdges << "\n";

        std::vector<Vertex> vertices;
        vertices.reserve(numVerts);
        for (auto v: boost::make_iterator_range(boost::vertices(g))) {
            vertices.emplace_back(v);
        }

        std::map<Vertex, VertexSizeType> order;
        for (VertexSizeType i = 0; i < vertices.size(); ++i) {
            order[vertices[i]] = i;
        }

        // Write edge lines
        // Format: e <vertex1> <vertex2>
        for (auto e: boost::make_iterator_range(boost::edges(g))) {
            Vertex source = boost::source(e, g);
            Vertex target = boost::target(e, g);

            // DIMACS format uses 1-based vertex numbering
            VertexSizeType u = order[source] + 1;
            VertexSizeType v = order[target] + 1;

            out << "e " << u << " " << v << "\n";
        }
    }

    template <typename Order>
    static void Read(VertexListGraph &g, Order order, std::istream &in)
    {
        VertexSizeType numVertices = 0;
        EdgeSizeType numEdges = 0;

        bool headerParsed = false;

        for (std::string line; std::getline(in, line);) {
            if (line.empty()) {
                continue;
            }

            if (line[0] == 'c') {
                continue;
            } else if (line[0] == 'p') {
                std::istringstream iss(line);
                char p;
                std::string type;
                iss >> p >> type >> numVertices >> numEdges;
                headerParsed = true;
            }
        }

        if (!headerParsed) {
            throw std::runtime_error("There is no problem description");
        }

        g.clear();
        
        for (size_t i = 0; i < numVertices; ++i) {
            Vertex v = boost::add_vertex({}, g);
            order[v] = i;
        }

        in.clear();
        in.seekg(0, std::ios::beg);
        
        for (std::string line; std::getline(in, line);) {
            if (line.empty() || line.starts_with('c') || line.starts_with('p')) {
                continue;
            }

            if (line.starts_with('e')) {
                std::istringstream iss(line);
                char e;
                std::size_t u, v;
                iss >> e >> u >> v;
                boost::add_edge(u - 1, v - 1, g);
            }
        }
    }
};
} // namespace utils