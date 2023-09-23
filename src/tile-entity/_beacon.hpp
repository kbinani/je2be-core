#pragma once

#include <je2be/integers.hpp>

#include "_static-reversible-map.hpp"

namespace je2be {

class Beacon : StaticReversibleMap<i32, std::u8string, Beacon> {
public:
  static std::optional<std::u8string> JavaEffectFromLegacyJavaAndBedrock(i32 id) {
    return Forward(id);
  }

  static i32 BedrockEffectFromJava(std::u8string const &id) {
    return Backward(id, -1);
  }

  static ReversibleMap<i32, std::u8string> *CreateTable() {
    return new ReversibleMap<i32, std::u8string>({
        {1, u8"minecraft:speed"},
        {3, u8"minecraft:haste"},
        {5, u8"minecraft:strength"},
        {8, u8"minecraft:jump_strength"},
        {11, u8"minecraft:resistance"},
    });
  }
};

} // namespace je2be
