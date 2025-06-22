#pragma once

#include "Constants.h"
#include "core/GameEvent.h"

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

inline std::string bytesToBitString(const std::vector<std::byte>& message) {
    std::string result;
    int bitCount = 0;

    for (std::byte b : message) {
        uint8_t value = std::to_integer<uint8_t>(b);
        for (int i = 7; i >= 0; --i) { // big-endian: старшие биты сначала
            result += ((value >> i) & 1) ? '1' : '0';
            ++bitCount;
            if (bitCount % 32 == 0) {
                result += '|';
            }
        }
    }

    return result;
}

inline std::string stringChanges(const std::vector<CellChange>& changes) {
    std::stringstream ss;
    for (auto [x, y, alive]: changes) {
        ss << '{' << x << ',' << y << "}, ";
    }
    return ss.str();
}