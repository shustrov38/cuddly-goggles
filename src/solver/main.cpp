#include <boost/program_options.hpp>

#include <iostream>

#include <graph.h>

namespace po = boost::program_options;

int32_t main(int32_t argc, char **argv)
{
    Graph g;
    g.FromDIMACS(std::cin);
    g.ToDIMACS(std::cout);
    return EXIT_SUCCESS;
}