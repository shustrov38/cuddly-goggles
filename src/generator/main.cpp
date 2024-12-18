#include <boost/program_options.hpp>

#include <iostream>

#include "generator.h"

namespace po = boost::program_options;
namespace fs = std::filesystem;

bool ProcessCommandLine(int32_t argc, char **argv, generator::Parameters &params)
{
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "Produce help message")
        ("numVertices,N", po::value<size_t>(&params.numVertices)->required(), "Number of vertices in graph")
        ("export-svg", po::value<fs::path>()->composing(), "Path to SVG result");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.contains("help")) {
        std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
        std::cout << desc << std::endl;
        return false;
    }

    if (vm.contains("export-svg")) {
        params.svgPath = vm["export-svg"].as<fs::path>();
    }

    try {
        po::notify(vm);
    } catch(std::exception& e) {
        std::cerr << "\033[31m" << "Error: " << e.what() << std::endl;
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

    if (params.svgPath) {
        std::ofstream svg(*params.svgPath);
        gen.ToSVG(svg);
        std::cout << "SVG image is written to ";
        std::cout << "\033[38;05;46m" << fs::canonical(*params.svgPath) << "\033[0m" << std::endl;
    }

    return EXIT_SUCCESS;
}