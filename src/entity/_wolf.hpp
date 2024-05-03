#pragma once

namespace je2be {

class Wolf : StaticReversibleMap<std::u8string, i32, Wolf> {
public:
  static i32 BedrockVariantFromJavaVariant(std::u8string const &vJ) {
    return Forward(Namespace::Remove(vJ), 0);
  }

  static std::u8string JavaVariantFromBedrockVariant(i32 vB) {
    return Namespace::Add(Backward(vB, u8"pale"));
  }

  static ReversibleMap<std::u8string, i32> *CreateTable() {
    return new ReversibleMap<std::u8string, i32>({{u8"pale", 0},
                                                  {u8"ashen", 1},
                                                  {u8"black", 2},
                                                  {u8"chestnut", 3},
                                                  {u8"rusty", 4},
                                                  {u8"snowy", 5},
                                                  {u8"spotted", 6},
                                                  {u8"striped", 7},
                                                  {u8"woods", 8}});
  }
};

} // namespace je2be
