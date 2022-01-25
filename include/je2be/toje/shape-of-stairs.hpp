#pragma once

namespace je2be::toje {

class ShapeOfStairs {
  ShapeOfStairs() = delete;

public:
  static void Do(mcfile::je::Chunk &out, ChunkCache<3, 3> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasStairs) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    auto const &in = cache.at(cx, cz);
    if (!in) {
      return;
    }

    for (int y = mcfile::be::Chunk::kMinBlockY; y <= mcfile::be::Chunk::kMaxBlockY; y++) {
      for (int lz = 0; lz < 16; lz++) {
        int z = cz * 16 + lz;
        for (int lx = 0; lx < 16; lx++) {
          int x = cx * 16 + lx;
          auto p = accessor.property(x, y, z);
          if (!BlockPropertyAccessor::IsStairs(p)) {
            continue;
          }
          auto stairs = in->blockAt(x, y, z);
          assert(stairs);
          if (!stairs) {
            continue;
          }
          auto direction = stairs->fStates->int32("weirdo_direction");
          if (!direction) {
            continue;
          }
          auto upsideDown = stairs->fStates->boolean("upside_down_bit");
          if (!upsideDown) {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }

          Pos2i vec = VecFromWeirdoDirection(*direction);
          optional<int> outerWeirdoDirection;
          optional<int> innerWeirdoDirection;

          Pos2i outer = Pos2i(x, z) + vec;
          auto outerBlock = cache.blockAt(outer.fX, y, outer.fZ);
          if (outerBlock) {
            if (BlockPropertyAccessor::IsStairs(*outerBlock)) {
              if (*upsideDown == outerBlock->fStates->boolean("upside_down_bit")) {
                outerWeirdoDirection = outerBlock->fStates->int32("weirdo_direction");
              }
            }
          }

          Pos2i inner = Pos2i(x, z) + Pos2i(-vec.fX, -vec.fZ);
          auto innerBlock = cache.blockAt(inner.fX, y, inner.fZ);
          if (innerBlock) {
            if (BlockPropertyAccessor::IsStairs(*innerBlock)) {
              if (*upsideDown == innerBlock->fStates->boolean("upside_down_bit")) {
                innerWeirdoDirection = innerBlock->fStates->int32("weirdo_direction");
              }
            }
          }

          auto shape = Shape(*direction, outerWeirdoDirection, innerWeirdoDirection);
          map<string, string> props(blockJ->fProperties);
          props["shape"] = shape;
          auto newBlock = make_shared<mcfile::je::Block const>(blockJ->fName, props);
          out.setBlockAt(x, y, z, newBlock);
        }
      }
    }
  }

  static std::string Shape(int direction, std::optional<int> outer, std::optional<int> inner) {
    Pos2i vec = VecFromWeirdoDirection(direction);
    if (outer) {
      Pos2i o = VecFromWeirdoDirection(*outer);
      if (Left90(vec) == o) {
        return "outer_left";
      } else if (Right90(vec) == o) {
        return "outer_right";
      }
    }
    if (inner) {
      Pos2i i = VecFromWeirdoDirection(*inner);
      if (Left90(vec) == i) {
        return "inner_left";
      } else if (Right90(vec) == i) {
        return "inner_right";
      }
    }
    return "straight";
  }

  static Pos2i VecFromWeirdoDirection(int32_t d) {
    switch (d) {
    case 2: // south
      return Pos2i(0, 1);
    case 3: // north
      return Pos2i(0, -1);
    case 1: // west
      return Pos2i(-1, 0);
    case 0: // east
    default:
      return Pos2i(1, 0);
    }
  }

  static Pos2i Right90(Pos2i vec) {
    constexpr int cos90 = 0;
    constexpr int sin90 = 1;
    int x = cos90 * vec.fX - sin90 * vec.fZ;
    int z = sin90 * vec.fX + cos90 * vec.fZ;
    return Pos2i(x, z);
  }

  static Pos2i Left90(Pos2i vec) {
    constexpr int cosM90 = 0;
    constexpr int sinM90 = -1;
    int x = cosM90 * vec.fX - sinM90 * vec.fZ;
    int z = sinM90 * vec.fX + cosM90 * vec.fZ;
    return Pos2i(x, z);
  }
};

} // namespace je2be::toje
