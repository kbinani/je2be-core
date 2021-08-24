#pragma once

namespace j2b {

enum class Dimension : uint8_t {
  Overworld = 0,
  Nether = 1,
  End = 2,
};

enum class LevelDirectoryStructure {
  Vanilla,
  Paper,
};

enum class BannerColorCodeBedrock : int32_t {
  Red = 1,
  Black = 0,
  Green = 2,
  Brown = 3,
  Blue = 4,
  Purple = 5,
  Cyan = 6,
  LightGray = 7,
  Gray = 8,
  Pink = 9,
  Lime = 10,
  Yellow = 11,
  LightBlue = 12,
  Magenta = 13,
  Orange = 14,
  White = 15,
};

inline BannerColorCodeBedrock BannerColorCodeFromName(std::string const &color) {
  static std::unordered_map<std::string, BannerColorCodeBedrock> const mapping = {
      {"white", BannerColorCodeBedrock::White},
      {"orange", BannerColorCodeBedrock::Orange},
      {"magenta", BannerColorCodeBedrock::Magenta},
      {"light_blue", BannerColorCodeBedrock::LightBlue},
      {"yellow", BannerColorCodeBedrock::Yellow},
      {"lime", BannerColorCodeBedrock::Lime},
      {"pink", BannerColorCodeBedrock::Pink},
      {"gray", BannerColorCodeBedrock::Gray},
      {"light_gray", BannerColorCodeBedrock::LightGray},
      {"cyan", BannerColorCodeBedrock::Cyan},
      {"purple", BannerColorCodeBedrock::Purple},
      {"blue", BannerColorCodeBedrock::Blue},
      {"brown", BannerColorCodeBedrock::Brown},
      {"green", BannerColorCodeBedrock::Green},
      {"red", BannerColorCodeBedrock::Red},
      {"black", BannerColorCodeBedrock::Black},
  };
  auto found = mapping.find(color);
  if (found == mapping.end()) {
    return BannerColorCodeBedrock::White;
  } else {
    return found->second;
  }
}

enum class ColorCodeJava : int32_t {
  White = 0,
  Orange = 1,
  Magenta = 2,
  LightBlue = 3,
  Yellow = 4,
  Lime = 5,
  Pink = 6,
  Gray = 7,
  LightGray = 8,
  Cyan = 9,
  Purple = 10,
  Blue = 11,
  Brown = 12,
  Green = 13,
  Red = 14,
  Black = 15,
};

inline ColorCodeJava ColorCodeJavaFromName(std::string const &color) {
  static std::unordered_map<std::string, ColorCodeJava> const mapping = {
      {"white", ColorCodeJava::White},
      {"orange", ColorCodeJava::Orange},
      {"magenta", ColorCodeJava::Magenta},
      {"light_blue", ColorCodeJava::LightBlue},
      {"yellow", ColorCodeJava::Yellow},
      {"lime", ColorCodeJava::Lime},
      {"pink", ColorCodeJava::Pink},
      {"gray", ColorCodeJava::Gray},
      {"light_gray", ColorCodeJava::LightGray},
      {"cyan", ColorCodeJava::Cyan},
      {"purple", ColorCodeJava::Purple},
      {"blue", ColorCodeJava::Blue},
      {"brown", ColorCodeJava::Brown},
      {"green", ColorCodeJava::Green},
      {"red", ColorCodeJava::Red},
      {"black", ColorCodeJava::Black},
  };
  auto found = mapping.find(color);
  if (found == mapping.end()) {
    return ColorCodeJava::White;
  } else {
    return found->second;
  }
}

inline std::string BedrockNameFromColorCodeJava(ColorCodeJava code) {
  switch (code) {
  case ColorCodeJava::Black:
    return "black";
  case ColorCodeJava::Blue:
    return "blue";
  case ColorCodeJava::Brown:
    return "brown";
  case ColorCodeJava::Cyan:
    return "cyan";
  case ColorCodeJava::Gray:
    return "gray";
  case ColorCodeJava::Green:
    return "green";
  case ColorCodeJava::LightBlue:
    return "light_blue";
  case ColorCodeJava::LightGray:
    return "silver";
  case ColorCodeJava::Lime:
    return "lime";
  case ColorCodeJava::Magenta:
    return "magenta";
  case ColorCodeJava::Orange:
    return "orange";
  case ColorCodeJava::Pink:
    return "pink";
  case ColorCodeJava::Purple:
    return "purple";
  case ColorCodeJava::Red:
    return "red";
  case ColorCodeJava::White:
    return "white";
  case ColorCodeJava::Yellow:
    return "yellow";
  }
}

inline int32_t BannerColorCodeFromJava(int32_t java) {
  ColorCodeJava ccj = (ColorCodeJava)java;
  switch (ccj) {
  case ColorCodeJava::Red:
    return (int32_t)BannerColorCodeBedrock::Red;
  case ColorCodeJava::Black:
    return (int32_t)BannerColorCodeBedrock::Black;
  case ColorCodeJava::Blue:
    return (int32_t)BannerColorCodeBedrock::Blue;
  case ColorCodeJava::Brown:
    return (int32_t)BannerColorCodeBedrock::Brown;
  case ColorCodeJava::Cyan:
    return (int32_t)BannerColorCodeBedrock::Cyan;
  case ColorCodeJava::Gray:
    return (int32_t)BannerColorCodeBedrock::Gray;
  case ColorCodeJava::Green:
    return (int32_t)BannerColorCodeBedrock::Green;
  case ColorCodeJava::LightBlue:
    return (int32_t)BannerColorCodeBedrock::LightBlue;
  case ColorCodeJava::LightGray:
    return (int32_t)BannerColorCodeBedrock::LightGray;
  case ColorCodeJava::Lime:
    return (int32_t)BannerColorCodeBedrock::Lime;
  case ColorCodeJava::Magenta:
    return (int32_t)BannerColorCodeBedrock::Magenta;
  case ColorCodeJava::Orange:
    return (int32_t)BannerColorCodeBedrock::Orange;
  case ColorCodeJava::Pink:
    return (int32_t)BannerColorCodeBedrock::Pink;
  case ColorCodeJava::Purple:
    return (int32_t)BannerColorCodeBedrock::Purple;
  case ColorCodeJava::White:
    return (int32_t)BannerColorCodeBedrock::White;
  case ColorCodeJava::Yellow:
    return (int32_t)BannerColorCodeBedrock::Yellow;
  }
}

class VillagerProfession {
  VillagerProfession() = delete;

public:
  enum RawValue : uint8_t {
    Fletcher,
    Librarian,
    Armorer,
    Cartographer,
    Shepherd,
    ToolSmith,
    Farmer,
    Fisherman,
    StoneMason,
    Cleric,
    LeatherWorker,
    Butcher,
    WeaponSmith,
  };

