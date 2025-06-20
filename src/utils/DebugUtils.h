#pragma once

#include "Constants.h"

#include <cstddef>
#include <iostream>
#include <vector>

inline void printField(const std::vector<std::byte>& v) {
    constexpr int width = LifeGame::FIELD_WIDTH;
    constexpr int height = LifeGame::FIELD_HEIGHT;
    constexpr int totalBits = width * height;

    for (int i = 0; i < totalBits; ++i) {
        int byteIndex = i / 8;
        int bitIndex = 7 - (i % 8);  // big-endian: старший бит первый
        if (byteIndex >= v.size()) {
            std::cout << "?";  // не хватает данных
        } else {
            bool bit = (std::to_integer<uint8_t>(v[byteIndex]) >> bitIndex) & 1;
            std::cout << bit;
        }

        if ((i + 1) % width == 0)
            std::cout << '\n';
    }
}
