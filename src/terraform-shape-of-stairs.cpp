#include "terraform/_shape-of-stairs.hpp"

#include "terraform/_block-property-accessor.hpp"

namespace je2be::terraform {

class ShapeOfStairs::Impl {
  Impl() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &dataAccessor, BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasStairs) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int lz = 0; lz < 16; lz++) {
        int z = cz * 16 + lz;
        for (int lx = 0; lx < 16; lx++) {
          int x = cx * 16 + lx;
          auto p = accessor.property(x, y, z);
          if (p != BlockPropertyAccessor::STAIRS) {
            continue;
          }
          auto stairs = StairsBlockData(dataAccessor, x, y, z);
          assert(stairs);
          if (!stairs) {
            continue;
          }
          auto direction = stairs->facing();
          auto upsideDown = stairs->half();
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }

          Pos2i vec = VecFromWeirdoDirection(direction);
          optional<mcfile::blocks::data::Directional::BlockFace> outerWeirdoDirection;
          optional<mcfile::blocks::data::Directional::BlockFace> innerWeirdoDirection;
          optional<mcfile::blocks::data::Directional::BlockFace> leftWeirdoDirection;
          optional<mcfile::blocks::data::Directional::BlockFace> rightWeirdoDirection;

          Pos2i outer = Pos2i(x, z) + vec;
          if (auto outerBlock = StairsBlockData(dataAccessor, outer.fX, y, outer.fZ); outerBlock) {
            if (upsideDown == outerBlock->half()) {
              outerWeirdoDirection = outerBlock->facing();
            }
          }

          Pos2i inner = Pos2i(x, z) + Pos2i(-vec.fX, -vec.fZ);
          if (auto innerBlock = StairsBlockData(dataAccessor, inner.fX, y, inner.fZ); innerBlock) {
            if (upsideDown == innerBlock->half()) {
              innerWeirdoDirection = innerBlock->facing();
            }
          }

          Pos2i left = Pos2i(x, z) + Left90(vec);
          if (auto leftBlock = StairsBlockData(dataAccessor, left.fX, y, left.fZ); leftBlock) {
            if (upsideDown == leftBlock->half()) {
              leftWeirdoDirection = leftBlock->facing();
            }
          }

          Pos2i right = Pos2i(x, z) + Right90(vec);
          if (auto rightBlock = StairsBlockData(dataAccessor, right.fX, y, right.fZ); rightBlock) {
            if (upsideDown == rightBlock->half()) {
              rightWeirdoDirection = rightBlock->facing();
            }
          }

          auto shape = Shape(direction, outerWeirdoDirection, innerWeirdoDirection, leftWeirdoDirection, rightWeirdoDirection);
          auto newBlock = blockJ->applying({{u8"shape", shape}});
          out.setBlockAt(x, y, z, newBlock);
        }
      }
    }
  }

  static std::u8string Shape(mcfile::blocks::data::Directional::BlockFace direction,
                             std::optional<mcfile::blocks::data::Directional::BlockFace> outer,
                             std::optional<mcfile::blocks::data::Directional::BlockFace> inner,
                             std::optional<mcfile::blocks::data::Directional::BlockFace> left,
                             std::optional<mcfile::blocks::data::Directional::BlockFace> right) {
    Pos2i vec = VecFromWeirdoDirection(direction);
    if (outer) {
      Pos2i o = VecFromWeirdoDirection(*outer);
      if (Left90(vec) == o && direction != right) {
        return u8"outer_left";
      } else if (Right90(vec) == o && direction != left) {
        return u8"outer_right";
      }
    }
    if (inner) {
      Pos2i i = VecFromWeirdoDirection(*inner);
      if (Left90(vec) == i && direction != left) {
        return u8"inner_left";
      } else if (Right90(vec) == i && direction != right) {
        return u8"inner_right";
      }
    }
    return u8"straight";
  }

  static std::shared_ptr<mcfile::blocks::data::type::Stairs> StairsBlockData(BlockAccessor<mcfile::je::Block> &dataAccessor, int x, int y, int z) {
    auto blockJ = dataAccessor.blockAt(x, y, z);
    if (!blockJ) {
      return nullptr;
    }
    auto data = mcfile::blocks::BlockData::Make(*blockJ);
    return std::dynamic_pointer_cast<mcfile::blocks::data::type::Stairs>(data);
  }

  static Pos2i VecFromWeirdoDirection(mcfile::blocks::data::Directional::BlockFace d) {
    using namespace mcfile::blocks::data;
    switch (d) {
    case Directional::BlockFace::South:
      return Pos2i(0, 1);
    case Directional::BlockFace::North:
      return Pos2i(0, -1);
    case Directional::BlockFace::West:
      return Pos2i(-1, 0);
    case Directional::BlockFace::East:
    default:
      return Pos2i(1, 0);
    }
  }
};

void ShapeOfStairs::Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &dataAccessor, BlockPropertyAccessor const &accessor) {
  Impl::Do(out, dataAccessor, accessor);
}

Pos2i ShapeOfStairs::VecFromWeirdoDirection(mcfile::blocks::data::Directional::BlockFace d) {
  return Impl::VecFromWeirdoDirection(d);
}

} // namespace je2be::terraform
