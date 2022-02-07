#pragma once

namespace je2be {

class Cat : StaticReversibleMap<int32_t, int32_t, Cat> {
public:
  enum Type : int32_t {
    Tabby = 0,
    Tuxedo = 1,
    Red = 2,
    Siamese = 3,
    British = 4,
    Calico = 5,
    Persian = 6,
    Ragdoll = 7,
    White = 8,
    Jellie = 9,
    Black = 10,
  };

  static Type CatTypeFromJavaCatType(int32_t catType) {
    return static_cast<Type>(catType);
  }

  static int32_t BedrockVariantFromJavaCatType(Type catType) {
    return Forward(catType, 8);
  }

  static int32_t JavaCatTypeFromBedrockVariant(int32_t variant) {
    return Backward(variant, 0);
  }

  static ReversibleMap<int32_t, int32_t> const *CreateTable() {
    return new ReversibleMap<int32_t, int32_t>({
        {Tabby, 8},
        {Tuxedo, 1},
        {Red, 2},
        {Siamese, 3},
        {British, 4},
        {Calico, 5},
        {Persian, 6},
        {Ragdoll, 7},
        {White, 0},
        {Jellie, 10},
        {Black, 9},
    });
  }

  static std::string BedrockNameFromCatType(Type t) {
    switch (t) {
    case 1:
      return "tuxedo";
    case 2:
      return "red";
    case 3:
      return "siamese";
    case 4:
      return "british";
    case 5:
      return "calico";
    case 6:
      return "persian";
    case 7:
      return "ragdoll";
    case 8:
      return "white";
    case 9:
      return "jellie";
    case 10:
      return "black";
    case 0:
    default:
      return "tabby";
    }
  }
};

} // namespace je2be
