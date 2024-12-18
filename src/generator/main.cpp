#include <boost/program_options.hpp>

#include <iostream>

#include "generator.h"

namespace po = boost::program_options;

bool ProcessCommandLine(int32_t argc, char **argv, generator::Parameters &params)
{
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "Produce help message")
        ("numVertices,N", po::value<size_t>(&params.numVertices)->required(), "Number of vertices in graph");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.contains("help")) {
        std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
        std::cout << desc << std::endl;
        return false;
    }

    try {
        po::notify(vm);
    } catch(std::exception& e) {
        std::cerr << "\033[31mError: " << e.what() << std::endl;
        return false;
    }
    return true;
}

int32_t main(int32_t argc, char **argv)
{
    generator::Parameters params;

    if (!ProcessCommandLine(argc, argv, params)) {
        return EXIT_FAILURE;
    }

    generator::Generator gen(params);
    gen.Generate();
    gen.ToDIMACS(std::cout);

    return EXIT_SUCCESS;
}