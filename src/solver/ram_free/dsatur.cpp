#include "dsatur.h"

#include <boost/heap/fibonacci_heap.hpp>

#include <cstdint>
#include <cstring>
#include <mutex>
#include <bit>

#include <omp.h>

#include <tqdm.hpp>
#include <debug.h>

namespace solver::ramfree {
namespace detail {
template <size_t BITS>
constexpr std::array<uint8_t, 1 << BITS> GenerateMexTable()
{
    static_assert(1 <= BITS && BITS <= 8);
    std::array<uint8_t, 1 << BITS> table{};
    for (int mask = 0; mask < (1 << BITS); ++mask) {
        uint8_t mex = 0;
        while (mask & (1 << mex)) {
            mex++;
        }
        table[mask] = mex;
    }
    return table;
}

struct MexBitSet {
    uint8_t bits;
    static constexpr auto MEX_TABLE = detail::GenerateMexTable<6>();

    MexBitSet() : bits(0) {}

    uint8_t Popcount() const
    {
        return std::popcount(bits);
    }

    uint8_t Mex() const
    {
        return MEX_TABLE[bits];
    }

    void AddMexBit()
    {
        bits |= (1 << Mex());
    }

    void AddMexBit(int32_t bit)
    {
        bits |= (1 << bit);
    }

    void PrintBits() const
    {
        for (int i = 7; i >= 0; i--) {
            std::cerr << ((bits >> i) & 1);
        }
        std::cerr << '\n';
    }
};
static_assert(sizeof(MexBitSet) == 1);
} // namespace detail

template <typename T, typename C>
using HeapT = boost::heap::fibonacci_heap<T, boost::heap::compare<C>>;
using Vertex = GraphAdaptor::VertexT;

struct Node {
    Vertex vertex;
    uint8_t degree;

    detail::MexBitSet bitset {};

    uint8_t color;
    uint8_t used { false };

    uint32_t neightbours[8];


    Node() = default;

    explicit Node(Vertex vertex, uint8_t degree)
        : vertex(vertex), degree(degree)
    {}

    struct CompareInfo {
        bool operator()(const Node *lhs, const Node *rhs) const
        {
            if (lhs->bitset.Popcount() != rhs->bitset.Popcount()) {
                return lhs->bitset.Popcount() < rhs->bitset.Popcount();
            }
            if (lhs->degree != rhs->degree) {
                return lhs->degree < rhs->degree;
            }
            return lhs->vertex > rhs->vertex;
        }
    };
};
static_assert(sizeof(Node) == 40);

using Heap = HeapT<const Node *, Node::CompareInfo>;
using Handle = typename Heap::handle_type;

void DSatur(GraphAdaptor &ada)
{
    uint32_t const numVertices = ada.GetNumVertices();
    std::vector<Node> graph(numVertices);
    
    std::mutex m;
    auto tqdm_read = tq::trange(numVertices);
    tqdm_read.set_prefix("Reading graph    ");

#pragma omp parallel for firstprivate(ada)
    for (uint32_t v = 0; v < numVertices; ++v) {
        auto &ne = ada.GetNeighhbours(v);
        
        graph[v].vertex = v;
        graph[v].degree = ne.size();
        std::memcpy(graph[v].neightbours, ne.data(), ne.size() * sizeof(uint32_t));

        std::lock_guard g(m);
        tqdm_read.update();
    }
    std::cout << std::endl;

    std::vector<Handle> handles(numVertices);
    Heap heap;

    auto tqdm_heap = tq::trange(numVertices);
    tqdm_heap.set_prefix("Loading heap     ");

    for (uint32_t v: tqdm_heap) {
        handles[v] = heap.push(&graph[v]);
    }
    std::cout << std::endl;

    auto tqdm_algo = tq::trange(numVertices);
    tqdm_algo.set_prefix("Colored vertices ");

    for (uint32_t _: tqdm_algo) {
        Vertex u = heap.top()->vertex;
        heap.pop();
        
        graph[u].color = graph[u].bitset.Mex();
        graph[u].used = true;
        
        for (uint8_t i = 0; i < graph[u].degree; ++i) {
            uint32_t v = graph[u].neightbours[i];    
            if (graph[v].used) continue;       
            graph[v].bitset.AddMexBit(graph[u].color);
            heap.increase(handles[v]);
        }
    }
    std::cout << std::endl;
}
} // namespace solver::ramfree