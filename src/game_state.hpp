#pragma once

#include <cstdint>
#include <vector>

// clang-format off
const std::vector<std::pair<uint32_t, uint32_t>> neighbours_offsets = {
    {-1, -1}, {-1, 0}, {-1, 1},
    {0, -1},          {0, 1},
    {1, -1}, {1, 0}, {1, 1}
};
// clang-format on

class GameState {
 public:
  using field_t = std::vector<std::vector<bool>>;

 private:
  field_t field1_;
  field_t field2_;
  field_t* first_field_ptr_;
  field_t* second_field_ptr_;

 public:
  GameState(uint32_t height, uint32_t width);
  GameState(field_t* field);

  void update();
  void toggleCell(uint32_t i, uint32_t j);
  const field_t& getField() const;
  void reset();

 private:
  void doUpdate_(const field_t& field_to_read, field_t& field_to_write);
  bool isValidCoord_(uint32_t i, uint32_t j);
};