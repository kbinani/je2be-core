#pragma once

namespace je2be {

class Boat : StaticReversibleMap<std::string, i32, Boat> {
  Boat() = delete;

public:
  static ReversibleMap<std::string, i32> const *CreateTable() {
    return new ReversibleMap<std::string, i32>({{"oak", 0},
                                                {"spruce", 1},
                                                {"birch", 2},
                                                {"jungle", 3},
                                                {"acacia", 4},
                                                {"dark_oak", 5},
                                                {"mangrove", 6},
                                                {"bamboo", 7},
                                                {"cherry", 8}});
  }

  static i32 BedrockVariantFromJavaType(std::string const &type) {
    i32 ret = Forward(type, 0);
    // TODO:1.20
    if (ret > 7) {
      return 0;
    } else {
      return ret;
    }
  }

  static std::string JavaTypeFromBedrockVariant(i32 variant) {
    return Backward(variant, "oak");
  }
};

} // namespace je2be
