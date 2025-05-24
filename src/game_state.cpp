#include "game_state.hpp"

GameState::GameState(uint32_t height, uint32_t width)
    : field1_(height, std::vector<bool>(width, 0)),
      field2_(field1_),
      first_field_ptr_(&field1_),
      second_field_ptr_(&field2_) {}

GameState::GameState(field_t* field)
    : field1_(*field),
      field2_(field1_),
      first_field_ptr_(&field1_),
      second_field_ptr_(&field2_) {}

void GameState::update() {
  doUpdate_(*first_field_ptr_, *second_field_ptr_);
  std::swap(first_field_ptr_, second_field_ptr_);
}

void GameState::toggleCell(uint32_t i, uint32_t j) {
  (*first_field_ptr_)[j][i] = !(*first_field_ptr_)[j][i];
}

const GameState::field_t& GameState::getField() const {
  return *first_field_ptr_;
}

void GameState::reset() {
  for (auto& row : field1_) {
    std::fill(row.begin(), row.end(), false);
  }
  for (auto& row : field2_) {
    std::fill(row.begin(), row.end(), false);
  }
  first_field_ptr_ = &field1_;
  second_field_ptr_ = &field2_;
}

void GameState::doUpdate_(const field_t& field_to_read,
                          field_t& field_to_write) {
  for (uint32_t i = 0; i < field_to_read.size(); ++i) {
    for (uint32_t j = 0; j < field_to_read[i].size(); ++j) {
      uint32_t alive_neighbours = 0;
      for (const auto& offset : neighbours_offsets) {
        int32_t ni = i + offset.first;
        int32_t nj = j + offset.second;
        if (isValidCoord_(ni, nj)) {
          alive_neighbours += field_to_read[ni][nj];
        }
      }
      if (field_to_read[i][j]) {
        field_to_write[i][j] = alive_neighbours == 2 || alive_neighbours == 3;
      } else {
        field_to_write[i][j] = alive_neighbours == 3;
      }
    }
  }
}

bool GameState::isValidCoord_(int32_t i, int32_t j) {
  return i >= 0 && i < field1_.size() && j >= 0 && j < field2_[0].size();
}