#include "graph_adaptor.h"

#include "debug.h"

namespace solver::adaptor {
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

    static std::vector<uint32_t> result(8);
    result.resize(size);
    mStream.read(reinterpret_cast<char *>(result.data()), size * sizeof(uint32_t));
    return result;
}

std::vector<GraphAdaptor::VertexT> const& GraphAdaptor::GetNeighhbours(VertexT vertex)
{
    _DEBUG(vertex);
    OffsetT offset = ReadMetadataOffset(vertex);
    _DEBUG_HEX(offset);
    auto &result = ReadMetadata(offset);
    _DEBUG(result.size());
    _DEBUG(result);
    return result;
}

} // namespace solver::adaptor