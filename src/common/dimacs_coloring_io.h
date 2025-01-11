#pragma once
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <boost/range/iterator_range.hpp>

#include <ostream>
#include <iomanip>

namespace utils {
template<typename VertexListGraph>
class DimacsColoringIO {
public:
    using GraphTraits = boost::graph_traits<VertexListGraph>;
    
    using Vertex = GraphTraits::vertex_descriptor;
    using VertexSizeType = GraphTraits::vertices_size_type;
    
    using Edge = GraphTraits::edge_descriptor;
    using EdgeSizeType = GraphTraits::edges_size_type;

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
    static void Write(VertexListGraph const& g, std::ostream &out, Commenters&&... comments) noexcept {
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
};
} // namespace utils