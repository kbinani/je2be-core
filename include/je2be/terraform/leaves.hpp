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
    int x0 = leavesRange->fStart.fX - 7;
    int y0 = leavesRange->fStart.fY - 7;
    int z0 = leavesRange->fStart.fZ - 7;
    int x1 = leavesRange->fEnd.fX + 7;
    int y1 = leavesRange->fEnd.fY + 7;
    int z1 = leavesRange->fEnd.fZ + 7;
    int sx = x1 - x0 + 1;
    int sy = y1 - y0 + 1;
    int sz = z1 - z0 + 1;
    vector<int8_t> data(sx * sy * sz, -2);
    for (int y = y0; y <= y1; y++) {
      for (int z = z0; z <= z1; z++) {
        for (int x = x0; x <= x1; x++) {
          auto block = cache.blockAt(x, y, z);
          if (!block) {
            continue;
          }
          int index = ((y - y0) * sz + (z - z0)) * sx + (x - x0);
          if (IsLog(*block)) {
            data[index] = 0;
          } else if (BlockPropertyAccessor::IsLeaves(*block)) {
            data[index] = -1;
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
            int index = ((y - y0) * sz + (z - z0)) * sx + (x - x0);
            if (data[index] != -1) {
              continue;
            }
            for (Pos3i const &d : sDirections) {
              int tx = x + d.fX;
              int ty = y + d.fY;
              int tz = z + d.fZ;
              if (tx < x0 || x1 < tx || ty < y0 || y1 < ty || tz < z0 || z1 < tz) {
                continue;
              }
              int i = ((ty - y0) * sz + (tz - z0)) * sx + (tx - x0);
              if (i < 0 || data.size() <= i) {
                continue;
              }
              if (data[i] == distance - 1) {
                data[index] = distance;
                break;
              }
            }
          }
        }
      }
    }
    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          if (x < x0 || x1 < x || y < y0 || y1 < y || z < z0 || z1 < z) {
            continue;
          }
          int index = ((y - y0) * sz + (z - z0)) * sx + (x - x0);
          int distance = data[index];
          if (distance <= 0) {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          map<string, string> props(blockJ->fProperties);
          props["distance"] = to_string(distance);
          auto replace = make_shared<mcfile::je::Block const>(blockJ->fName, props);
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
