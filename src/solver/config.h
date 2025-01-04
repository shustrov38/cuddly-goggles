#pragma once 

#include <cstdint>

namespace solver {
enum Config: uint16_t {
    DSATUR,                 // O(n^2)
    DSATUR_BINARY_HEAP,     // O((m + n)logn)
    DSATUR_FIBONACCI_HEAP,  // O(m + nlogn)

    __DSATUR_BOUND,

    __END
};
} // namespace solver
