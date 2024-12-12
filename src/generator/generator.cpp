#include "generator.h"

#include <iostream>

namespace generator {
Generator::Generator(Parameters const& params)
{
    mGraph.resize(params.numVertices);
    mGraph.shrink_to_fit();

    mCoords.resize(params.numVertices);
    mCoords.shrink_to_fit();
}

void Generator::Generate()
{
    for (auto &[x, y]: mCoords) {
        x = rnd::mt19937::Uniform();
        y = rnd::mt19937::Uniform();
    }

    std::vector<int32_t> candidates(mGraph.size());
    std::iota(candidates.begin(), candidates.end(), 0);

    // auto const vertices = candidates;
    // while (!candidates.empty()) {
    for (int i = 0; i < 300; ++i) {
        int32_t n = static_cast<int32_t>(candidates.size());
        int32_t vIndex = rnd::mt19937::Get(0, n - 1);
        int32_t v = candidates[vIndex];
        // size_t prevSize = mSegments.size()

        for (auto _ = 0; _ < 1; ++_) {
            int32_t u = rnd::mt19937::Get(0, n - 1);

            if (v == u || mGraph[v].contains(u)) {
                continue;
            }

            mSegments.emplace_back(
                mCoords[v],
                mCoords[u],
                mSegments.size()
            );


            if (geometry::HasInersection(mSegments)) {
                mSegments.pop_back();
                continue;
            }

            mGraph[v].emplace(u);
            mGraph[u].emplace(v);
        }

        // if (mSegments.size() == prevSize) {
        //     continue;
        // }

        // std::swap(candidates[vIndex], candidates.back());
        // candidates.pop_back();

        // rnd::mt19937::Sample(
        //     vertices.begin(),
        //     vertices.end(),
        //     attemts,
        //     callback
        // );
    }

    for (size_t v = 0; v < mGraph.size(); ++v) {
        std::cout << "v " << v << ' ' << mCoords[v].x << ' ' << mCoords[v].y << std::endl;
    }

    for (size_t v = 0; v < mGraph.size(); ++v) {
        for (auto u: mGraph[v]) {
            std::cout << "e "<< v << ' ' << u << std::endl;
        }
    }
}
} // namespace generator