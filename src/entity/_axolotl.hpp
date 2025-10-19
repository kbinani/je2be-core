#pragma once

#include "_static-reversible-map.hpp"

namespace je2be {

class Axolotl : StaticReversibleMap<i32, i32, Axolotl> {
  Axolotl() = delete;

public:
  static ReversibleMap<i32, i32> const *CreateTable() {
    return new ReversibleMap<i32, i32>({
        {0, 0},
        {1, 3},
        {2, 2},
        {3, 1},
        {4, 4},
    });
  }

  static i32 BedrockVariantFromJavaVariant(i32 java) {
    return Forward(java, 0);
  }

  static i32 JavaVariantFromBedrockVariant(i32 b) {
    return Backward(b, 0);
  }

  static i32 JavaIntVariantFromJavaStringVariant(std::u8string const &v) {
    if (v == u8"wild") {
      return 1;
    } else if (v == u8"gold") {
      return 2;
    } else if (v == u8"cyan") {
      return 3;
    } else if (v == u8"blue") {
      return 4;
    } else {
      return 0;
    }
  }

  static std::u8string JavaStringVariantFromIntVariant(i32 v) {
    switch (v) {
    case 1:
      return u8"wild";
    case 2:
      return u8"gold";
    case 3:
      return u8"cyan";
    case 4:
      return u8"blue";
    case 0:
    default:
      return u8"lucy";
    }
  }
};

}; // namespace je2be