  constexpr VillagerProfession(RawValue value) : fValue(value) {}

  std::string tradeTablePath() const {
    switch (this->fValue) {
    case Fletcher:
      return "trading/economy_trades/fletcher_trades.json";
    case Librarian:
      return "trading/economy_trades/librarian_trades.json";
    case Armorer:
      return "trading/economy_trades/armorer_trades.json";
    case Cartographer:
      return "trading/economy_trades/cartographer_trades.json";
    case Shepherd:
      return "trading/economy_trades/shepherd_trades.json";
    case ToolSmith:
      return "trading/economy_trades/tool_smith_trades.json";
    case Farmer:
      return "trading/economy_trades/farmer_trades.json";
    case Fisherman:
      return "trading/economy_trades/fisherman_trades.json";
    case StoneMason:
      return "trading/economy_trades/stone_mason_trades.json";
    case Cleric:
      return "trading/economy_trades/cleric_trades.json";
    case LeatherWorker:
      return "trading/economy_trades/leather_worker_trades.json";
    case Butcher:
      return "trading/economy_trades/butcher_trades.json";
    case WeaponSmith:
      return "trading/economy_trades/weapon_smith_trades.json";
    }
    return "";
  }

  static std::optional<VillagerProfession> FromJavaProfession(std::string profession) {
    if (strings::StartsWith(profession, "minecraft:")) {
      profession = profession.substr(10);
    }
    if (profession == "shepherd") {
      return Shepherd;
    } else if (profession == "farmer") {
      return Farmer;
    } else if (profession == "fisherman") {
      return Fisherman;
    } else if (profession == "butcher") {
      return Butcher;
    } else if (profession == "armorer") {
      return Armorer;
    } else if (profession == "cartographer") {
      return Cartographer;
    } else if (profession == "fletcher") {
      return Fletcher;
    } else if (profession == "weaponsmith") {
      return WeaponSmith;
    } else if (profession == "toolsmith") {
      return ToolSmith;
    } else if (profession == "mason") {
      return StoneMason;
    } else if (profession == "leatherworker") {
      return LeatherWorker;
    } else if (profession == "cleric") {
      return Cleric;
    } else if (profession == "librarian") {
      return Librarian;
    }
    return std::nullopt;
  }

  int32_t variant() const {
    switch (fValue) {
    case Shepherd:
      return 3;
    case Farmer:
      return 1;
    case Fisherman:
      return 2;
    case Butcher:
      return 11;
    case Armorer:
      return 8;
    case Cartographer:
      return 6;
    case Fletcher:
      return 4;
    case WeaponSmith:
      return 9;
    case ToolSmith:
      return 10;
    case StoneMason:
      return 13;
    case LeatherWorker:
      return 12;
    case Cleric:
      return 7;
    case Librarian:
      return 5;
    }
  }

  std::string string() const {
    switch (fValue) {
    case Shepherd:
      return "shepherd";
    case Farmer:
      return "farmer";
    case Fisherman:
      return "fisherman";
    case Butcher:
      return "butcher";
    case Armorer:
      return "armorer";
    case Cartographer:
      return "cartographer";
    case Fletcher:
      return "fletcher";
    case WeaponSmith:
      return "weaponsmith";
    case ToolSmith:
      return "toolsmith";
    case StoneMason:
      return "mason";
    case LeatherWorker:
      return "leatherworker";
    case Cleric:
      return "cleric";
    case Librarian:
      return "librarian";
    }
  }

private:
  RawValue fValue;
};

class VillagerType {
public:
  enum RawValue : uint8_t {
    Savanna,
    Plains,
    Desert,
    Jungle,
    Snow,
    Swamp,
    Taiga,
  };

  constexpr VillagerType(RawValue value) : fValue(value) {}

  static std::optional<VillagerType> FromJavaType(std::string type) {
    if (strings::StartsWith(type, "minecraft:")) {
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

  int32_t variant() const {
    switch (fValue) {
    case Savanna:
      return 3;
    case Plains:
      return 0;
    case Desert:
      return 1;
    case Jungle:
      return 2;
    case Snow:
      return 4;
    case Swamp:
      return 5;
    case Taiga:
      return 6;
    }
  }

  std::string string() const {
    switch (fValue) {
    case Savanna:
      return "savanna";
    case Plains:
      return "plains";
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
    }
  }

private:
  RawValue fValue;
};

} // namespace j2b
