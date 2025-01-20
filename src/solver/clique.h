#pragma once

#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/planar_face_traversal.hpp>

#include <unordered_map>

#include "graph.h"

#include <vector>

namespace solver {
struct CliqueFinder : public boost::planar_face_traversal_visitor
{
    std::vector<SizeType> face;    

    std::unordered_map<SizeType, size_t> containingTriangles;
    std::unordered_map<SizeType, size_t> otherFaces;

    int32_t &answer;

    explicit CliqueFinder(int32_t &answer) : answer(answer) {}

    void begin_face() { face.resize(0); }

    template <typename Vertex>
    void next_vertex(Vertex v) { face.emplace_back(v); }

    void end_face()
    {
        auto &container = (face.size() == 3 ? containingTriangles : otherFaces);

        for (auto v: face) {
            container[v]++;
        }
    }

    void end_traversal()
    {
        for (auto [v, cnt]: containingTriangles) {
            if (cnt == 3 && !otherFaces.contains(v)) {
                answer = 4;
                return; 
            }
        }

        if (!containingTriangles.empty()) {
            answer = 3;
            return;
        }

        answer = -1;
    }
};

inline int32_t FindClique(Graph const& g)
{
    std::vector<std::vector<Edge>> embedding(boost::num_vertices(g));

    namespace P = boost::boyer_myrvold_params; 
    bool isPlanar = boost::boyer_myrvold_planarity_test(
        P::graph = g,
        P::embedding = &embedding[0],
        P::edge_index_map = boost::get(&EdgeProperty::index, g),
        P::vertex_index_map = boost::get(&VertexProperty::index, g)
    );

    if (!isPlanar) {
        return -1;
    }

    int32_t answer;
    CliqueFinder visitor(answer);
    boost::planar_face_traversal(g, &embedding[0], visitor, boost::get(&EdgeProperty::index, g));

    return answer;
}
} // namespace solver