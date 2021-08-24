#pragma once

namespace j2b {

class VillagerType {
  VillagerType() = delete;

public:
  enum Variant : int32_t {
    Plains = 0,
    Desert = 1,
    Jungle = 2,
    Savanna = 3,
    Snow = 4,
    Swamp = 5,
    Taiga = 6,
  };

  constexpr VillagerType(Variant variant) : fVariant(variant) {}

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
    return fVariant;
  }

  std::string string() const {
    switch (fVariant) {
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
  Variant fVariant;
};

} // namespace j2b
