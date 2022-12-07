#pragma once

namespace je2be {

enum class SkullType : uint8_t {
  Skeleton = 0,
  WitherSkeleton = 1,
  Zombie = 2,
  Player = 3,
  Creeper = 4,
  Dragon = 5,
  Piglin = 6,

  SkullTypeLast,
};

static inline std::string JavaNameFromSkullType(SkullType st) {
  switch (st) {
  case SkullType::Player:
    return "player_head";
  case SkullType::Zombie:
    return "zombie_head";
  case SkullType::Creeper:
    return "creeper_head";
  case SkullType::Dragon:
    return "dragon_head";
  case SkullType::Skeleton:
    return "skeleton_skull";
  case SkullType::WitherSkeleton:
    return "wither_skeleton_skull";
  case SkullType::Piglin:
    return "piglin_head";
  default:
    assert(false);
    return "";
  }
}

static inline std::optional<SkullType> SkullTypeFromJavaName(std::string const &name) {
  for (uint8_t i = 0; i < static_cast<uint8_t>(SkullType::SkullTypeLast); i++) {
    SkullType st = static_cast<SkullType>(i);
    std::string n = JavaNameFromSkullType(st);
    if (n == name) {
      return st;
    }
  }
  return std::nullopt;
}

} // namespace je2be
