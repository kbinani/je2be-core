#pragma once

namespace j2b {

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
