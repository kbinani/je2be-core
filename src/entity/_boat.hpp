#pragma once

namespace je2be {

class Boat : StaticReversibleMap<std::u8string, i32, Boat> {
  Boat() = delete;

public:
  static ReversibleMap<std::u8string, i32> const *CreateTable() {
    return new ReversibleMap<std::u8string, i32>({{u8"oak", 0},
                                                  {u8"spruce", 1},
                                                  {u8"birch", 2},
                                                  {u8"jungle", 3},
                                                  {u8"acacia", 4},
                                                  {u8"dark_oak", 5},
                                                  {u8"mangrove", 6},
                                                  {u8"bamboo", 7},
                                                  {u8"cherry", 8},
                                                  {u8"pale_oak", 9}});
  }

  static i32 BedrockVariantFromJavaType(std::u8string const &type) {
    return Forward(type, 0);
  }

  static std::u8string JavaTypeFromBedrockVariant(i32 variant) {
    return Backward(variant, u8"oak");
  }
};

} // namespace je2be
