#pragma once

#include <optional>
#include <filesystem>

namespace generator {
struct Parameters {
    bool isDefaultMode { false };

    // params for default mode
    
    size_t numVertices { 0 };
    
    bool connectivity { false };
    double removeProbability { 0.5 };

    std::optional<std::filesystem::path> svgPath { std::nullopt };

    // params for huge graph mode

    std::filesystem::path resultPath;
    
    uint32_t width;
    uint32_t height; 
};
} // namespace generator