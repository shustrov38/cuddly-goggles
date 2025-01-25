#include "config.h"

namespace solver {
std::istream &operator>>(std::istream& in, Config& config)
{
    std::string token;
    in >> token;

    for (auto &ch: token) {
        ch = std::toupper(ch);
    }

    if (token == "DSATUR") {
        config = DSATUR;
    } else if (token == "DSATUR_BINARY_HEAP") {
        config = DSATUR_BINARY_HEAP;
    } else if (token == "DSATUR_FIBONACCI_HEAP") {
        config = DSATUR_FIBONACCI_HEAP;
    } else if (token == "DSATUR_SEWELL") {
        config = DSATUR_SEWELL;
    } else if (token == "DSATUR_PASS") {
        config = DSATUR_PASS;

    } else if (token == "BNB_DSATUR") {
        config = BNB_DSATUR;
    } else if (token == "BNB_DSATUR_SEWELL") {
        config = BNB_DSATUR_SEWELL;
    } else if (token == "BNB_DSATUR_PASS") {
        config = BNB_DSATUR_PASS;

    } else {
        in.setstate(std::ios_base::failbit);
    }
    return in;
}
} // namespace solver
