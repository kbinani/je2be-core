#pragma once

#include <je2be/enums/facing4.hpp>
#include <je2be/pos3.hpp>
#include <je2be/size.hpp>
#include <je2be/strings.hpp>

namespace je2be {

class Painting {
  Painting() = delete;

public:
  enum Motive {
    Bust,
    Pigscene,
    BurningSkull,
    Pointer,
    Skeleton,
    DonkeyKong,
    Fighters,
    SkullAndRoses,
    Match,
    Stage,
    Void,
    Wither,
    Sunset,
    Courbet,
    Creebet,
    Sea,
    Wanderer,
    Graham,
    Aztec2,
    Alban,
    Bomb,
    Kebab,
    Wasteland,
    Aztec,
    Plant,
    Pool,

    Earth,
    Wind,
    Fire,
    Water,
  };

  static std::optional<Size> PaintingSize(Motive m) {
    using namespace std;
    static unordered_map<Motive, Size> const mapping = {
        {Pigscene, Size(4, 4)},
        {BurningSkull, Size(4, 4)},
        {Pointer, Size(4, 4)},
        {Skeleton, Size(4, 3)},
        {DonkeyKong, Size(4, 3)},
        {Fighters, Size(4, 2)},
        {SkullAndRoses, Size(2, 2)},
        {Match, Size(2, 2)},
        {Bust, Size(2, 2)},
        {Stage, Size(2, 2)},
        {Void, Size(2, 2)},
        {Wither, Size(2, 2)},
        {Sunset, Size(2, 1)},
        {Courbet, Size(2, 1)},
        {Creebet, Size(2, 1)},
        {Sea, Size(2, 1)},
        {Wanderer, Size(1, 2)},
        {Graham, Size(1, 2)},
        {Aztec2, Size(1, 1)},
        {Alban, Size(1, 1)},
        {Bomb, Size(1, 1)},
        {Kebab, Size(1, 1)},
        {Wasteland, Size(1, 1)},
        {Aztec, Size(1, 1)},
        {Plant, Size(1, 1)},
        {Pool, Size(2, 1)},

        {Earth, Size(2, 2)},
        {Wind, Size(2, 2)},
        {Fire, Size(2, 2)},
        {Water, Size(2, 2)},
    };
    auto found = mapping.find(m);
    if (found != mapping.end()) {
      return found->second;
    }
    return nullopt;
  }

  static std::unordered_map<std::string, Motive> const *CreateRawTableJava() {
    return new std::unordered_map<std::string, Motive>{
        {"minecraft:bust", Bust},
        {"minecraft:pigscene", Pigscene},
        {"minecraft:burning_skull", BurningSkull},
        {"minecraft:pointer", Pointer},
        {"minecraft:skeleton", Skeleton},
        {"minecraft:donkey_kong", DonkeyKong},
        {"minecraft:fighters", Fighters},
        {"minecraft:skull_and_roses", SkullAndRoses},
        {"minecraft:match", Match},
        {"minecraft:bust", Bust},
        {"minecraft:stage", Stage},
        {"minecraft:void", Void},
        {"minecraft:wither", Wither},
        {"minecraft:sunset", Sunset},
        {"minecraft:courbet", Courbet},
        {"minecraft:creebet", Creebet},
        {"minecraft:sea", Sea},
        {"minecraft:wanderer", Wanderer},
        {"minecraft:graham", Graham},
        {"minecraft:aztec2", Aztec2},
        {"minecraft:alban", Alban},
        {"minecraft:bomb", Bomb},
        {"minecraft:kebab", Kebab},
        {"minecraft:wasteland", Wasteland},
        {"minecraft:aztec", Aztec},
        {"minecraft:plant", Plant},
        {"minecraft:pool", Pool},

        {"minecraft:earth", Earth},
        {"minecraft:wind", Wind},
        {"minecraft:fire", Fire},
        {"minecraft:water", Water},
    };
  }

  static ReversibleMap<std::string, Motive> const *CreateTableJava() {
    return new ReversibleMap(*CreateRawTableJava());
  }

  static ReversibleMap<std::string, Motive> const *GetTableJava() {
    static std::unique_ptr<ReversibleMap<std::string, Motive> const> const sTable(CreateTableJava());
    return sTable.get();
  }

