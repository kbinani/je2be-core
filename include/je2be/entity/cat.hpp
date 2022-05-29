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

  static int32_t BedrockVariantFromCatType(Type catType) {
    auto const &table = GetTableBedrock();
    if (auto variantB = table.forward(catType); variantB) {
      return *variantB;
    } else {
      return 8;
    }
  }

  static Type CatTypeFromBedrockVariant(int32_t variantB) {
    auto const &table = GetTableBedrock();
    if (auto catType = table.backward(variantB); catType) {
      return static_cast<Type>(*catType);
    } else {
      return Tabby;
    }
  }

  static ReversibleMap<int32_t, int32_t> const *CreateTableBedrock() {
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

  static ReversibleMap<std::string, int32_t> const *CreateTableJava() {
    return new ReversibleMap<std::string, int32_t>({
        {"black", Tuxedo},
        {"red", Red},
        {"siamese", Siamese},
        {"british_shorthair", British},
        {"calico", Calico},
        {"persian", Persian},
        {"ragdoll", Ragdoll},
        {"white", White},
        {"jellie", Jellie},
        {"all_black", Black},
        {"tabby", Tabby},
    });
  }

  static ReversibleMap<int32_t, int32_t> const &GetTableBedrock() {
    static std::unique_ptr<ReversibleMap<int32_t, int32_t> const> sTable(CreateTableBedrock());
    return *sTable;
  }

  static std::string BedrockDefinitionKeyFromCatType(Type t) {
    return Backward(static_cast<int32_t>(t), "tabby");
  }

  static ReversibleMap<std::string, int32_t> const &GetTableJava() {
    static std::unique_ptr<ReversibleMap<std::string, int32_t> const> sTable(CreateTableJava());
    return *sTable;
  }

  static Type CatTypeFromJavaVariant(std::string const &variantJ) {
    auto const &table = GetTableJava();
    if (auto type = table.forward(variantJ); type) {
      return static_cast<Type>(*type);
    } else {
      return Tabby;
    }
  }

  static std::string JavaVariantFromCatType(Type type) {
    auto const &table = GetTableJava();
    if (auto variantJ = table.backward(type); variantJ) {
      return *variantJ;
    } else {
      return "tabby";
    }
  }
};

} // namespace je2be
