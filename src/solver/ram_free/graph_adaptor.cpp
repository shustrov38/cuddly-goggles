#include "graph_adaptor.h"

#include "debug.h"

namespace solver::ramfree {
const char *GraphAdaptor::MAGIC_VER_1_0 = "DSHUV1.0";

GraphAdaptor::OffsetT GraphAdaptor::ReadMetadataOffset(VertexT vertex)
{
    mStream.seekg(mVertexSectionStart + vertex * sizeof(OffsetT));
    uint64_t offset;
    Read(offset);
    return offset;
}

std::vector<GraphAdaptor::VertexT> const& GraphAdaptor::ReadMetadata(OffsetT offset)
{
    mStream.seekg(offset);
    uint8_t size;
    Read(size);

    thread_local std::vector<uint32_t> result(8);
    result.resize(size);
    mStream.read(reinterpret_cast<char *>(result.data()), size * sizeof(uint32_t));
    return result;
}

std::vector<GraphAdaptor::VertexT> const& GraphAdaptor::GetNeighhbours(VertexT vertex)
{
    OffsetT offset = ReadMetadataOffset(vertex);
    auto &result = ReadMetadata(offset);
    return result;
}

} // namespace solver::ramfree