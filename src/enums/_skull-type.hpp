#pragma once

namespace je2be {

enum class SkullType : u8 {
  Skeleton = 0,
  WitherSkeleton = 1,
  Zombie = 2,
  Player = 3,
  Creeper = 4,
  Dragon = 5,
  Piglin = 6,

  SkullTypeLast,
};

static inline std::u8string JavaNameFromSkullType(SkullType st) {
  switch (st) {
  case SkullType::Player:
    return u8"player_head";
  case SkullType::Zombie:
    return u8"zombie_head";
  case SkullType::Creeper:
    return u8"creeper_head";
  case SkullType::Dragon:
    return u8"dragon_head";
  case SkullType::Skeleton:
    return u8"skeleton_skull";
  case SkullType::WitherSkeleton:
    return u8"wither_skeleton_skull";
  case SkullType::Piglin:
    return u8"piglin_head";
  default:
    assert(false);
    return u8"";
  }
}

static inline std::optional<SkullType> SkullTypeFromJavaName(std::u8string const &name) {
  for (u8 i = 0; i < static_cast<u8>(SkullType::SkullTypeLast); i++) {
    SkullType st = static_cast<SkullType>(i);
    std::u8string n = JavaNameFromSkullType(st);
    if (n == name) {
      return st;
    }
  }
  return std::nullopt;
}

} // namespace je2be
