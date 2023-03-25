#pragma once

namespace je2be {

class Cat : StaticReversibleMap<std::u8string, i32, Cat> {
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

  static ReversibleMap<std::u8string, i32> const *CreateTable() {
    return new ReversibleMap<std::u8string, i32>({
        {u8"tuxedo", Tuxedo},
        {u8"red", Red},
        {u8"siamese", Siamese},
        {u8"british", British},
        {u8"calico", Calico},
        {u8"persian", Persian},
        {u8"ragdoll", Ragdoll},
        {u8"white", White},
        {u8"jellie", Jellie},
        {u8"black", Black},
        {u8"tabby", Tabby},
    });
  }

  static ReversibleMap<std::u8string, i32> const *CreateTableJava() {
    return new ReversibleMap<std::u8string, i32>({
        {u8"black", Tuxedo},
        {u8"red", Red},
        {u8"siamese", Siamese},
        {u8"british_shorthair", British},
        {u8"calico", Calico},
        {u8"persian", Persian},
        {u8"ragdoll", Ragdoll},
        {u8"white", White},
        {u8"jellie", Jellie},
        {u8"all_black", Black},
        {u8"tabby", Tabby},
    });
  }

  static ReversibleMap<i32, i32> const &GetTableBedrock() {
    static std::unique_ptr<ReversibleMap<i32, i32> const> sTable(CreateTableBedrock());
    return *sTable;
  }

  static std::u8string BedrockDefinitionKeyFromCatType(Type t) {
    return Backward(static_cast<i32>(t), u8"tabby");
  }

  static ReversibleMap<std::u8string, i32> const &GetTableJava() {
    static std::unique_ptr<ReversibleMap<std::u8string, i32> const> sTable(CreateTableJava());
    return *sTable;
  }

  static Type CatTypeFromJavaVariant(std::u8string const &variantJ) {
    auto const &table = GetTableJava();
    if (auto type = table.forward(variantJ); type) {
      return static_cast<Type>(*type);
    } else {
      return Tabby;
    }
  }

  static std::u8string JavaVariantFromCatType(Type type) {
    auto const &table = GetTableJava();
    if (auto variantJ = table.backward(type); variantJ) {
      return *variantJ;
    } else {
      return u8"tabby";
    }
  }
};

} // namespace je2be
