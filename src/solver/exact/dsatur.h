#pragma once

#include <boost/range/iterator_range.hpp>

#include <vector>
#include <stack>

#include "../selectors/dsatur_dense_selector.h"
#include "../selectors/dsatur_sewell_selector.h"
#include "../selectors/dsatur_pass_selector.h"

#include "../graph.h"
#include "../config.h"

namespace solver::exact {
ColorType DSatur(Graph &g, Config config, TimeLimitFuncCRef timeLimitFunctor);
} // namespace solver::exact