  static ReversibleMap<std::string, Motive> const *CreateTableBedrock() {
    using namespace std;
    unique_ptr<unordered_map<string, Motive> const> j2b(CreateRawTableJava());
    unordered_map<std::string, Motive> ret;
    for (auto const &it : *j2b) {
      string motiveJ = it.first;
      string motiveB = strings::UpperCamelFromSnake(motiveJ.substr(10));
      ret.insert(make_pair(motiveB, it.second));
    }
    return new ReversibleMap<std::string, Motive>(ret);
  }

  static ReversibleMap<std::string, Motive> const *GetTableBedrock() {
    static std::unique_ptr<ReversibleMap<std::string, Motive> const> const sTable(CreateTableBedrock());
    return sTable.get();
  }

  static Motive MotiveFromJava(std::string const &motiveJ) {
    auto const *table = GetTableJava();
    auto found = table->forward(motiveJ);
    if (found) {
      return *found;
    } else {
      return Aztec;
    }
  }

  static Motive MotiveFromBedrock(std::string const &motiveB) {
    auto const *table = GetTableBedrock();
    auto found = table->forward(motiveB);
    if (found) {
      return *found;
    } else {
      return Aztec;
    }
  }

  static std::string JavaFromMotive(Motive m) {
    auto table = GetTableJava();
    auto found = table->backward(m);
    if (found) {
      return *found;
    } else {
      return "minecraft:aztec";
    }
  }

  static std::string BedrockFromMotive(Motive m) {
    auto table = GetTableBedrock();
    auto found = table->backward(m);
    if (found) {
      return *found;
    } else {
      return "Aztec";
    }
  }

  static std::optional<Pos3i> JavaTilePosFromBedrockPos(Pos3f pos, Facing4 direction, Motive motive) {
    auto size = PaintingSize(motive);
    if (!size) {
      return std::nullopt;
    }
    int dh = 0;
    int dv = 0;
    if (size->fWidth >= 4) {
      dh = 1;
    }
    if (size->fHeight >= 3) {
      dv = 1;
    }

    double const thickness = 1.0 / 32.0;
    double tileX;
    double tileZ;
    double tileY = pos.fY + dv - size->fHeight * 0.5;

    switch (direction) {
    case Facing4::West:
      tileX = pos.fX - 1 + thickness;
      tileZ = pos.fZ + dh - size->fWidth * 0.5;
      break;
    case Facing4::North:
      tileX = pos.fX - 1 - dh + size->fWidth * 0.5;
      tileZ = pos.fZ - 1 + thickness;
      break;
    case Facing4::East:
      tileX = pos.fX + thickness;
      tileZ = pos.fZ - 1 - dh + size->fWidth * 0.5;
      break;
    case Facing4::South:
    default:
      tileX = pos.fX + dh - size->fWidth * 0.5;
      tileZ = pos.fZ - thickness;
      break;
    }
    return Pos3i((int)round(tileX), (int)round(tileY), (int)round(tileZ));
  }

  static std::optional<Pos3f> BedrockPosFromJavaTilePos(Pos3i tile, Facing4 direction, Motive motive) {
    auto size = PaintingSize(motive);
    if (!size) {
      return std::nullopt;
    }
    float const thickness = 1.0f / 32.0f;

    int dh = 0;
    int dv = 0;
    if (size->fWidth >= 4) {
      dh = 1;
    }
    if (size->fHeight >= 3) {
      dv = 1;
    }

    float x, z;
    float y = tile.fY - dv + size->fHeight * 0.5f;

    if (direction == Facing4::South) {
      x = tile.fX - dh + size->fWidth * 0.5f;
      z = tile.fZ + thickness;
    } else if (direction == Facing4::West) {
      x = tile.fX + 1 - thickness;
      z = tile.fZ - dh + size->fWidth * 0.5f;
    } else if (direction == Facing4::North) {
      x = tile.fX + 1 + dh - size->fWidth * 0.5f;
      z = tile.fZ + 1 - thickness;
    } else {
      x = tile.fX + thickness;
      z = tile.fZ + 1 + dh - size->fWidth * 0.5f;
    }
    return Pos3f(x, y, z);
  }
};

} // namespace je2be
