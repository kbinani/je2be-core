#pragma once

#include <je2be/pos2.hpp>

namespace je2be {

enum class Facing4 {
  North,
  East,
  South,
  West,
};

static inline std::string JavaNameFromFacing4(Facing4 f) {
  switch (f) {
  case Facing4::North:
    return "north";
  case Facing4::East:
    return "east";
  case Facing4::South:
    return "south";
  case Facing4::West:
    return "west";
  }
  assert(false);
  return "";
}

static inline Facing4 Facing4FromJavaName(std::string_view const &n) {
  if (n == "north") {
    return Facing4::North;
  } else if (n == "east") {
    return Facing4::East;
  } else if (n == "west") {
    return Facing4::West;
  }
  return Facing4::South;
}

static inline int BedrockDirectionFromFacing4(Facing4 f) {
  switch (f) {
  case Facing4::North:
    return 2;
  case Facing4::East:
    return 3;
  case Facing4::West:
    return 1;
  case Facing4::South:
    return 0;
  }
  assert(false);
  return 0;
}

static inline Facing4 Facing4FromBedrockDirection(int d) {
  switch (d) {
  case 2:
    return Facing4::North;
  case 3:
    return Facing4::East;
  case 1:
    return Facing4::West;
  case 0:
  default:
    return Facing4::South;
  }
}

static inline float YawFromFacing4(Facing4 f4) {
  switch (f4) {
  case Facing4::North:
    return -180;
  case Facing4::East:
    return -90;
  case Facing4::West:
    return 90;
  case Facing4::South:
  default:
    return 0;
  }
}

static inline Pos2i Pos2iFromFacing4(Facing4 f4) {
  switch (f4) {
  case Facing4::North:
    return Pos2i(0, -1);
  case Facing4::East:
    return Pos2i(1, 0);
  case Facing4::West:
    return Pos2i(-1, 0);
  case Facing4::South:
  default:
    return Pos2i(0, 1);
  }
}

static inline std::optional<Facing4> Facing4FromPos2i(Pos2i const &p) {
  if (p.fX == 0 && p.fZ == 0) {
    return std::nullopt;
  }
  if (p.fX != 0 && p.fZ != 0) {
    return std::nullopt;
  }
  if (p.fX > 0) {
    return Facing4::East;
  } else if (p.fX < 0) {
    return Facing4::West;
  } else if (p.fZ > 0) {
    return Facing4::South;
  } else {
    return Facing4::North;
  }
}

} // namespace je2be
