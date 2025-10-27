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
  enum Motive : uint16_t {
    Bust = 1,
    Pigscene = 2,
    BurningSkull = 3,
    Pointer = 4,
    Skeleton = 5,
    DonkeyKong = 6,
    Fighters = 7,
    SkullAndRoses = 8,
    Match = 9,
    Stage = 10,
    Void = 11,
    Wither = 12,
    Sunset = 13,
    Courbet = 14,
    Creebet = 15,
    Sea = 16,
    Wanderer = 17,
    Graham = 18,
    Aztec2 = 19,
    Alban = 20,
    Bomb = 21,
    Kebab = 22,
    Wasteland = 23,
    Aztec = 24,
    Plant = 25,
    Pool = 26,

    Earth = 27,
    Wind = 28,
    Fire = 29,
    Water = 30,

    Meditative = 31,
    PrairieRide = 32,
    Baroque = 33,
    Humble = 34,
    Unpacked = 35,

    Endboss = 36,
    Tides = 37,
    Fern = 38,
    Sunflowers = 39,
    Cotan = 40,
    Owlemons = 41,
    Cavebird = 42,
    Bouquet = 43,
    Backyard = 44,
    Pond = 45,
    Passage = 46,
    Changing = 47,
    Finding = 48,
    Lowmist = 49,
    Orb = 50,

    Dennis = 51,
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

        {Meditative, Size(1, 1)},
        {PrairieRide, Size(1, 2)},
        {Baroque, Size(2, 2)},
        {Humble, Size(2, 2)},
        {Unpacked, Size(4, 4)},

        {Endboss, Size(3, 3)},
        {Tides, Size(3, 3)},
        {Fern, Size(3, 3)},
        {Sunflowers, Size(3, 3)},
        {Cotan, Size(3, 3)},
        {Owlemons, Size(3, 3)},
        {Cavebird, Size(3, 3)},
        {Bouquet, Size(3, 3)},
        {Backyard, Size(3, 4)},
        {Pond, Size(3, 4)},
        {Passage, Size(4, 2)},
        {Changing, Size(4, 2)},
        {Finding, Size(4, 2)},
        {Lowmist, Size(4, 2)},
        {Orb, Size(4, 4)},

        {Dennis, Size(3, 3)},
    };
    auto found = mapping.find(m);
    if (found != mapping.end()) {
      return found->second;
    }
    return nullopt;
  }

  static std::unordered_map<std::u8string, Motive> const *CreateRawTableJava() {
    return new std::unordered_map<std::u8string, Motive>{
        {u8"bust", Bust},
        {u8"pigscene", Pigscene},
        {u8"burning_skull", BurningSkull},
        {u8"pointer", Pointer},
        {u8"skeleton", Skeleton},
        {u8"donkey_kong", DonkeyKong},
        {u8"fighters", Fighters},
        {u8"skull_and_roses", SkullAndRoses},
        {u8"match", Match},
        {u8"bust", Bust},
        {u8"stage", Stage},
        {u8"void", Void},
        {u8"wither", Wither},
        {u8"sunset", Sunset},
        {u8"courbet", Courbet},
        {u8"creebet", Creebet},
        {u8"sea", Sea},
        {u8"wanderer", Wanderer},
        {u8"graham", Graham},
        {u8"aztec2", Aztec2},
        {u8"alban", Alban},
        {u8"bomb", Bomb},
        {u8"kebab", Kebab},
        {u8"wasteland", Wasteland},
        {u8"aztec", Aztec},
        {u8"plant", Plant},
        {u8"pool", Pool},

        {u8"earth", Earth},
        {u8"wind", Wind},
        {u8"fire", Fire},
        {u8"water", Water},

        {u8"meditative", Meditative},
        {u8"prairie_ride", PrairieRide},
        {u8"baroque", Baroque},
        {u8"humble", Humble},
        {u8"unpacked", Unpacked},

        {u8"endboss", Endboss},
        {u8"tides", Tides},
        {u8"fern", Fern},
        {u8"sunflowers", Sunflowers},
        {u8"cotan", Cotan},
        {u8"owlemons", Owlemons},
        {u8"cavebird", Cavebird},
        {u8"bouquet", Bouquet},
        {u8"backyard", Backyard},
        {u8"pond", Pond},
        {u8"passage", Passage},
        {u8"changing", Changing},
        {u8"finding", Finding},
        {u8"lowmist", Lowmist},
        {u8"orb", Orb},

        {u8"dennis", Dennis},
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
      u8string motifB;
      if (static_cast<uint16_t>(it.second) >= static_cast<uint16_t>(Meditative)) {
        motifB = motiveJ;
      } else {
        motifB = strings::UpperCamelFromSnake(motiveJ);
      }
      ret.insert(make_pair(motifB, it.second));
    }
    return new ReversibleMap<std::u8string, Motive>(ret);
  }

  static ReversibleMap<std::u8string, Motive> const *GetTableBedrock() {
    static std::unique_ptr<ReversibleMap<std::u8string, Motive> const> const sTable(CreateTableBedrock());
    return sTable.get();
  }

  static Motive MotiveFromJava(std::u8string const &motiveJ) {
    auto const *table = GetTableJava();
    auto found = table->forward(Namespace::Remove(motiveJ));
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

  static std::u8string JavaMotiveFromBedrockMotif(Motive m) {
    auto table = GetTableJava();
    auto found = table->backward(m);
    if (found) {
      return Namespace::Add(*found);
    } else {
      return u8"minecraft:aztec";
    }
  }

  static std::u8string BedrockMotifFromJavaMotive(Motive m) {
    auto table = GetTableBedrock();
    auto found = table->backward(m);
    if (found) {
      return *found;
    } else {
      return u8"Aztec";
    }
  }

  static std::optional<Pos3i> JavaTilePosFromBedrockPos(Pos3f pos, Facing4 direction, Motive motive) {
    return JavaTilePosFromBedrockOrLegacyConsolePos(pos, direction, motive);
  }

  static std::optional<Pos3i> JavaTilePosFromLegacyConsolePos(Pos3f pos, Facing4 direction, Motive motive) {
    return JavaTilePosFromBedrockOrLegacyConsolePos(pos, direction, motive);
  }

  static std::optional<Pos3f> BedrockPosFromJavaTilePos(Pos3i tile, Facing4 direction, Motive motive) {
    auto size = PaintingSize(motive);
    if (!size) {
      return std::nullopt;
    }
    float const thickness = 1.0f / 32.0f;

    int dh = 0;
    int dv = 0;
    if (size->fWidth >= 3) {
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

private:
  static std::optional<Pos3i> JavaTilePosFromBedrockOrLegacyConsolePos(Pos3f pos, Facing4 direction, Motive motive) {
    auto size = PaintingSize(motive);
    if (!size) {
      return std::nullopt;
    }
    int dh = 0;
    int dv = 0;
    if (size->fWidth >= 3) {
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
};

} // namespace je2be
