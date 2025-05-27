#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "invalid_argument_exception.hpp"

constexpr char kHelpText[] =
    "game_of_life - Game of Life cellular automaton simulation\n\n"
    "Usage: game_of_life SIZE_X SIZE_Y [SPEED]\n\n"
    "Game of Life simulates cells that live or die based on their "
    "neighbors.\n\n"
    "Parameters:\n"
    "  SIZE_X              integer, Width of the game grid\n"
    "  SIZE_Y              integer, Height of the game grid\n"
    "  SPEED               integer, Game simulation speed in updates/sec "
    "(optional, default: 1)\n\n"
    "Options:\n"
    "  -h, --help          Display this help and exit\n\n"
    "Examples:\n"
    "  game_of_life 50 40      Run with a 50x40 grid at default speed\n"
    "  game_of_life 80 60 3    Run with an 80x60 grid at speed 3\n";

constexpr int kDefaultGameSpeed = 8;

struct Options {
  int size_x;
  int size_y;
  int updates_per_second;
};

/**
 * @brief Parses command line arguments for the Game of Life simulation
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return std::optional<Options> Parsed options if successful, std::nullopt if
 * help is requested
 * @throws invalid_argument_exception If arguments are invalid, missing, or in
 * wrong format
 */
static std::optional<Options> ParseCommandLineOptions(int argc, char* argv[]) {
  Options opts{};

  if (argc == 2 &&
      (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
    return std::nullopt;
  }

  if (argc < 3) {
    throw invalid_argument_exception("Not enough arguments");
  }

  try {
    opts.size_x = std::stoi(argv[1]);
    opts.size_y = std::stoi(argv[2]);
    opts.updates_per_second = argc > 3
                                  ? std::stoi(argv[3])
                                  : kDefaultGameSpeed;
    if (opts.size_x <= 0 || opts.size_y <= 0 ||
        opts.updates_per_second <= 0) {
      throw invalid_argument_exception("Arguments must be positive integers");
    }
  } catch (const std::invalid_argument& e) {
    throw invalid_argument_exception("Arguments must be integers");
  } catch (const std::out_of_range& e) {
    throw invalid_argument_exception("Arguments too large");
  }

  return opts;
}