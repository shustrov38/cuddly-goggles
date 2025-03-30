#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/planar_face_traversal.hpp>

#include <boost/program_options.hpp>

#include <iostream>
#include <fstream>
#include <ostream>
#include <iomanip>
#include <vector>

#include <dimacs_coloring_io.h>
#include "generator.h"

namespace po = boost::program_options;
namespace fs = std::filesystem;

bool ProcessCommandLine(int32_t argc, char **argv, generator::Parameters &params)
{
    po::options_description desc("Options");
    desc.add_options()
        ("help,h", "Produce help message.")
        ("num-vertices,N", po::value<size_t>(&params.numVertices)->required(), "Number of vertices in graph.")
        ("remove-prop,p", po::value<double>(&params.removePropability)->default_value(0.5), "Propability of removing edge from graph.")
        ("connectivity,c", "Make graph connected (1 connected component).")
        ("export-svg", po::value<fs::path>()->composing(), "Path to SVG result.");

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

    if (vm.contains("connectivity")) {
        params.connectivity = true;
    }

    if (vm.contains("export-svg")) {
        params.svgPath = vm["export-svg"].as<fs::path>();
    }

    try {
        po::notify(vm);
    } catch(std::exception& e) {
        std::cerr << "\033[31m" << "Error: " << e.what() << "\033[0m" << std::endl;
        return false;
    }
    return true;
}

namespace user_comments {
namespace detail {
struct FaceCounter : public boost::planar_face_traversal_visitor
{
    size_t currentFaceVerts { 0 };
    std::map<size_t, size_t>& faceStats;

    FaceCounter(std::map<size_t, size_t>& stats) : faceStats(stats) {}

    void begin_traversal() { faceStats.clear(); }

    void begin_face() { currentFaceVerts = 0; }

    template <typename Vertex>
    void next_vertex(Vertex) { currentFaceVerts++; }

    void end_face() { faceStats[currentFaceVerts]++; }
};
} // namespace detail

static void ConnectedComponents(std::ostream &out, generator::Generator::Graph const& g)
{
    using VertexProperty = generator::Generator::VertexProperty;

    int32_t ncomps = 0;
    for (auto v: boost::make_iterator_range(boost::vertices(g))) {
        ncomps = std::max(ncomps, boost::get(&VertexProperty::component, g, v) + 1);
    }

    out << "c STATS: Connected components = " << ncomps << std::endl;
}

static void FaceCounts(std::ostream &out, generator::Generator::Graph const& g)
{
    using Edge = generator::Generator::Edge;
    using EdgeProperty = generator::Generator::EdgeProperty;
    using VertexProperty = generator::Generator::VertexProperty;
    
    std::vector<std::vector<Edge>> embedding(boost::num_vertices(g));

    namespace P = boost::boyer_myrvold_params; 
    bool isPlanar = boost::boyer_myrvold_planarity_test(
        P::graph = g,
        P::embedding = &embedding[0],
        P::edge_index_map = boost::get(&EdgeProperty::index, g),
        P::vertex_index_map = boost::get(&VertexProperty::index, g)
    );

    if (!isPlanar) {
        out << "c STATS: The graph is not planar :D" << std::endl;
        return;
    }

    std::map<size_t, size_t> faceStats;
    detail::FaceCounter visitor(faceStats);
    boost::planar_face_traversal(g, &embedding[0], visitor, boost::get(&EdgeProperty::index, g));

    out << "c STATS: Face vertex count distribution:" << std::endl;
    for (auto const& [vertCount, faceCount] : faceStats) {
        out << "c        " 
            << std::setw(4) << vertCount << " verts: "
            << std::setw(7) << faceCount << " face";
        if (faceCount != 1) [[likely]] {
            out << 's';
        }
        out << std::endl;
    }
}
} // namespace user_comments

int32_t main(int32_t argc, char **argv)
{
    generator::Parameters params;
    if (!ProcessCommandLine(argc, argv, params)) {
        return EXIT_FAILURE;
    }

    generator::Generator gen;
    gen.Generate(params);

    auto const& graph = gen.GetGraph();
    
    using DimacsIO = utils::DimacsColoringIO<generator::Generator::Graph>;
    using Comments = DimacsIO::Comments;

    DimacsIO::Write(graph, std::cout, 
        Comments::Description,      // SOURCE and DESCRIPTION
        Comments::Separator,        // beginning of STATS section
        Comments::Density,
        user_comments::FaceCounts,
        user_comments::ConnectedComponents,
        Comments::Separator         // beginning of problem description
    );

    if (params.svgPath) {
        std::ofstream svg(*params.svgPath);
        gen.ToSVG(svg);
        std::cout << "SVG image is written to ";
        std::cout << "\033[38;05;46m" << fs::canonical(*params.svgPath) << "\033[0m" << std::endl;
    }

    return EXIT_SUCCESS;
}