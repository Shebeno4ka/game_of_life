#pragma once

#include "Constants.h"

#include <cstddef>
#include <iostream>
#include <sstream>
#include <vector>

inline std::string stringField(const std::vector<std::byte>& v) {
    constexpr int width = LifeGame::FIELD_WIDTH;
    constexpr int height = LifeGame::FIELD_HEIGHT;
    constexpr int totalBits = width * height;

    std::stringstream ss;
    for (int i = 0; i < totalBits; ++i) {
        int byteIndex = i / 8;
        int bitIndex = i % 8; // little-endian: младший бит первый

        if (byteIndex >= v.size()) {
            ss << "?"; // не хватает данных
        } else {
            bool bit = (std::to_integer<uint8_t>(v[byteIndex]) >> bitIndex) & 1;
            ss << bit;
        }

        if ((i + 1) % width == 0)
            ss << '\n';
    }
    return ss.str();
}

inline void printField(const std::vector<std::byte>& v) {
    std::cout << stringField(v);
}