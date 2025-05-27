#pragma once

#include <stdexcept>
#include <string>

class invalid_argument_exception : public std::runtime_error {
 public:
  explicit invalid_argument_exception(const std::string& message)
      : std::runtime_error("Invalid argument: " + message) {}
};
