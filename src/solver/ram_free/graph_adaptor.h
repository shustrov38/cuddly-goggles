#pragma once

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <atomic>
#include <vector>

#include <sys/mman.h>
#include <sys/stat.h>

#include <omp.h>

#include <tqdm.hpp>

namespace solver::ramfree {
class MyTQDM
{
public:
    MyTQDM(tq::index num_iters)
        : num_iters_(num_iters)
    {}

    MyTQDM(const MyTQDM&) = delete;
    MyTQDM(MyTQDM&&) = delete;
    MyTQDM& operator=(MyTQDM&&) = delete;
    MyTQDM& operator=(const MyTQDM&) = delete;
    ~MyTQDM() = default;

    void set_ostream(std::ostream& os) { bar_.set_ostream(os); }
    void set_prefix(std::string s) { bar_.set_prefix(std::move(s)); }
    void set_bar_size(int size) { bar_.set_bar_size(size); }
    void set_min_update_time(double time) { bar_.set_min_update_time(time); }

    void manually_set_progress(double to)
    {
        tq::clamp(to, 0, 1);
        iters_done_ = std::round(to*num_iters_);
        bar_.update(calc_progress());
    }

private:
    double calc_progress() const
    {
        double denominator = num_iters_;
        if (num_iters_ == 0) denominator += 1e-9;
        return iters_done_/denominator;
    }

    tq::index num_iters_{0};
    tq::index iters_done_{0};
    tq::progress_bar bar_;
};

template <class Info>
concept NodeInfo = requires(Info info) {
    { info.degree };
    { info.neighbours };
};

template <NodeInfo Node>
class GraphAdaptor {
public:
    explicit GraphAdaptor(std::filesystem::path const& path)
    {
        int fd = open64(path.c_str(), O_RDONLY | O_NONBLOCK);
        if(fd == -1) {
            throw std::runtime_error("Cannot open file");
        }

        struct stat st;
        fstat(fd, &st);
        size_t fileSize = st.st_size;

        void* mapped = mmap(nullptr, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
        if(mapped == MAP_FAILED) {
            throw std::runtime_error("mmap failed");
        }
        close(fd);

        const uint8_t* data = static_cast<const uint8_t*>(mapped);

        const char *version = reinterpret_cast<const char *>(data);
        data += 8;

        if (std::memcmp(MAGIC_VER_1_0, version, 8)) {
            throw std::runtime_error("provided trace have unknown format");
        }

        mNumVertices = *reinterpret_cast<const uint32_t*>(data);
        data += 4;
        mNumEdges = *reinterpret_cast<const uint64_t*>(data);
        data += 8;

        const uint64_t* offsets = reinterpret_cast<const uint64_t*>(data);
        data += static_cast<uint64_t>(mNumVertices) * sizeof(uint64_t);

        mVertices.resize(mNumVertices);
        constexpr uint32_t chunkSize = 100;

        auto tqdm = MyTQDM(mNumVertices);
        tqdm.set_prefix("Reading graph    ");

        std::atomic_uint32_t progress = 0;

        #pragma omp parallel
        #pragma omp single
        {
            #pragma omp task
            {
                uint32_t lastProgress = -1;
                tqdm.manually_set_progress(0);

                while (progress < mNumVertices) {
                    if (progress != lastProgress) {
                        tqdm.manually_set_progress(static_cast<double>(progress) / mNumVertices);
                        lastProgress = progress;
                    }
                    #pragma omp taskyield
                }

                tqdm.manually_set_progress(1);
                std::cout << std::endl;
            }

            #pragma omp taskgroup
            {
                for(uint32_t v = 0; v < mNumVertices; v += chunkSize) {
                    #pragma omp task firstprivate(v)
                    {
                        const uint32_t end = std::min(v + chunkSize, mNumVertices);
                        for(uint32_t i = v; i < end; ++i) {
                            const uint8_t* meta = static_cast<const uint8_t*>(mapped) + offsets[i];

                            mVertices[i].degree = *meta++;
                            const uint32_t* neighbors = reinterpret_cast<const uint32_t*>(meta);
                            std::memcpy(mVertices[i].neighbours, neighbors, mVertices[i].degree * sizeof(uint32_t));
                        }
                        progress += end - v;
                    }
                }
            }
        }

        // #pragma omp parallel for schedule(dynamic, 100)
        // for (uint32_t v = 0; v < mNumVertices; ++v) {
        //     const uint8_t* meta = static_cast<const uint8_t*>(mapped) + offsets[v];
        
        //     mVertices[v].degree = *meta++;            
        //     const uint32_t* neighbors = reinterpret_cast<const uint32_t*>(meta);
        //     std::memcpy(mVertices[v].neighbours, neighbors, mVertices[v].degree * sizeof(uint32_t));
        // }

        munmap(mapped, fileSize);
    }

    inline uint64_t GetNumEdges() const noexcept
    {
        return  mNumEdges;
    }

    inline uint32_t GetNumVertices() const noexcept
    {
        return mNumVertices;
    }

    Node const& GetVertex(uint32_t vertex) const
    {
        return mVertices[vertex];
    }

    Node &GetVertex(uint32_t vertex)
    {
        return mVertices[vertex];
    }

private:
    static constexpr char MAGIC_VER_1_0[] = "DSHUV1.0";

    std::vector<Node> mVertices;

    uint32_t mNumVertices;
    uint64_t mNumEdges;
};
} // namespace solver::ramfree