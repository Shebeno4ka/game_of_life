#include "BitField.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace LifeGame {

BitField::BitField() : data_(nullptr) {
  allocateAlignedMemory();
  clear();
}

BitField::BitField(BitField&& other) noexcept : data_(other.data_) {
  other.data_ = nullptr;
}

BitField& BitField::operator=(BitField&& other) noexcept {
  if (this != &other) {
    deallocateMemory();
    data_ = other.data_;
    other.data_ = nullptr;
  }
  return *this;
}

BitField::~BitField() {
  deallocateMemory();
}

bool BitField::isAlive(uint32_t x, uint32_t y) const {
  if (x >= FIELD_WIDTH || y >= FIELD_HEIGHT) {
    return false;
  }

  uint32_t index = getIndex(x, y);
  uint32_t byteIndex = getByteIndex(index);
  uint8_t bitMask = getBitMask(index);

  return (data_[byteIndex] & bitMask) != 0;
}

void BitField::setAlive(uint32_t x, uint32_t y, bool alive) {
  if (x >= FIELD_WIDTH || y >= FIELD_HEIGHT) {
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
  if (x >= FIELD_WIDTH || y >= FIELD_HEIGHT) {
    return;
  }

  uint32_t index = getIndex(x, y);
  uint32_t byteIndex = getByteIndex(index);
  uint8_t bitMask = getBitMask(index);

  data_[byteIndex] ^= bitMask;
}

std::vector<std::byte> BitField::getData() const {
  std::vector<std::byte> data(FIELD_BYTES);
  std::transform(data_, data_ + FIELD_BYTES, data.begin(), [](char c) {
      return static_cast<std::byte>(c);
  });

  return data;
}

void BitField::clear() {
  if (data_) {
    std::memset(data_, 0, FIELD_BYTES);
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
      if (nx >= 0 && nx < static_cast<int>(FIELD_WIDTH) && ny >= 0 &&
          ny < static_cast<int>(FIELD_HEIGHT)) {
        if (isAlive(static_cast<uint32_t>(nx), static_cast<uint32_t>(ny))) {
          count++;
        }
      }
    }
  }

  return count;
}

void BitField::allocateAlignedMemory() {
  // Выравнивание памяти для SIMD операций (32 байта для AVX2)
  data_ = static_cast<uint8_t*>(std::aligned_alloc(32, FIELD_BYTES));
  if (!data_) {
    throw std::bad_alloc();
  }
}

void BitField::deallocateMemory() {
  if (data_) {
    std::free(data_);
    data_ = nullptr;
  }
}

uint32_t BitField::getAliveCellCount() const {
  uint32_t count = 0;

  for (size_t i = 0; i < FIELD_BYTES; ++i) {
    uint8_t byte = data_[i];
    for (int b = 0; b < 8; ++b) {
      count += (byte >> b) & 1;
    }
  }

  return count;
}

}  // namespace LifeGame
