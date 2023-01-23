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
};

}; // namespace je2be
