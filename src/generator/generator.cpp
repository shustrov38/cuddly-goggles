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
}
} // namespace generator