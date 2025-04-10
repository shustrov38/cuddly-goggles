#include "dsatur.h"
#include "boost/heap/policies.hpp"
#include "graph_adaptor.h"

#include <boost/heap/fibonacci_heap.hpp>

#include <cstdint>
#include <cstring>
#include <bit>

#include <omp.h>

#include <tqdm.hpp>
#include <debug.h>
#include <vector>

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

struct Node {
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

    using Heap = HeapT<const Node *, Node::CompareInfo>;
    using Handle = typename Heap::handle_type;

    uint32_t vertex;
    uint8_t degree;

    detail::MexBitSet bitset {};

    uint8_t color = -1;
    uint8_t used { false };

    uint32_t neighbours[8];

    Handle handle;

    Node() = default;

    explicit Node(uint32_t vertex, uint8_t degree)
        : vertex(vertex), degree(degree)
    {}
};
static_assert(sizeof(Node) == 48);

static void HeapLoader(GraphAdaptor<Node> &ada, Node::Heap &heap)
{
    uint32_t const numVertices = ada.GetNumVertices();
    uint32_t constexpr batchSize = 1000;

    std::vector<Node::Heap> threadHeap(omp_get_max_threads());

    auto tqdm_heap = MyTQDM(numVertices);
    tqdm_heap.set_prefix("Loading heaps    ");
    tqdm_heap.manually_set_progress(0);

    #pragma omp parallel for schedule(dynamic, batchSize)
    for (uint32_t v = 0; v < numVertices; ++v) {
        auto &vnode = ada.GetVertex(v);
        vnode.vertex = v;
        vnode.handle = threadHeap[omp_get_thread_num()].push(&vnode);
    }
    tqdm_heap.manually_set_progress(1);
    std::cout << std::endl;

    auto tqdm_merge = MyTQDM(numVertices);
    tqdm_merge.set_prefix("Merging heaps    ");
    tqdm_merge.manually_set_progress(0);

    for (auto &h: threadHeap) {
        heap.merge(h);
    }
    tqdm_merge.manually_set_progress(1);
    std::cout << std::endl;
}

static int32_t DsaturImpl(GraphAdaptor<Node> &ada, Node::Heap &heap)
{
    uint32_t const numVertices = ada.GetNumVertices();
    uint32_t constexpr batchSize = 1000;
    
    uint8_t maxColor = 0;
    double progress = 0;

    auto tqdm_algo = MyTQDM(numVertices);
    tqdm_algo.set_prefix("Colored vertices ");
    for (uint32_t batch = 0; batch < numVertices; batch += batchSize) {
        uint32_t end = std::min(batch + batchSize, numVertices);
        for (uint32_t _ = batch; _ < end; ++_) {
            uint32_t u = heap.top()->vertex;
            heap.pop();
            
            auto &unode = ada.GetVertex(u);
            unode.color = unode.bitset.Mex();
            maxColor = std::max(maxColor, unode.color);
            unode.used = true;
            
            for (uint8_t i = 0; i < unode.degree; ++i) {
                uint32_t v = unode.neighbours[i];
                auto &vnode = ada.GetVertex(v);
                if (vnode.used) continue;       
                vnode.bitset.AddMexBit(unode.color);
                heap.increase(vnode.handle);
            }
        }
        progress += (end - batch);
        tqdm_algo.manually_set_progress(progress / numVertices);
    }
    std::cout << std::endl;

    return maxColor;
}

static bool Validate(GraphAdaptor<Node> &ada)
{
    uint32_t const numVertices = ada.GetNumVertices();
    uint32_t constexpr batchSize = 1000;

    double progress = 0;
    bool flag = false;

    auto tqdm_vali = MyTQDM(numVertices);
    tqdm_vali.set_prefix("Validating       ");

    #pragma omp parallel for
    for (uint32_t batch = 0; batch < numVertices; batch += batchSize) {
        uint32_t end = std::min(batch + batchSize, numVertices);
        if (flag) {
            continue;
        }
        for (uint32_t u = batch; u < end; ++u) {
            auto const& unode = ada.GetVertex(u);
            for (uint8_t i = 0; i < unode.degree; ++i) {
                uint32_t v = unode.neighbours[i];
                auto const& vnode = ada.GetVertex(v);
                if (vnode.color == unode.color) {
                    flag = true;
                }
            }
        }
        progress += (end - batch);
        tqdm_vali.manually_set_progress(progress / numVertices);
    }
    tqdm_vali.manually_set_progress(1);
    std::cout << std::endl;
    return !flag;
}

void DSatur(std::filesystem::path const& path)
{
    solver::ramfree::GraphAdaptor<Node> ada(path);

    Node::Heap heap;
    HeapLoader(ada, heap);

    auto maxColor = DsaturImpl(ada, heap);
    
    if (Validate(ada)) {
        std::cout << "Coloring with K=" << (int)maxColor+1 << std::endl;
    } else {
        std::cout << "Bad coloring!" << std::endl;
    }    
}
} // namespace solver::ramfree