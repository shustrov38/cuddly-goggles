#pragma once

#include <random>
#include <iterator>

#include "callback_iterator.h"

namespace rnd {
namespace mt19937 {
namespace detail {
extern std::mt19937 gen; 
} // namespace detail

int32_t Get(int32_t lo, int32_t hi);
double Uniform();

template <typename ForwardIt, typename Func>
void Sample(ForwardIt begin, ForwardIt end, size_t count, Func callback)
{
    using T = std::decay_t<typename std::iterator_traits<ForwardIt>::value_type>;
    std::sample(
        begin, end, 
        utils::CallbackIterator<T>(callback),
        count, detail::gen
    );
}
} // namespace mt19937
} // namespace rnd
