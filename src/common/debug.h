#pragma once

#include <ostream>
#include <vector>

#ifdef DEBUG
    template<typename T>
    std::ostream &operator <<(std::ostream &out, std::vector<T> const& v)
    {
        char sep = ' ';
        out << '{';
        for (auto& e: v) {
            out << sep << e; sep = ',';
        }
        out << " }";
        return out;
    }

    #define _DEBUG(x) do { std::cerr << #x << " = " << (x) << std::endl; } while(0)
    #define _DEBUG_HEX(x) do { std::cerr << #x << " = " << std::hex << (x)  << std::dec << std::endl; } while(0)
#else
    #define _DEBUG(x) {}
    #define _DEBUG_HEX(x) {}
#endif