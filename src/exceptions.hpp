#pragma once

#include <stdexcept>
#include <string>

class InvalidArgumentException : public std::runtime_error {
 public:
  explicit InvalidArgumentException(const std::string& message)
      : std::runtime_error("Invalid argument: " + message) {}
};
