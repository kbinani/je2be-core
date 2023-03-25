#pragma once

#include "_namespace.hpp"

namespace je2be {

class VillagerType {
  VillagerType() = delete;

public:
  enum Variant : i32 {
    Plains = 0,
    Desert = 1,
    Jungle = 2,
    Savanna = 3,
    Snow = 4,
    Swamp = 5,
    Taiga = 6,
  };

  constexpr VillagerType(Variant variant) : fVariant(variant) {}

  /*
  biome         Variant     skin                MarkVariant     PreferredProfession     definition          SkinID
  plains        11          villager_skin_4     0               butcher                 N/A                 4
  desert        10          villager_skin_5     1               toolsmith               desert_villager     5
  savanna       1           villager_skin_1     3               farmer                  savanna_villager    1
  taiga         12          villager_skin_0     6               leatherworker           taiga_villager      0
  ice_plains    8           villager_skin_2     4               armorer                 snow_villager       2
  jungle        3           villager_skin_1     2               shepherd                jungle_villager     1
  swampland     11          villager_skin_0     5               butcher                 swamp_villager      0
  */

  static std::optional<VillagerType> FromJavaType(std::u8string type) {
    type = Namespace::Remove(type);
    if (type == u8"savanna") {
      return Savanna;
    } else if (type == u8"plains") {
      return Plains;
    } else if (type == u8"desert") {
      return Desert;
    } else if (type == u8"jungle") {
      return Jungle;
    } else if (type == u8"snow") {
      return Snow;
    } else if (type == u8"swamp") {
      return Swamp;
    } else if (type == u8"taiga") {
      return Taiga;
    }
    return std::nullopt;
  }

  i32 variant() const {
    return fVariant;
  }

  std::u8string string() const {
    switch (fVariant) {
    case Savanna:
      return u8"savanna";
    case Desert:
      return u8"desert";
    case Jungle:
      return u8"jungle";
    case Snow:
      return u8"snow";
    case Swamp:
      return u8"swamp";
    case Taiga:
      return u8"taiga";
    case Plains:
    default:
      return u8"plains";
    }
  }

private:
  Variant fVariant;
};

} // namespace je2be
