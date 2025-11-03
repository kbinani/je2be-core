#pragma once

#include <je2be/pos2.hpp>

namespace je2be {

enum class Facing4 {
  North,
  East,
  South,
  West,
};

static inline std::u8string JavaNameFromFacing4(Facing4 f) {
  switch (f) {
  case Facing4::North:
    return u8"north";
  case Facing4::East:
    return u8"east";
  case Facing4::South:
    return u8"south";
  case Facing4::West:
    return u8"west";
  }
  assert(false);
  return u8"";
}

static inline Facing4 Facing4FromJavaName(std::u8string_view const &n) {
  if (n == u8"north") {
    return Facing4::North;
  } else if (n == u8"east") {
    return Facing4::East;
  } else if (n == u8"west") {
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

static inline int North0East1South2West3FromFacing4(Facing4 f) {
  switch (f) {
  case Facing4::North:
    return 0;
  case Facing4::East:
    return 1;
  case Facing4::South:
    return 2;
  case Facing4::West:
    return 3;
  }
  assert(false);
  return 0;
}

static inline Facing4 Facing4FromNorth0East1South2West3(int v) {
  switch (v) {
  case 0:
    return Facing4::North;
  case 1:
    return Facing4::East;
  case 2:
    return Facing4::South;
  case 3:
    return Facing4::West;
  }
  return Facing4::North;
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

static inline Facing4 Facing4FromNorth2East3South0West1(int d) {
  return Facing4FromBedrockDirection(d);
}

static inline int North2East3South0West1FromFacing4(Facing4 f) {
  return BedrockDirectionFromFacing4(f);
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

static inline Facing4 Facing4ByRotatingLeft(Facing4 f) {
  switch (f) {
  case Facing4::North:
    return Facing4::West;
  case Facing4::East:
    return Facing4::North;
  case Facing4::South:
    return Facing4::East;
  case Facing4::West:
    return Facing4::South;
  }
}

static inline Facing4 Facing4ByRotatingRight(Facing4 f) {
  switch (f) {
  case Facing4::North:
    return Facing4::East;
  case Facing4::East:
    return Facing4::South;
  case Facing4::South:
    return Facing4::West;
  case Facing4::West:
    return Facing4::North;
  }
}

} // namespace je2be
