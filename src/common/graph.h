#pragma once

#include <unordered_set>
#include <stdlib.h>
#include <cassert>
#include <vector>

class Graph {
public:
    Graph(size_t n)
        : mNumVerts(n)
        , mNumEdges(0)
        , mNeighbours(mNumVerts)
        , mVertices(mNumVerts)
    {
        std::iota(mVertices.begin(), mVertices.end(), 0);
    }

    void AddEdge(int32_t v, int32_t u)
    {
        assert(v < mNumVerts && u < mNumVerts);
        if (mNeighbours[v].contains(u)) {
            return;
        }
        mNeighbours[v].emplace(u);
        mNeighbours[u].emplace(v);
        ++mNumEdges;
    }

    void RemoveEdge(int32_t v, int32_t u)
    {
        assert(v < mNumVerts && u < mNumVerts);
        if (!mNeighbours[v].contains(u)) {
            return;
        }
        mNeighbours[v].erase(u);
        mNeighbours[u].erase(v);
        --mNumEdges;
    }

    auto const& Vertices() const
    {
        return mVertices;
    }

    auto const& Neighbours(int32_t v) const
    {
        return mNeighbours[v];
    }

    void ToDIMACS(std::ostream &out) const
    {
        out << "p edge " << mNumVerts << ' ' << mNumEdges * 2 << std::endl;
        for (auto v: Vertices()) {
            for (auto u: Neighbours(v)) {
                out << v + 1 << ' ' << u + 1 << '\n';
            }
        }
        out.flush();
    }

private:
    int32_t mNumVerts;
    int32_t mNumEdges;
    std::vector<std::unordered_set<int32_t>> mNeighbours;
    std::vector<int32_t> mVertices;
};