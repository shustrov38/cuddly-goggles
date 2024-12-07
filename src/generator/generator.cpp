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

    // int32_t n = static_cast<int32_t>(candidates.size());
    // auto c = rnd::mt19937::Get(0, n - 1);

    auto vertices = candidates;
    rnd::mt19937::Sample(
        vertices.begin(),
        vertices.end(),
        5,
        [](auto &value) { std::cout << value << std::endl; }
    );

}
} // namespace generator