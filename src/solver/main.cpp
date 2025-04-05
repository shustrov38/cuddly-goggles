#include <boost/range/iterator_range_core.hpp>

#include <boost/program_options.hpp>

#include <boost/timer/timer.hpp>

#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <optional>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <thread>
#include <memory>
#include <atomic>
#include <chrono>
#include <string>
#include <limits>
#include <array>

#include <dimacs_coloring_io.h>

#include "heuristics/dsatur.h"
#include "exact/dsatur.h"
#include "coloring.h"
#include "config.h"
#include "clique.h"
#include "graph.h"

#include "ram_free/dsatur.h"

namespace fs = std::filesystem;

struct Parameters {
    std::chrono::seconds timeLimit { std::numeric_limits<int64_t>::max() };
    std::optional<fs::path> inputPath { std::nullopt };
    solver::Config config;
};

namespace po = boost::program_options;

bool ProcessCommandLine(int32_t argc, char **argv, Parameters &params)
{
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "Produce help message.")
        ("input,i", po::value<fs::path>()->composing(), "Path to DIMACS problem.")
        ("config,c", po::value<solver::Config>(&params.config),
            "Coloring implementation. Possible values:"
            " DSATUR,"
            " DSATUR_BINARY_HEAP,"
            " DSATUR_FIBONACCI_HEAP,"
            " DSATUR_SEWEL,"
            " DSATUR_PASS,"

            " BNB_DSATUR,"
            " BNB_DSATUR_SEWELL,"
            " BNB_DSATUR_PASS.")
        ("time-limit,t", po::value<int64_t>(), "Time limit");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
    } catch(std::exception& e) {
        std::cerr << "\033[31m" << "Error: " << e.what() << "\033[0m" << std::endl;
        return false;
    }

    if (vm.contains("help")) {
        std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
        std::cout << desc << std::endl;
        return false;
    }

    if (vm.contains("input")) {
        params.inputPath = vm["input"].as<fs::path>();
    }


    if (vm.contains("time-limit")) {
        params.timeLimit = std::chrono::seconds(vm["time-limit"].as<int64_t>());
    }

    try {
        po::notify(vm);
    } catch(std::exception& e) {
        std::cerr << "\033[31m" << "Error: " << e.what() << "\033[0m" << std::endl;
        return false;
    }
    return true;
}

std::unique_ptr<std::istream> CreateIstream(std::string&& source, bool fromFile) {
    if (fromFile) {
        auto ptr = std::make_unique<std::ifstream>(std::string(source));
        if (!ptr->is_open()) {
            throw std::runtime_error("Unable to open file for reading");
        }
        return ptr;
    }
    return std::make_unique<std::stringstream>(source);
}

auto ToSeconds(boost::timer::cpu_times const& times)
{
    auto nanoseconds = std::chrono::nanoseconds(times.wall);
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(nanoseconds);
    return seconds;
}

int32_t main(int32_t argc, char **argv)
{
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " trace" << std::endl;
        return EXIT_FAILURE;
    }

    std::filesystem::path path(argv[1]);
    solver::ramfree::DSatur(path);

    return EXIT_SUCCESS;

    Parameters params;
    if (!ProcessCommandLine(argc, argv, params)) {
        return EXIT_FAILURE;
    }

    solver::Graph g;

    std::unique_ptr<std::istream> streamPtr;
    try {
        if (params.inputPath) {
            streamPtr = CreateIstream(*params.inputPath, true);
        } else {
            std::ostringstream oss;
            oss << std::cin.rdbuf();
            streamPtr = CreateIstream(oss.str(), false);
        }
    } catch(std::exception& e) {
        std::cerr << "\033[31m" << "Error: " << e.what() << "\033[0m" << std::endl;
        return EXIT_FAILURE;
    }

    using DimacsIO = utils::DimacsColoringIO<solver::Graph>;
    DimacsIO::Read(g, boost::get(&solver::VertexProperty::index, g), *streamPtr);

    {
        int32_t edgeIndex = 0;
        auto edgeIndexMap = boost::get(&solver::EdgeProperty::index, g);
        for (auto e: boost::make_iterator_range(boost::edges(g))) {
            edgeIndexMap[e] = edgeIndex++;
        }
    }

    auto LB = solver::FindClique(g);
    std::cout << "Clique LB=" << LB << std::endl;

    std::atomic_bool isJobDone = false;
    boost::timer::cpu_timer jobTimer;
    std::thread timerThread([&isJobDone, &jobTimer]() {
        std::chrono::seconds constexpr delayTime { 10 };

        auto prev = std::chrono::system_clock::now();
        while (!isJobDone) {
            auto now = std::chrono::system_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - prev) < delayTime) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            prev = now;

            boost::timer::cpu_times times = jobTimer.elapsed();
            std::cout << boost::timer::format(times, 5, "Running solver... %w") << 's' << std::endl;
        }
    });

    solver::ColorType ncolors;
    boost::timer::cpu_timer t;

    auto timeLimitFunctor = [&t, &params]() {
        return ToSeconds(t.elapsed()) > params.timeLimit;
    };

    if (params.config < solver::__DSATUR_BOUND) {
        ncolors = solver::heuristics::DSatur(g, params.config, timeLimitFunctor);
    } else if (params.config < solver::__BNB_DSATUR_BOUND) {
        ncolors = solver::exact::DSatur(g, params.config, timeLimitFunctor);
    } else {
        assert("We should never be here.");
    }

    isJobDone = true;
    boost::timer::cpu_times times = t.elapsed();

    timerThread.join();

    if (ncolors == -1) {
        std::cout << "Time limit exceeded." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << boost::timer::format(times, 5, "Elapsed time: %w") << 's' << std::endl;

    if (!solver::Validate(g, boost::get(&solver::VertexProperty::color, g))) {
        std::cout << "Bad coloring." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Found coloring K=" << ncolors << std::endl;

    auto colors = boost::get(&solver::VertexProperty::color, g);
    std::array<std::set<uint64_t>, 4> colorClasses;
    for (auto v: boost::make_iterator_range(boost::vertices(g))) {
        auto c = colors[v];
        colorClasses[c].emplace(v);
    }

    for (auto& cls: colorClasses) {
        for (auto v: cls) {
            std::cout << v << ' ';
        }
        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}
