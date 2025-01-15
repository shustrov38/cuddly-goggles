#pragma once 

#include <cstdint>
#include <istream>
#include <cctype>

namespace solver {
enum Config: uint16_t {
    DSATUR,                 // O(n^2)
    DSATUR_BINARY_HEAP,     // O((m + n)logn)
    DSATUR_FIBONACCI_HEAP,  // O(m + nlogn)

    DSATUR_SEWELL,          // O(n^3)

    __DSATUR_BOUND,

    __END
};

std::istream &operator >>(std::istream& in, Config& config);
} // namespace solver
