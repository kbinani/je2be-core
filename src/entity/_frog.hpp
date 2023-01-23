#pragma once

namespace je2be {

class Frog : StaticReversibleMap<std::string, i32, Frog> {
  Frog() = delete;

public:
  static i32 BedrockVariantFromJavaVariant(std::string const &variant) {
    return Forward(variant, 0);
  }

  static std::string JavaVariantFromBedrockVariant(i32 variant) {
    return Backward(variant, "minecraft:temperate");
  }

  static ReversibleMap<std::string, i32> const *CreateTable() {
    return new ReversibleMap<std::string, i32>({
        {"minecraft:temperate", 0}, // brown
        {"minecraft:cold", 1},      // green
        {"minecraft:warm", 2},      // white
    });
  }

  static std::string BedrockDefinitionFromJavaVariant(std::string const &variant) {
    std::string v = variant;
    if (variant.starts_with("minecraft:")) {
      v = variant.substr(10);
    }
    return "+" + v + "_frog";
  }
};

} // namespace je2be
