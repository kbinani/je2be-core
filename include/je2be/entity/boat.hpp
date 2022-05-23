#pragma once

namespace je2be {

class Boat : StaticReversibleMap<std::string, int32_t, Boat> {
  Boat() = delete;

public:
  static ReversibleMap<std::string, int32_t> const *CreateTable() {
    return new ReversibleMap<std::string, int32_t>({{"oak", 0},
                                                    {"spruce", 1},
                                                    {"birch", 2},
                                                    {"jungle", 3},
                                                    {"acacia", 4},
                                                    {"dark_oak", 5},
                                                    {"mangrove", 6}});
  }

  static int32_t BedrockVariantFromJavaType(std::string const &type) {
    return Forward(type, 0);
  }

  static std::string JavaTypeFromBedrockVariant(int32_t variant) {
    return Backward(variant, "oak");
  }
};

} // namespace je2be
