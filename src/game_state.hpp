#pragma once

#include <array>
#include <cstdint>
#include <vector>

// clang-format off
const std::array<std::pair<int8_t, int8_t>, 8> neighbours_offsets = {{
  {-1, -1}, {-1, 0}, {-1, 1},
  {0, -1},          {0, 1},
  {1, -1}, {1, 0}, {1, 1}
}};
// clang-format on

/**
 * \class GameState
 * \brief Represents the state of the game, including the game field and its
 * operations.
 */
class GameState {
 public:
  /**
   * \typedef field_t
   * \brief A 2D vector representing the game field, where each cell is a
   * boolean.
   */
  using field_t = std::vector<std::vector<bool>>;

 private:
  /// First game field buffer.
  field_t field1_;
  /// Second game field buffer.
  field_t field2_;
  /// Pointer to the active game field.
  field_t* first_field_ptr_;
  /// Pointer to the inactive game field.
  field_t* second_field_ptr_;

 public:
  /**
   * \brief Constructs a GameState with the specified dimensions.
   * \param height Positive number, height of the game field.
   * \param width Positive number, width of the game field.
   */
  GameState(uint32_t height, uint32_t width);

  /**
   * \brief Constructs a GameState using an existing game field.
   * \param field Pointer to an existing game field.
   */
  explicit GameState(field_t&& field);

  /**
   * \brief Updates the game state by one step.
   */
  void update();

  /**
   * \brief Toggles the state of a specific cell in the game field.
   * \param i The row index of the cell.
   * \param j The column index of the cell.
   */
  void toggleCell(uint32_t i, uint32_t j);

  /**
   * \brief Retrieves the current game field.
   * \return A constant reference to the active game field.
   */
  field_t& getField();

  /**
   * \brief Resets the game state to its initial configuration.
   */
  void reset();

 private:
  /**
   * \brief Performs the update operation on the game field.
   * \param field_to_read The field to read the current state from.
   * \param field_to_write The field to write the updated state to.
   */
  void doUpdate_(const field_t& field_to_read, field_t& field_to_write);

  /**
   * \brief Checks if the given coordinates are valid within the game field.
   * \param i The row index to check.
   * \param j The column index to check.
   * \return True if the coordinates are valid, false otherwise.
   */
  bool isValidCoord_(int32_t i, int32_t j);
};