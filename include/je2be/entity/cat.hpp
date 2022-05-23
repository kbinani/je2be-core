#pragma once

namespace je2be {

class Cat : StaticReversibleMap<std::string, int32_t, Cat> {
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

  static Type CatTypeFromJavaLegacyCatType(int32_t catType) {
    return static_cast<Type>(catType);
  }

  static int32_t BedrockVariantFromJavaLegacyCatType(Type catType) {
    auto const &table = GetTableLegacy();
    if (auto variantB = table.forward(catType); variantB) {
      return *variantB;
    } else {
      return 8;
    }
  }

  static int32_t JavaLegacyCatTypeFromBedrockVariant(int32_t variant) {
    auto const &table = GetTableLegacy();
    if (auto catType = table.backward(variant); catType) {
      return *catType;
    } else {
      return 0;
    }
  }

  static ReversibleMap<int32_t, int32_t> const *CreateTableLegacy() {
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

  static ReversibleMap<std::string, int32_t> const *CreateTable() {
    return new ReversibleMap<std::string, int32_t>({
        {"tuxedo", Tuxedo},
        {"red", Red},
        {"siamese", Siamese},
        {"british", British},
        {"calico", Calico},
        {"persian", Persian},
        {"ragdoll", Ragdoll},
        {"white", White},
        {"jellie", Jellie},
        {"black", Black},
        {"tabby", Tabby},
    });
  }

  static ReversibleMap<int32_t, int32_t> const &GetTableLegacy() {
    static std::unique_ptr<ReversibleMap<int32_t, int32_t> const> sTable(CreateTableLegacy());
    return *sTable;
  }

  static std::string NameFromCatType(Type t) {
    return Backward(static_cast<int32_t>(t), "tabby");
  }

  static Type CatTypeFromName(std::string const &name) {
    return static_cast<Type>(Forward(name, static_cast<int32_t>(Tabby)));
  }
};

} // namespace je2be
