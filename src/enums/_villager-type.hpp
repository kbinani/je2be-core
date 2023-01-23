#pragma once

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

  static std::optional<VillagerType> FromJavaType(std::string type) {
    if (type.starts_with("minecraft:")) {
      type = type.substr(10);
    }
    if (type == "savanna") {
      return Savanna;
    } else if (type == "plains") {
      return Plains;
    } else if (type == "desert") {
      return Desert;
    } else if (type == "jungle") {
      return Jungle;
    } else if (type == "snow") {
      return Snow;
    } else if (type == "swamp") {
      return Swamp;
    } else if (type == "taiga") {
      return Taiga;
    }
    return std::nullopt;
  }

  i32 variant() const {
    return fVariant;
  }

  std::string string() const {
    switch (fVariant) {
    case Savanna:
      return "savanna";
    case Desert:
      return "desert";
    case Jungle:
      return "jungle";
    case Snow:
      return "snow";
    case Swamp:
      return "swamp";
    case Taiga:
      return "taiga";
    case Plains:
    default:
      return "plains";
    }
  }

private:
  Variant fVariant;
};

} // namespace je2be
