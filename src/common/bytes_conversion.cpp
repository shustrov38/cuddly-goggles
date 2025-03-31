#include "bytes_conversion.h"

#include <cstring>
#include <cmath>
#include <array>
#include <charconv>

namespace utils {
std::string BytesToHumanReadable(uint64_t bytes) {
    static std::array<const char*, 7> constexpr units = {"B", "KB", "MB", "GB", "TB", "PB", "EB"};
    size_t unitIndex = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024 && unitIndex < units.size() - 1) {
        size /= 1024;
        unitIndex++;
    }

    char buffer[32];

    if (unitIndex == 0) {
        auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), bytes);
        *ptr++ = ' ';
        std::memcpy(ptr, units[unitIndex], 2);
    } else if (std::floor(size) == size) {
        auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), static_cast<int>(size));
        *ptr++ = ' ';
        std::memcpy(ptr, units[unitIndex], 3);
    } else {
        auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), size, std::chars_format::fixed, 2);
        *ptr++ = ' ';
        std::memcpy(ptr, units[unitIndex], 3);
    }

    return buffer;
}
} // namespace utils