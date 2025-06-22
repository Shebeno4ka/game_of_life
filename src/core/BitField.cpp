#include "BitField.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace LifeGame {

BitField::BitField(uint32_t width, uint32_t height) : width_(width), height_(height), data_(nullptr) {
    allocateAlignedMemory();
    clear();
}

BitField::BitField(const BitField& other) : width_(other.width_), height_(other.height_), data_(nullptr) {
    allocateAlignedMemory();
    std::memcpy(data_, other.data_, fieldBytes());
}

BitField::BitField(BitField&& other) noexcept : width_(other.width_), height_(other.height_), data_(other.data_) {
    other.width_ = 0;
    other.height_ = 0;
    other.data_ = nullptr;
}

BitField& BitField::operator=(const BitField& other) {
    if (this != &other) {
        if (width_ != other.width_ || height_ != other.height_) {
            deallocateMemory();
            width_ = other.width_;
            height_ = other.height_;
            allocateAlignedMemory();
        }

        std::memcpy(data_, other.data_, fieldBytes());
    }
    return *this;
}

BitField& BitField::operator=(BitField&& other) noexcept {
    if (this != &other) {
        deallocateMemory();
        data_ = other.data_;
        width_ = other.width_;
        height_ = other.height_;
        other.data_ = nullptr;
        other.width_ = 0;
        other.height_ = 0;
    }
    return *this;
}

BitField::~BitField() {
    deallocateMemory();
}

bool BitField::isAlive(uint32_t x, uint32_t y) const {
    if (x >= width_ || y >= height_) {
        return false;
    }

    uint32_t index = getIndex(x, y);
    uint32_t byteIndex = getByteIndex(index);
    uint8_t bitMask = getBitMask(index);

    return (data_[byteIndex] & bitMask) != 0;
}

void BitField::setAlive(uint32_t x, uint32_t y, bool alive) {
    if (x >= width_ || y >= height_) {
        return;
    }

    uint32_t index = getIndex(x, y);
    uint32_t byteIndex = getByteIndex(index);
    uint8_t bitMask = getBitMask(index);

    if (alive) {
        data_[byteIndex] |= bitMask;
    } else {
        data_[byteIndex] &= ~bitMask;
    }
}

void BitField::toggleCell(uint32_t x, uint32_t y) {
    if (x >= width_ || y >= height_) {
        return;
    }

    uint32_t index = getIndex(x, y);
    uint32_t byteIndex = getByteIndex(index);
    uint8_t bitMask = getBitMask(index);

    data_[byteIndex] ^= bitMask;
}

std::vector<std::byte> BitField::serialize() const {
    std::vector<std::byte> data(fieldBytes());
    std::transform(data_, data_ + fieldBytes(), data.begin(), [](char c) { return static_cast<std::byte>(c); });

    return data;
}

void BitField::clear() {
    if (data_) {
        std::memset(data_, 0, fieldBytes());
    }
}

uint8_t BitField::countNeighbors(uint32_t x, uint32_t y) const {
    uint8_t count = 0;

    // Проверяем 8 соседних клеток
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0)
                continue;

            int nx = static_cast<int>(x) + dx;
            int ny = static_cast<int>(y) + dy;

            // Проверяем границы
            if (nx >= 0 && nx < static_cast<int>(width_) && ny >= 0 && ny < static_cast<int>(height_)) {
                if (isAlive(static_cast<uint32_t>(nx), static_cast<uint32_t>(ny))) {
                    count++;
                }
            }
        }
    }

    return count;
}

void BitField::allocateAlignedMemory() {
    data_ = static_cast<uint8_t*>(::operator new(fieldBytes(), std::align_val_t{kCacheLineSize}));
    if (!data_) {
        throw std::bad_alloc();
    }
}

void BitField::deallocateMemory() {
    if (data_) {
        ::operator delete(data_, std::align_val_t{kCacheLineSize});
        data_ = nullptr;
    }
}

uint32_t BitField::getAliveCellCount() const {
    uint32_t count = 0;

    for (size_t i = 0; i < fieldBytes(); ++i) {
        uint8_t byte = data_[i];
        for (int b = 0; b < 8; ++b) {
            count += (byte >> b) & 1;
        }
    }

    return count;
}

} // namespace LifeGame
