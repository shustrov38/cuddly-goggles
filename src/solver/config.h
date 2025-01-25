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
    DSATUR_PASS,            // O(n^3) in worst case, O(n^2) on average

    __DSATUR_BOUND,

    BNB_DSATUR,             // BnB with O(n^2) check    
    BNB_DSATUR_SEWELL,      // BnB with O(n^3) check
    BNB_DSATUR_PASS,        // BnB with O(n^3) in worst case, O(n^2) on average

    __BNB_DSATUR_BOUND,

    __END
};

std::istream &operator >>(std::istream& in, Config& config);
} // namespace solver
