#pragma once

namespace je2be {

class Cat : StaticReversibleMap<std::string, i32, Cat> {
public:
  enum Type : i32 {
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

  static Type CatTypeFromJavaLegacyCatType(i32 catType) {
    return static_cast<Type>(catType);
  }

  static i32 BedrockVariantFromCatType(Type catType) {
    auto const &table = GetTableBedrock();
    if (auto variantB = table.forward(catType); variantB) {
      return *variantB;
    } else {
      return 8;
    }
  }

  static Type CatTypeFromBedrockVariant(i32 variantB) {
    auto const &table = GetTableBedrock();
    if (auto catType = table.backward(variantB); catType) {
      return static_cast<Type>(*catType);
    } else {
      return Tabby;
    }
  }

  static ReversibleMap<i32, i32> const *CreateTableBedrock() {
    return new ReversibleMap<i32, i32>({
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

  static ReversibleMap<std::string, i32> const *CreateTable() {
    return new ReversibleMap<std::string, i32>({
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

  static ReversibleMap<std::string, i32> const *CreateTableJava() {
    return new ReversibleMap<std::string, i32>({
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

  static ReversibleMap<i32, i32> const &GetTableBedrock() {
    static std::unique_ptr<ReversibleMap<i32, i32> const> sTable(CreateTableBedrock());
    return *sTable;
  }

  static std::string BedrockDefinitionKeyFromCatType(Type t) {
    return Backward(static_cast<i32>(t), "tabby");
  }

  static ReversibleMap<std::string, i32> const &GetTableJava() {
    static std::unique_ptr<ReversibleMap<std::string, i32> const> sTable(CreateTableJava());
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
