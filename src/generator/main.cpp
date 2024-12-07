#include <iostream>

#include <getopt/getopt.hpp>

#include "generator.h"

int32_t main(int32_t argc, char **argv)
{
    using argparser = argparser::argparser;
    argparser opt(argc, argv);

    generator::Parameters params;

    opt
        .info("Planar Graph Generator", argv[0])
        .help({"-h", "--help"}, 
            "Display this help")
        .reg({"-n", "--vertices"},
            "NUM_VERTICES",
            argparser::required_argument,
            "Set number of graph vertices",
            [&params](std::string const &arg) {
                params.numVertices = std::stoul(arg);
            });

    try {
        opt();
    } catch (::argparser::help_requested_exception const &) {
        return EXIT_SUCCESS;
    } catch (::argparser::argument_required_exception const &e) {
        std::cerr << "\u001b[31;1mERROR: " << e.what() << "\u001b[0m\n";
        return EXIT_FAILURE;
    } catch (::argparser::unknown_option_exception const &e) {
        std::cerr << "\u001b[31;1mERROR: " << e.what() << "\u001b[0m\n";
        opt.display_help(); 
        return EXIT_FAILURE;
    } catch (std::exception const &e) {
        std::cerr << "\u001b[31;1mERROR: " << e.what() << "\u001b[0m\n";
        return EXIT_FAILURE;
    }

    generator::Generator gen(params);
    gen.Generate();

    return EXIT_SUCCESS;
}