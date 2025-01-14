#include <boost/timer/timer.hpp>

#include <iostream>
#include <sstream>

#include <dimacs_coloring_io.h>

#include "heuristics/dsatur.h"
#include "graph.h"

#include "coloring.h"

int32_t main(int32_t argc, char **argv)
{
    solver::Graph g;

    using DimacsIO = utils::DimacsColoringIO<solver::Graph>;
    std::stringstream ss;
    ss << std::cin.rdbuf();
    DimacsIO::Read(g, boost::get(&solver::VertexProperty::index, g), ss);

    boost::timer::cpu_timer t;
    auto ncolors = solver::heuristics::DSatur(g, solver::DSATUR_FIBONACCI_HEAP);
    boost::timer::cpu_times times = t.elapsed();
    std::cout << boost::timer::format(times, 5, "%w") << 's' << std::endl;

    auto status = solver::Validate(g, boost::get(&solver::VertexProperty::color, g));
    if (!status) {
        std::cout << "Bad coloring" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Found coloring N=" << ncolors << std::endl;

    return EXIT_SUCCESS;
}
