#pragma once

#include <unordered_set>
#include <stdlib.h>
#include <cassert>
#include <numeric>
#include <vector>

class Graph {
public:
    Graph() = default;

    explicit Graph(size_t n)
    {
        Init(n);
    }

    void Init(size_t n)
    {
        mNumVerts = n;
        mNumEdges = 0;
        mNeighbours.assign(mNumVerts, std::unordered_set<int32_t>());
        mVertices.resize(mNumVerts);
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
                out << "e " << v + 1 << ' ' << u + 1 << '\n';
            }
        }
        out.flush();
    }

    void FromDIMACS(std::istream &in) 
    {
        char ch;
        std::string line;

        // reading comments
        while (std::getline(in, line) && line.starts_with('c'));
        
        int32_t n;
        int32_t m;
        std::stringstream ss(line);
        ss >> ch >> line >> n >> m;
        Init(n);

        for (int32_t i = 0, v, u; i < m; ++i) {
            in >> ch >> v >> u;
            AddEdge(v - 1, u - 1);
        }
    }

private:
    int32_t mNumVerts;
    int32_t mNumEdges;
    std::vector<std::unordered_set<int32_t>> mNeighbours;
    std::vector<int32_t> mVertices;
};