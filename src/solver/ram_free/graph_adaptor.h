#pragma once

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>

namespace solver::ramfree {
class GraphAdaptor {
public:
    using VertexT = uint32_t;
    using OffsetT = uint64_t;

    explicit GraphAdaptor(std::filesystem::path &path)
        : mPath(path)
        , mStream(mPath, std::ios::binary)
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

    GraphAdaptor(GraphAdaptor const& other)
        : mPath(other.mPath)
    {
        mStream.open(mPath, std::ios::binary);
        
        mNumVertices = other.mNumVertices;
        mNumEdges = other.mNumEdges;

        mVertexSectionStart = other.mVertexSectionStart;
        mMetadataSectionStart = other.mMetadataSectionStart;
    }

    inline VertexT GetNumVertices() const noexcept
    {
        return mNumVertices;
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
    
    std::filesystem::path const mPath;
    std::ifstream mStream;

    VertexT mNumVertices;
    uint64_t mNumEdges;

    OffsetT mVertexSectionStart;
    OffsetT mMetadataSectionStart;
};
} // namespace solver::ramfree