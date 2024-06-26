#include "terraform/_leaves.hpp"

#include "_data-version.hpp"
#include "_data3d.hpp"
#include "_volume.hpp"
#include "java/_block-data.hpp"

namespace je2be::terraform {

class Leaves::Impl {
  Impl() = delete;

public:
  template <class Block>
  static void Do(mcfile::je::Chunk &out, BlockAccessor<Block> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;

    enum Distance : i8 {
      Unknown = -2,
      Leaves = -1,
      Log = 0,
    };

    if (!accessor.fHasLeaves) {
      return;
    }
    int cx = out.fChunkX;
    int cz = out.fChunkZ;
    optional<Volume> leavesRange;
    Data3d<i8> data({cx * 16 - 7, accessor.minBlockY(), cz * 16 - 7}, {cx * 16 + 15 + 7, accessor.maxBlockY(), cz * 16 + 15 + 7}, Unknown);
    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (p == BlockPropertyAccessor::LEAVES) {
            data.set({x, y, z}, Leaves);
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
    for (int y = y0; y <= y1; y++) {
      for (int z = z0; z <= z1; z++) {
        for (int x = x0; x <= x1; x++) {
          Pos3i p(x, y, z);
          if (data.get(p) == Leaves) {
            continue;
          }
          shared_ptr<Block const> block = cache.blockAt(p.fX, p.fY, p.fZ);
          if (!block) {
            continue;
          }
          if (IsLog(*block)) {
            data.set(p, Log);
          } else if (BlockPropertyAccessor::IsLeaves(*block)) {
            data.set(p, Leaves);
          }
        }
      }
    }
    static array<Pos3i, 6> const sDirections = {
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
            Pos3i p(x, y, z);
            if (data.get(p) != Leaves) {
              continue;
            }
            for (Pos3i const &d : sDirections) {
              optional<i8> adjacent = data.get(p + d);
              if (!adjacent) {
                continue;
              }
              if (*adjacent == distance - 1) {
                data.set(p, distance);
                break;
              }
            }
          }
        }
      }
    }
    for (int y = leavesRange->fStart.fY; y <= leavesRange->fEnd.fY; y++) {
      for (int z = leavesRange->fStart.fZ; z <= leavesRange->fEnd.fZ; z++) {
        for (int x = leavesRange->fStart.fX; x <= leavesRange->fEnd.fX; x++) {
          auto distance = data.get({x, y, z});
          if (!distance) {
            continue;
          }
          if (*distance == Leaves) {
            distance = 7;
          }
          if (*distance <= 0) {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          auto replace = blockJ->applying({{u8"distance", mcfile::String::ToString(*distance)}});
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }

  template <class Block>
  static bool IsLog(Block const &b);

  static std::unordered_set<std::u8string> *CreateBedrockLogBlocksSet();
};

template <>
bool Leaves::Impl::IsLog(mcfile::je::Block const &b) {
  using namespace mcfile::blocks;
  if (b.fName.ends_with(u8"log") || b.fName.ends_with(u8"wood")) {
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

template <>
bool Leaves::Impl::IsLog(mcfile::be::Block const &b) {
  using namespace std;
  static unique_ptr<unordered_set<u8string> const> const sTable(CreateBedrockLogBlocksSet());
  return sTable->find(b.fName) != sTable->end();
}

std::unordered_set<std::u8string> *Leaves::Impl::CreateBedrockLogBlocksSet() {
  using namespace std;
  using namespace mcfile::blocks;
  unordered_set<u8string> *ret = new unordered_set<u8string>();
  DataVersion dv(mcfile::je::Chunk::kDataVersion, mcfile::je::Chunk::kDataVersion);
  for (BlockId id = unknown + 1; id < minecraft::minecraft_max_block_id; id++) {
    auto blockJ = mcfile::je::Block::FromId(id, mcfile::je::Chunk::kDataVersion);
    if (!IsLog(*blockJ)) {
      continue;
    }
    auto blockB = je2be::java::BlockData::From(blockJ, nullptr, dv, {});
    if (!blockB) {
      continue;
    }
    auto nameB = blockB->string(u8"name");
    if (!nameB) {
      continue;
    }
    ret->insert(*nameB);
  }
  return ret;
}

void Leaves::Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &cache, BlockPropertyAccessor const &accessor) {
  Impl::Do(out, cache, accessor);
}

void Leaves::Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::be::Block> &cache, BlockPropertyAccessor const &accessor) {
  Impl::Do(out, cache, accessor);
}

} // namespace je2be::terraform
