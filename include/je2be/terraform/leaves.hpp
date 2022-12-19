#pragma once

namespace je2be::terraform {

class Leaves {
  Leaves() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;
    if (!accessor.fHasLeaves) {
      return;
    }
    int cx = out.fChunkX;
    int cz = out.fChunkZ;
    optional<Volume> leavesRange;
    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (BlockPropertyAccessor::IsLeaves(p)) {
            if (leavesRange) {
              leavesRange->fStart.fX = (std::min)(leavesRange->fStart.fX, x);
              leavesRange->fStart.fY = (std::min)(leavesRange->fStart.fY, y);
              leavesRange->fStart.fZ = (std::min)(leavesRange->fStart.fZ, z);

              leavesRange->fEnd.fX = (std::max)(leavesRange->fEnd.fX, x);
              leavesRange->fEnd.fY = (std::max)(leavesRange->fEnd.fY, y);
              leavesRange->fEnd.fZ = (std::max)(leavesRange->fEnd.fZ, z);
            } else {
              Volume v(Pos3i(x, y, z), Pos3i(x, y, z));
              leavesRange = v;
            }
          }
        }
      }
    }
    if (!leavesRange) {
      return;
    }

    enum Distance : int8_t {
      Unknown = -2,
      Leaves = -1,
      Log = 0,
    };

    int x0 = leavesRange->fStart.fX - 7;
    int y0 = leavesRange->fStart.fY - 7;
    int z0 = leavesRange->fStart.fZ - 7;
    int x1 = leavesRange->fEnd.fX + 7;
    int y1 = leavesRange->fEnd.fY + 7;
    int z1 = leavesRange->fEnd.fZ + 7;
    Data3D<int8_t> data({x0, y0, z0}, {x1, y1, z1}, Unknown);
    for (int y = y0; y <= y1; y++) {
      for (int z = z0; z <= z1; z++) {
        for (int x = x0; x <= x1; x++) {
          auto block = cache.blockAt(x, y, z);
          if (!block) {
            continue;
          }
          if (IsLog(*block)) {
            data.set({x, y, z}, Log);
          } else if (BlockPropertyAccessor::IsLeaves(*block)) {
            data.set({x, y, z}, Leaves);
          }
        }
      }
    }
    static vector<Pos3i> const sDirections = {
        Pos3i(0, 1, 0),  // up
        Pos3i(0, -1, 0), // down
        Pos3i(0, 0, -1), // north
        Pos3i(1, 0, 0),  // east
        Pos3i(0, 0, 1),  // south
        Pos3i(-1, 0, 0), // west
    };
    for (int distance = 1; distance <= 7; distance++) {
      for (int y = y0; y <= y1; y++) {
        for (int z = z0; z <= z1; z++) {
          for (int x = x0; x <= x1; x++) {
            if (data.get({x, y, z}) != Leaves) {
              continue;
            }
            for (Pos3i const &d : sDirections) {
              int tx = x + d.fX;
              int ty = y + d.fY;
              int tz = z + d.fZ;
              if (tx < x0 || x1 < tx || ty < y0 || y1 < ty || tz < z0 || z1 < tz) {
                continue;
              }
              optional<int8_t> current = data.get({tx, ty, tz});
              if (!current) {
                continue;
              }
              if (*current == distance - 1) {
                data.set({x, y, z}, distance);
                break;
              }
            }
          }
        }
      }
    }
    for (int y = y0; y <= y1; y++) {
      for (int z = z0; z <= z1; z++) {
        for (int x = x0; x <= x1; x++) {
          int8_t distance = *data.get({x, y, z});
          if (distance == Leaves) {
            distance = 7;
          }
          if (distance <= 0) {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          auto replace = blockJ->applying({{"distance", to_string(distance)}});
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }

  static bool IsLog(mcfile::je::Block const &b) {
    using namespace mcfile::blocks;
    if (b.fName.ends_with("log") || b.fName.ends_with("wood")) {
      return true;
    }
    switch (b.fId) {
    case minecraft::warped_stem:
    case minecraft::crimson_stem:
    case minecraft::stripped_warped_stem:
    case minecraft::stripped_crimson_stem:
    case minecraft::stripped_warped_hyphae:
    case minecraft::stripped_crimson_hyphae:
      return true;
    default:
      return false;
    }
  }
};

} // namespace je2be::terraform
