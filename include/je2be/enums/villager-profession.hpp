#pragma once

namespace je2be {

class VillagerProfession {
  VillagerProfession() = delete;

public:
  enum Variant : int32_t {
    Farmer = 1,
    Fisherman = 2,
    Shepherd = 3,
    Fletcher = 4,
    Librarian = 5,
    Cartographer = 6,
    Cleric = 7,
    Armorer = 8,
    WeaponSmith = 9,
    ToolSmith = 10,
    Butcher = 11,
    LeatherWorker = 12,
    StoneMason = 13,
  };

  constexpr VillagerProfession(Variant variant) : fVariant(variant) {}

  std::string tradeTablePath() const {
    switch (fVariant) {
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
    return fVariant;
  }

  std::string string() const {
    switch (fVariant) {
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
  Variant fVariant;
};
} // namespace je2be
