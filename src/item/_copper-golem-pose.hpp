#pragma once

#include "_static-reversible-map.hpp"

namespace je2be {

class CopperGolemPose : StaticReversibleMap<std::u8string, i32, CopperGolemPose> {
  friend class StaticReversibleMap;

protected:
  static ReversibleMap<std::u8string, i32> const *CreateTable() {
    return new ReversibleMap<std::u8string, i32>({{u8"standing", 0}, {u8"sitting", 1}, {u8"running", 2}, {u8"star", 3}});
  }

public:
  static std::u8string JavaFromBedrock(i32 poseB) {
    return Backward(poseB, u8"standing");
  }

  static i32 BedrockFromJava(std::u8string const &poseJ) {
    return Forward(poseJ, 0);
  }
};

} // namespace je2be
