#pragma once

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>

namespace solver::adaptor {
class GraphAdaptor {
public:
    using VertexT = uint32_t;
    using OffsetT = uint64_t;

    explicit GraphAdaptor(std::filesystem::path &path)
        : mStream(path, std::ios::binary)
    {
        char version[8];
        mStream.read(version, sizeof(version));

        if (std::memcmp(MAGIC_VER_1_0, version, sizeof(version))) {
            throw std::runtime_error("Provided trace have unknown format.");
        }

        Read(mNumVertices, mNumEdges);

        mVertexSectionStart = mStream.tellg();
        mMetadataSectionStart = mVertexSectionStart + mNumVertices * sizeof(OffsetT);
    }

    std::vector<VertexT> const& GetNeighhbours(VertexT vertex);

private:
    template <typename... T>
    void Read(T &... value)
    {
        (mStream.read(reinterpret_cast<char *>(&value), sizeof(value)), ...);
    }

    OffsetT ReadMetadataOffset(VertexT vertex);
    std::vector<VertexT> const& ReadMetadata(OffsetT offset);

    static const char *MAGIC_VER_1_0;
    
    std::ifstream mStream;

    VertexT mNumVertices;
    uint64_t mNumEdges;

    OffsetT mVertexSectionStart;
    OffsetT mMetadataSectionStart;
};
} // namespace solver::adaptor