#pragma once

#include <type_traits>
#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <limits>
#include <ios>

#include <bytes_conversion.h>

namespace generator {
class HugeGraphGenerator {
public:
    explicit HugeGraphGenerator(std::filesystem::path const& path, uint32_t width, uint32_t height)
        : mStream(path, std::ios::binary)
        , mWidth(width)
        , mHeight(height)
        , mVertexSectionStart(8 * sizeof(char) + sizeof(uint32_t) + sizeof(uint64_t))
        , mMetadataSectionStart(mVertexSectionStart + mHeight * mHeight * sizeof(uint64_t))
    {
        if (std::numeric_limits<uint32_t>::max() / mWidth < mHeight) {
            throw std::invalid_argument("uint32_t overflow in (width x height)");
        }
    }

    void Generate();

private:
    template <typename... T>
    void Write(T const&... value)
    {
        (mStream.write(reinterpret_cast<const char *>(&value), sizeof(std::decay_t<T>)), ...);
    }

    void WriteVertexMetadataOffset(uint32_t v)
    {
        uint64_t const pos = mStream.tellp();
        mStream.seekp(mVertexSectionStart + v * sizeof(uint64_t), std::ios_base::beg);
        Write(pos);
        mStream.seekp(pos, std::ios::beg);
    }

    constexpr uint32_t GetV(uint32_t x, uint32_t y) const
    {
        return x * mWidth + y;
    }

    constexpr uint32_t GetX(uint32_t v) const
    {
        return v / mWidth;
    }

    constexpr uint32_t GetY(uint32_t v) const
    {
        return v / mHeight;
    }

    static size_t constexpr MAGIC_SIZE = 8;
    static const char *MAGIC_VER_1_0;

    std::ofstream mStream;
    
    uint32_t const mWidth;
    uint32_t const mHeight;

    uint64_t const mVertexSectionStart;
    uint64_t const mMetadataSectionStart;
};
} // namespace generator