#pragma once

#include <vector>

#include "utils/Constants.h"

namespace LifeGame {

/**
 * Оптимизированное битовое поле для хранения состояния игры "Жизнь"
 * Использует выровненную память и SIMD инструкции для максимальной производительности
 */
class BitField {
public:
    BitField();
    BitField(const BitField& other) = delete;
    BitField(BitField&& other) noexcept;
    BitField& operator=(const BitField& other) = delete;
    BitField& operator=(BitField&& other) noexcept;
    ~BitField();

    // Основные операции с клетками
    bool isAlive(uint32_t x, uint32_t y) const;
    void setAlive(uint32_t x, uint32_t y, bool alive);
    void toggleCell(uint32_t x, uint32_t y);

    std::vector<std::byte> getData() const;
    
    // Очистка поля
    void clear();
    
    // Подсчет живых клеток в области (для многопоточной симуляции)
    uint8_t countNeighbors(uint32_t x, uint32_t y) const;
    
    // Подсчёт общего количества живых клеток
    uint32_t getAliveCellCount() const;

private:
    uint8_t* data_;
    
    // Вспомогательные методы
    inline uint32_t getIndex(uint32_t x, uint32_t y) const {
        return y * FIELD_WIDTH + x;
    }
    
    inline uint32_t getByteIndex(uint32_t index) const {
        return index / 8;
    }
    
    inline uint8_t getBitMask(uint32_t index) const {
        return 1 << (index % 8);
    }
    
    void allocateAlignedMemory();
    void deallocateMemory();
};

}
