#pragma once

#include "enums/_facing4.hpp"

namespace je2be {

class Door {
  Door() = delete;

public:
  static std::u8string JavaFacingFromBedrockCardinalDirection(std::u8string_view const &d) {
    auto facingB = Facing4FromJavaName(d);
    auto facingJ = Facing4ByRotatingLeft(facingB);
    return JavaNameFromFacing4(facingJ);
  }

  static std::u8string BedrockCardinalDirectionFromJavaFacing(std::u8string_view const &f) {
    auto facingJ = Facing4FromJavaName(f);
    auto facingB = Facing4ByRotatingRight(facingJ);
    return JavaNameFromFacing4(facingB);
  }
};

} // namespace je2be
