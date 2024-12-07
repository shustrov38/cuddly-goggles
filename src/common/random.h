#pragma once

#include <random>

namespace rnd {
namespace mt19937 {
int32_t Get(int32_t lo, int32_t hi);
double Uniform();
} // namespace mt19937
} // namespace rnd
