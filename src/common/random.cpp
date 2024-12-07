#include "random.h"

namespace rnd::mt19937 {
static std::random_device rd {};
static std::mt19937 gen { rd() };

int32_t Get(int32_t lo, int32_t hi)
{
    std::uniform_int_distribution<int32_t> dist(lo, hi);
    return dist(gen);
}

double Uniform()
{
    std::uniform_real_distribution dist;
    return dist(gen);
}
} // namespace rnd::mt19937