#pragma once

#include <boost/range/iterator_range.hpp>

#include "../selectors/dsatur_sparse_selector.h"
#include "../selectors/dsatur_dense_selector.h"
#include "../selectors/dsatur_sewell_selector.h"
#include "../selectors/dsatur_pass_selector.h"

#include "../config.h"
#include "../graph.h"

namespace solver::heuristics {
ColorType DSatur(Graph &g, Config config, TimeLimitFuncCRef timeLimitFunctor);
} // namespace solver::heuristics
