#include "dsatur.h"
#include "boost/heap/policies.hpp"
#include "graph_adaptor.h"

#include <boost/heap/fibonacci_heap.hpp>
#include <boost/heap/d_ary_heap.hpp>
#include <boost/pool/pool_alloc.hpp>

#include <cstdint>
#include <cstring>
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
using HeapT = boost::heap::fibonacci_heap<
    T,
    boost::heap::compare<C>,
    boost::heap::allocator<boost::fast_pool_allocator<T>>
>;

// template <typename T, typename C>
// using HeapT = boost::heap::d_ary_heap<
//     T,
//     boost::heap::compare<C>,
//     boost::heap::arity<8>,
//     boost::heap::mutable_<true>,
//     boost::heap::allocator<boost::fast_pool_allocator<T>>
// >;

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

void DSatur(std::filesystem::path const& path)
{
    solver::ramfree::GraphAdaptor<Node> ada(path);

    uint32_t const numVertices = ada.GetNumVertices();
    Node::Heap heap;

    constexpr uint32_t batchSize = 100;
    double progress = 0;

    auto tqdm_heap = MyTQDM(numVertices);
    tqdm_heap.set_prefix("Loading heap     ");
    progress = 0;
    for (uint32_t batch = 0; batch < numVertices; batch += batchSize) {
        uint32_t end = std::min(batch + batchSize, numVertices);
        for (uint32_t v = batch; v < end; ++v) {
            auto &vnode = ada.GetVertex(v);
            vnode.vertex = v;
            vnode.handle = heap.push(&vnode);
        }
        progress += (end - batch);
        tqdm_heap.manually_set_progress(progress / numVertices);
    }
    std::cout << std::endl;

    uint8_t maxColor = 0;

    auto tqdm_algo = MyTQDM(numVertices);
    tqdm_algo.set_prefix("Colored vertices ");
    progress = 0;
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

    auto tqdm_vali = MyTQDM(numVertices);
    tqdm_vali.set_prefix("Validating       ");
    progress = 0;
    auto res = [&]() {
        for (uint32_t batch = 0; batch < numVertices; batch += batchSize) {
            uint32_t end = std::min(batch + batchSize, numVertices);
            for (uint32_t u = batch; u < end; ++u) {
                auto const& unode = ada.GetVertex(u);
                for (uint8_t i = 0; i < unode.degree; ++i) {
                    uint32_t v = unode.neighbours[i];
                    auto const& vnode = ada.GetVertex(v);
                    if (vnode.color == unode.color) {
                        return false;
                    }
                }
            }
            progress += (end - batch);
            tqdm_vali.manually_set_progress(progress / numVertices);
        }
        return true;
    }();
    std::cout << std::endl;
    if (!res) {
        std::cout << "Bad coloring!" << std::endl;
    } else {
        std::cout << "Coloring with K=" << (int)maxColor+1 << std::endl;
    }    
}
} // namespace solver::ramfree