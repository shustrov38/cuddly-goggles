#include <boost/program_options.hpp>

#include <iostream>

#include <graph.h>

namespace po = boost::program_options;

int32_t main(int32_t argc, char **argv)
{
    Graph g;
    g.FromDIMACS(std::cin);

    auto components = g.SplitByComponents();
    std::cout << components.size() << std::endl;
    for (auto &comp: components) {
        std::cout << comp.size() << ' ';
    }
    std::cout << std::endl;
    g.ToDIMACS(std::cout);
    return EXIT_SUCCESS;
}