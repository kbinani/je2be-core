#pragma once

#include "_namespace.hpp"

namespace je2be {

class VillagerProfession {
  VillagerProfession() = delete;

public:
  enum Variant : i32 {
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
    Nitwit = 14,
  };

  constexpr VillagerProfession(Variant variant) : fVariant(variant) {}

  std::optional<std::u8string> tradeTablePath() const {
    switch (fVariant) {
    case Fletcher:
      return u8"trading/economy_trades/fletcher_trades.json";
    case Librarian:
      return u8"trading/economy_trades/librarian_trades.json";
    case Armorer:
      return u8"trading/economy_trades/armorer_trades.json";
    case Cartographer:
      return u8"trading/economy_trades/cartographer_trades.json";
    case Shepherd:
      return u8"trading/economy_trades/shepherd_trades.json";
    case ToolSmith:
      return u8"trading/economy_trades/tool_smith_trades.json";
    case Farmer:
      return u8"trading/economy_trades/farmer_trades.json";
    case Fisherman:
      return u8"trading/economy_trades/fisherman_trades.json";
    case StoneMason:
      return u8"trading/economy_trades/stone_mason_trades.json";
    case Cleric:
      return u8"trading/economy_trades/cleric_trades.json";
    case LeatherWorker:
      return u8"trading/economy_trades/leather_worker_trades.json";
    case Butcher:
      return u8"trading/economy_trades/butcher_trades.json";
    case WeaponSmith:
      return u8"trading/economy_trades/weapon_smith_trades.json";
    case Nitwit:
      return std::nullopt;
    }
    return std::nullopt;
  }

  static std::optional<VillagerProfession> FromJavaProfession(std::u8string profession) {
    profession = Namespace::Remove(profession);
    if (profession == u8"shepherd") {
      return Shepherd;
    } else if (profession == u8"farmer") {
      return Farmer;
    } else if (profession == u8"fisherman") {
      return Fisherman;
    } else if (profession == u8"butcher") {
      return Butcher;
    } else if (profession == u8"armorer") {
      return Armorer;
    } else if (profession == u8"cartographer") {
      return Cartographer;
    } else if (profession == u8"fletcher") {
      return Fletcher;
    } else if (profession == u8"weaponsmith") {
      return WeaponSmith;
    } else if (profession == u8"toolsmith") {
      return ToolSmith;
    } else if (profession == u8"mason") {
      return StoneMason;
    } else if (profession == u8"leatherworker") {
      return LeatherWorker;
    } else if (profession == u8"cleric") {
      return Cleric;
    } else if (profession == u8"librarian") {
      return Librarian;
    } else if (profession == u8"nitwit") {
      return Nitwit;
    }
    return std::nullopt;
  }

  i32 variant() const {
    return fVariant;
  }

  std::u8string string() const {
    switch (fVariant) {
    case Shepherd:
      return u8"shepherd";
    case Farmer:
      return u8"farmer";
    case Fisherman:
      return u8"fisherman";
    case Butcher:
      return u8"butcher";
    case Armorer:
      return u8"armorer";
    case Cartographer:
      return u8"cartographer";
    case Fletcher:
      return u8"fletcher";
    case WeaponSmith:
      return u8"weaponsmith";
    case ToolSmith:
      return u8"toolsmith";
    case StoneMason:
      return u8"mason";
    case LeatherWorker:
      return u8"leatherworker";
    case Cleric:
      return u8"cleric";
    case Librarian:
      return u8"librarian";
    case Nitwit:
      return u8"nitwit";
    }
    return u8"none";
  }

private:
  Variant fVariant;
};
} // namespace je2be
