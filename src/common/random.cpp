#include "random.h"

namespace rnd::mt19937 {
namespace detail {
std::random_device rd {};
std::mt19937 gen { rd() };
} // namespace detail

int32_t Get(int32_t lo, int32_t hi)
{
    std::uniform_int_distribution<int32_t> dist(lo, hi);
    return dist(detail::gen);
}

double Uniform()
{
    std::uniform_real_distribution dist;
    return dist(detail::gen);
}
} // namespace rnd::mt19937