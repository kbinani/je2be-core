#pragma once

#include <je2be/strings.hpp>

#include "_namespace.hpp"
#include "_pos3.hpp"
#include "_reversible-map.hpp"
#include "_size.hpp"
#include "enums/_facing4.hpp"

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

  static std::unordered_map<std::u8string, Motive> const *CreateRawTableJava() {
    return new std::unordered_map<std::u8string, Motive>{
        {u8"minecraft:bust", Bust},
        {u8"minecraft:pigscene", Pigscene},
        {u8"minecraft:burning_skull", BurningSkull},
        {u8"minecraft:pointer", Pointer},
        {u8"minecraft:skeleton", Skeleton},
        {u8"minecraft:donkey_kong", DonkeyKong},
        {u8"minecraft:fighters", Fighters},
        {u8"minecraft:skull_and_roses", SkullAndRoses},
        {u8"minecraft:match", Match},
        {u8"minecraft:bust", Bust},
        {u8"minecraft:stage", Stage},
        {u8"minecraft:void", Void},
        {u8"minecraft:wither", Wither},
        {u8"minecraft:sunset", Sunset},
        {u8"minecraft:courbet", Courbet},
        {u8"minecraft:creebet", Creebet},
        {u8"minecraft:sea", Sea},
        {u8"minecraft:wanderer", Wanderer},
        {u8"minecraft:graham", Graham},
        {u8"minecraft:aztec2", Aztec2},
        {u8"minecraft:alban", Alban},
        {u8"minecraft:bomb", Bomb},
        {u8"minecraft:kebab", Kebab},
        {u8"minecraft:wasteland", Wasteland},
        {u8"minecraft:aztec", Aztec},
        {u8"minecraft:plant", Plant},
        {u8"minecraft:pool", Pool},

        {u8"minecraft:earth", Earth},
        {u8"minecraft:wind", Wind},
        {u8"minecraft:fire", Fire},
        {u8"minecraft:water", Water},
    };
  }

  static ReversibleMap<std::u8string, Motive> const *CreateTableJava() {
    return new ReversibleMap(*CreateRawTableJava());
  }

  static ReversibleMap<std::u8string, Motive> const *GetTableJava() {
    static std::unique_ptr<ReversibleMap<std::u8string, Motive> const> const sTable(CreateTableJava());
    return sTable.get();
  }

  static ReversibleMap<std::u8string, Motive> const *CreateTableBedrock() {
    using namespace std;
    unique_ptr<unordered_map<u8string, Motive> const> j2b(CreateRawTableJava());
    unordered_map<std::u8string, Motive> ret;
    for (auto const &it : *j2b) {
      u8string motiveJ = it.first;
      u8string motiveB = strings::UpperCamelFromSnake(Namespace::Remove(motiveJ));
      ret.insert(make_pair(motiveB, it.second));
    }
    return new ReversibleMap<std::u8string, Motive>(ret);
  }

  static ReversibleMap<std::u8string, Motive> const *GetTableBedrock() {
    static std::unique_ptr<ReversibleMap<std::u8string, Motive> const> const sTable(CreateTableBedrock());
    return sTable.get();
  }

  static Motive MotiveFromJava(std::u8string const &motiveJ) {
    auto const *table = GetTableJava();
    auto found = table->forward(motiveJ);
    if (found) {
      return *found;
    } else {
      return Aztec;
    }
  }

  static Motive MotiveFromBedrock(std::u8string const &motiveB) {
    auto const *table = GetTableBedrock();
    auto found = table->forward(motiveB);
    if (found) {
      return *found;
    } else {
      return Aztec;
    }
  }

  static std::u8string JavaFromMotive(Motive m) {
    auto table = GetTableJava();
    auto found = table->backward(m);
    if (found) {
      return *found;
    } else {
      return u8"minecraft:aztec";
    }
  }

  static std::u8string BedrockFromMotive(Motive m) {
    auto table = GetTableBedrock();
    auto found = table->backward(m);
    if (found) {
      return *found;
    } else {
      return u8"Aztec";
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
