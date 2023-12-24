#pragma once

#include "_pos3.hpp"

namespace je2be {

enum class Facing6 {
  North,
  East,
  South,
  West,
  Up,
  Down,
};

static inline std::u8string JavaNameFromFacing6(Facing6 f) {
  switch (f) {
  case Facing6::North:
    return u8"north";
  case Facing6::East:
    return u8"east";
  case Facing6::South:
    return u8"south";
  case Facing6::West:
    return u8"west";
  case Facing6::Up:
    return u8"up";
  case Facing6::Down:
    return u8"down";
  }
  assert(false);
  return u8"";
}

static inline Facing6 Facing6FromJavaName(std::u8string_view const &n) {
  if (n == u8"north") {
    return Facing6::North;
  } else if (n == u8"east") {
    return Facing6::East;
  } else if (n == u8"south") {
    return Facing6::South;
  } else if (n == u8"west") {
    return Facing6::West;
  } else if (n == u8"up") {
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

static inline Facing6 Facing6FromBedrockCardinalDirection(std::u8string const &cardinalDirection) {
  return Facing6FromJavaName(cardinalDirection);
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

static inline i32 BedrockFacingDirectionBFromFacing6(Facing6 f6) {
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

static inline Facing6 Facing6Invert(Facing6 f) {
  switch (f) {
  case Facing6::Up:
    return Facing6::Down;
  case Facing6::North:
    return Facing6::South;
  case Facing6::East:
    return Facing6::West;
  case Facing6::South:
    return Facing6::North;
  case Facing6::West:
    return Facing6::East;
  case Facing6::Down:
  default:
    return Facing6::Up;
  }
}

static inline void Facing6Enumerate(std::function<void(Facing6)> action) {
  action(Facing6::North);
  action(Facing6::East);
  action(Facing6::South);
  action(Facing6::West);
  action(Facing6::Up);
  action(Facing6::Down);
}

} // namespace je2be
