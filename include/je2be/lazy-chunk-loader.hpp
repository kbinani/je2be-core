namespace j2b {
class LazyChunkLoader {
public:
  explicit LazyChunkLoader(mcfile::Region const &region) : fRegion(region) {
  }

  std::shared_ptr<mcfile::Block const> blockAt(int bx, int by, int bz) {
    auto chunk = chunkAtBlock(bx, bz);
    if (chunk) {
      return chunk->blockAt(bx, by, bz);
    } else {
      return nullptr;
    }
  }

  void eachTileEntitiesInRange(int centerBlockX, int centerBlockY, int centerBlockZ, int radius, std::function<bool(mcfile::nbt::CompoundTag const &, Pos3)> callback) {
    if (radius < 0) {
      return;
    }
    int minBx = centerBlockX - radius;
    int maxBx = centerBlockX + radius;
    int minCx = mcfile::Coordinate::ChunkFromBlock(minBx);
    int maxCx = mcfile::Coordinate::ChunkFromBlock(maxBx);

    int minBy = centerBlockY - radius;
    int maxBy = centerBlockY + radius;

    int minBz = centerBlockZ - radius;
    int maxBz = centerBlockZ + radius;
    int minCz = mcfile::Coordinate::ChunkFromBlock(minBz);
    int maxCz = mcfile::Coordinate::ChunkFromBlock(maxBz);

    for (int cx = minCx; cx <= maxCx; cx++) {
      for (int cz = minCz; cz <= maxCz; cz++) {
        auto const &chunk = chunkAt(cx, cz);
        if (!chunk) {
          continue;
        }
        for (auto const &item : chunk->fTileEntities) {
          if (!item) {
            continue;
          }
          auto pos = props::GetPos3<'x', 'y', 'z'>(*item);
          if (!pos) {
            continue;
          }
          if (pos->fX < minBx || maxBx < pos->fX) {
            continue;
          }
          if (pos->fY < minBy || maxBy < pos->fY) {
            continue;
          }
          if (pos->fZ < minBz || maxBz < pos->fZ) {
            continue;
          }
          if (!callback(*item, *pos)) {
            return;
          }
        }
      }
    }
  }

private:
  std::shared_ptr<mcfile::Chunk const> chunkAtBlock(int bx, int bz) {
    return chunkAt(mcfile::Coordinate::ChunkFromBlock(bx), mcfile::Coordinate::ChunkFromBlock(bz));
  }

  std::shared_ptr<mcfile::Chunk const> chunkAt(int cx, int cz) {
    Pos2 pos(cx, cz);
    auto found = fCache.find(pos);
    if (found == fCache.end()) {
      auto chunk = fRegion.chunkAt(pos.fX, pos.fZ);
      fCache.insert(std::make_pair(pos, chunk));
      return chunk;
    } else {
      return found->second;
    }
  }

private:
  mcfile::Region const &fRegion;
  std::unordered_map<Pos2, std::shared_ptr<mcfile::Chunk const>, Pos2Hasher> fCache;
};
} // namespace j2b
