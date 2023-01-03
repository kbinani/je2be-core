#pragma once

#include <je2be/_pos3.hpp>

namespace je2be {

enum class Facing6 {
  North,
  East,
  South,
  West,
  Up,
  Down,
};

static inline std::string JavaNameFromFacing6(Facing6 f) {
  switch (f) {
  case Facing6::North:
    return "north";
  case Facing6::East:
    return "east";
  case Facing6::South:
    return "south";
  case Facing6::West:
    return "west";
  case Facing6::Up:
    return "up";
  case Facing6::Down:
    return "down";
  }
  assert(false);
  return "";
}

static inline Facing6 Facing6FromJavaName(std::string_view const &n) {
  if (n == "north") {
    return Facing6::North;
  } else if (n == "east") {
    return Facing6::East;
  } else if (n == "south") {
    return Facing6::South;
  } else if (n == "west") {
    return Facing6::West;
  } else if (n == "up") {
    return Facing6::Up;
  }
  return Facing6::Down;
}

static inline Pos3i Pos3iFromFacing6(Facing6 f6) {
  switch (f6) {
  case Facing6::East:
    return Pos3i(1, 0, 0);
  case Facing6::South:
    return Pos3i(0, 0, 1);
  case Facing6::West:
    return Pos3i(-1, 0, 0);
  case Facing6::North:
    return Pos3i(0, 0, -1);
  case Facing6::Up:
    return Pos3i(0, 1, 0);
  case Facing6::Down:
  default:
    return Pos3i(0, -1, 0);
  }
}

static inline int BedrockFacingDirectionAFromFacing6(Facing6 f) {
  switch (f) {
  case Facing6::North:
    return 2;
  case Facing6::East:
    return 5;
  case Facing6::South:
    return 3;
  case Facing6::West:
    return 4;
  case Facing6::Up:
    return 1;
  case Facing6::Down:
    return 0;
  }
  assert(false);
  return 0;
}

static inline Facing6 Facing6FromBedrockFacingDirectionA(int facingDirectionA) {
  switch (facingDirectionA) {
  case 1:
    return Facing6::Up;
  case 2:
    return Facing6::North;
  case 3:
    return Facing6::South;
  case 4:
    return Facing6::West;
  case 5:
    return Facing6::East;
  case 0:
  default:
    return Facing6::Down;
  }
}

static inline Facing6 Facing6FromBedrockFacingDirectionB(int direction) {
  switch (direction) {
  case 4:
    return Facing6::East;
  case 2:
    return Facing6::South;
  case 5:
    return Facing6::West;
  case 3:
    return Facing6::North;
  case 1:
    return Facing6::Up;
  case 0:
  default:
    return Facing6::Down;
  }
}

static inline int32_t BedrockFacingDirectionBFromFacing6(Facing6 f6) {
  switch (f6) {
  case Facing6::East:
    return 4;
  case Facing6::South:
    return 2;
  case Facing6::West:
    return 5;
  case Facing6::North:
    return 3;
  case Facing6::Up:
    return 1;
  case Facing6::Down:
  default:
    return 0;
  }
}

} // namespace je2be
