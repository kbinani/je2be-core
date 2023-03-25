#pragma once

#include "_namespace.hpp"

namespace je2be {

class Frog : StaticReversibleMap<std::u8string, i32, Frog> {
  Frog() = delete;

public:
  static i32 BedrockVariantFromJavaVariant(std::u8string const &variant) {
    return Forward(variant, 0);
  }

  static std::u8string JavaVariantFromBedrockVariant(i32 variant) {
    return Backward(variant, u8"minecraft:temperate");
  }

  static ReversibleMap<std::u8string, i32> const *CreateTable() {
    return new ReversibleMap<std::u8string, i32>({
        {u8"minecraft:temperate", 0}, // brown
        {u8"minecraft:cold", 1},      // green
        {u8"minecraft:warm", 2},      // white
    });
  }

  static std::u8string BedrockDefinitionFromJavaVariant(std::u8string const &variant) {
    std::u8string v = Namespace::Remove(variant);
    return u8"+" + v + u8"_frog";
  }
};

} // namespace je2be
