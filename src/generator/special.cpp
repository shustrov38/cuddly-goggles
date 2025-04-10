#include "special.h"
#include "bytes_conversion.h"

#include <iostream>
#include <tqdm.hpp>

namespace generator {
const char *HugeGraphGenerator::MAGIC_VER_1_0 = "DSHUV1.0";

void HugeGraphGenerator::Generate()
{
    mStream.write(MAGIC_VER_1_0, MAGIC_SIZE);
    std::cerr << "Graph format : " << MAGIC_VER_1_0 << std::endl;
    
    uint32_t const numVertices = mWidth * mHeight;

    // assumption: 
    //      (w-2)*(h-2) inside  - 8 edges
    //      4           corners - 3 edges
    //      2*(w+h-4)   border  - 5 edges
    uint64_t const numEdges = 8 * static_cast<uint64_t>(mWidth - 2) * static_cast<uint64_t>(mHeight - 2)
                            + 3 * 4
                            + 5 * 2 * static_cast<uint64_t>(mWidth + mHeight - 4);
    
    Write(numVertices, numEdges);
    std::cerr << "Num vertices : " << numVertices << std::endl;
    std::cerr << "Num edges    : " << numEdges << std::endl;
    
    uint64_t const resultSizeInBytes = mMetadataSectionStart
                                     + (sizeof(uint8_t) + 8 * sizeof(uint32_t)) * static_cast<uint64_t>(mWidth - 2) * static_cast<uint64_t>(mHeight - 2)
                                     + (sizeof(uint8_t) + 3 * sizeof(uint32_t)) * 4
                                     + (sizeof(uint8_t) + 5 * sizeof(uint32_t)) * 2 * static_cast<uint64_t>(mWidth + mHeight - 4);
    std::cerr << "Result size  : " << utils::BytesToHumanReadable(resultSizeInBytes) << std::endl; 

    uint64_t const zero = 0;
    for (uint32_t i = 0; i < numVertices; ++i) {
        Write(zero);
    }

    auto tqdm = tq::trange(mWidth);

    WriteVertexMetadataOffset(GetV(0, 0));
    Write(static_cast<uint8_t>(3),
        GetV(0, 1),
        GetV(1, 0),
        GetV(1, 1)
    );

    for (uint32_t j = 1; j < mHeight - 1; ++j) {
        WriteVertexMetadataOffset(GetV(0, j));
        Write(static_cast<uint8_t>(5), 
            GetV(0, j - 1),
            GetV(1, j - 1),
            GetV(1, j),
            GetV(1, j + 1), 
            GetV(0, j + 1)
        );
    }

    WriteVertexMetadataOffset(GetV(0, mHeight - 1));
    Write(static_cast<uint8_t>(3),
        GetV(0, mHeight - 2),
        GetV(1, mHeight - 1),
        GetV(1, mHeight - 2)
    );
    
    tqdm.update();

    for (uint32_t i = 1; i < mWidth - 1; ++i) {

        WriteVertexMetadataOffset(GetV(i, 0));
        Write(static_cast<uint8_t>(5),
            GetV(i - 1, 0),
            GetV(i - 1, 1),
            GetV(i, 1),
            GetV(i + 1, 1),
            GetV(i + 1, 0)
        );

        for (uint32_t j = 1; j < mHeight - 1; ++j) {
            WriteVertexMetadataOffset(GetV(i, j));
            Write(static_cast<uint8_t>(8),
                GetV(i - 1, j),
                GetV(i - 1, j + 1),
                GetV(i, j + 1),
                GetV(i + 1, j + 1),
                GetV(i + 1, j),
                GetV(i + 1, j - 1),
                GetV(i, j - 1),
                GetV(i - 1, j - 1)
            );
        }

        WriteVertexMetadataOffset(GetV(i, mHeight - 1));
        Write(static_cast<uint8_t>(5),
            GetV(i - 1, mHeight - 1),
            GetV(i - 1, mHeight - 2),
            GetV(i, mHeight - 2),
            GetV(i + 1, mHeight - 2),
            GetV(i + 1, mHeight - 1)
        );
        tqdm.update();
    }


    WriteVertexMetadataOffset(GetV(mWidth - 1, 0));
    Write(static_cast<uint8_t>(3),
        GetV(mWidth - 2, 0),
        GetV(mWidth - 2, 1),
        GetV(mWidth - 1, 1)
    );

    for (uint32_t j = 1; j < mHeight - 1; ++j) {
        WriteVertexMetadataOffset(GetV(mWidth - 1, j));
        Write(static_cast<uint8_t>(5), 
            GetV(mWidth - 1, j - 1),
            GetV(mWidth - 2, j - 1),
            GetV(mWidth - 2, j),
            GetV(mWidth - 2, j + 1),
            GetV(mWidth - 1, j + 1)
        );
    }

    WriteVertexMetadataOffset(GetV(mWidth - 1, mHeight - 1));
    Write(static_cast<uint8_t>(3),
        GetV(mWidth - 2, mHeight - 1),
        GetV(mWidth - 2, mHeight - 2),
        GetV(mWidth - 1, mHeight - 2)
    );

    tqdm.update();
    std::cerr << '\n';
}
} // namespace generator