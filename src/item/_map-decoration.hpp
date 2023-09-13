#pragma once

#include <je2be/integers.hpp>

#include "_static-reversible-map.hpp"

namespace je2be {

class MapDecoration : StaticReversibleMap<i8, i32, MapDecoration> {
public:
  static std::optional<i32> BedrockTypeFromJava(i8 type) {
    if (auto be = Forward(type, -1); be > 0) {
      return be;
    } else {
      return std::nullopt;
    }
  }

  static std::optional<i8> JavaTypeFromBedrock(i32 type) {
    if (auto je = Backward(type, -1); je > 0) {
      return je;
    } else {
      return std::nullopt;
    }
  }

  static ReversibleMap<i8, i32> const *CreateTable() {
    return new ReversibleMap<i8, i32>({
        {9, 15},  // id = "+", monument
        {8, 14},  // id = "+", mansion
        {26, 4},  // id = "+", buried treasure
        {27, 17}, // village_desert
        {28, 18}, // village_plains
        {29, 19}, // village_savanna
        {30, 20}, // village_snowy
        {31, 21}, // village_taiga
        {32, 22}, // jungle_temple
        {33, 23}, // swamp_hut
    });
  }
};

} // namespace je2be
