#pragma once

#include <je2be/_static-reversible-map.hpp>

namespace je2be {

class Axolotl : StaticReversibleMap<int32_t, int32_t, Axolotl> {
  Axolotl() = delete;

public:
  static ReversibleMap<int32_t, int32_t> const *CreateTable() {
    return new ReversibleMap<int32_t, int32_t>({
        {0, 0},
        {1, 3},
        {2, 2},
        {3, 1},
        {4, 4},
    });
  }

  static int32_t BedrockVariantFromJavaVariant(int32_t java) {
    return Forward(java, 0);
  }

  static int32_t JavaVariantFromBedrockVariant(int32_t b) {
    return Backward(b, 0);
  }
};

}; // namespace je2be
