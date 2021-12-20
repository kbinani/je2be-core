#pragma once

namespace je2be::tobe {

class WorldDataFragment {
public:
  explicit WorldDataFragment(mcfile::Dimension dim) : fDim(dim) {}

  void addStatChunkVersion(uint32_t chunkDataVersion) { fStat.addChunkVersion(chunkDataVersion); }
  void addStat(uint64_t numChunks, uint64_t numBlockEntitites, uint64_t numEntities) { fStat.add(numChunks, numBlockEntitites, numEntities); }

  void addStatError(mcfile::Dimension dim, int32_t chunkX, int32_t chunkZ) { fStat.addError(dim, chunkX, chunkZ); }

  void addPortalBlock(int32_t x, int32_t y, int32_t z, bool xAxis) { fPortalBlocks.add(x, y, z, xAxis); }

  void addMap(int32_t javaMapId, std::shared_ptr<mcfile::nbt::CompoundTag> const &item) { fMapItems[javaMapId] = item; }

  void addAutonomousEntity(std::shared_ptr<mcfile::nbt::CompoundTag> const &entity) { fAutonomousEntities.push_back(entity); }

  void addEndPortal(int32_t x, int32_t y, int32_t z) {
    Pos3i p(x, y, z);
    fEndPortalsInEndDimension.insert(p);
  }

  void addStructures(mcfile::je::Chunk const &chunk) {
    if (!chunk.fStructures) {
      return;
    }
    auto start = chunk.fStructures->compoundTag("Starts");
    if (!start) {
      return;
    }
    auto fortress = start->compoundTag("fortress");
    if (fortress) {
      addStructures(*fortress, StructureType::Fortress);
    }
    auto monument = start->compoundTag("monument");
    if (monument) {
      addStructures(*monument, StructureType::Monument);
    }
    auto outpost = start->compoundTag("pillager_outpost");
    if (outpost) {
      addStructures(*outpost, StructureType::Outpost);
    }
  }

  void updateChunkLastUpdate(mcfile::je::Chunk const &chunk) {
    fMaxChunkLastUpdate = std::max(fMaxChunkLastUpdate, chunk.fLastUpdate);
  }

  void drain(WorldData &wd) {
    wd.fPortals.add(fPortalBlocks, fDim);
    for (auto const &it : fMapItems) {
      wd.fMapItems[it.first] = it.second;
    }
    for (auto const &e : fAutonomousEntities) {
      wd.fAutonomousEntities.push_back(e);
    }
    for (auto const &pos : fEndPortalsInEndDimension) {
      wd.fEndPortalsInEndDimension.insert(pos);
    }
    for (auto it = fStructures.begin(); it != fStructures.end(); it++) {
      wd.fStructures.add(*it, fDim);
    }
    wd.fStat.merge(fStat);
    wd.fMaxChunkLastUpdate = std::max(wd.fMaxChunkLastUpdate, fMaxChunkLastUpdate);
  }

private:
  void addStructures(mcfile::nbt::CompoundTag const &structure, StructureType type) {
    auto children = structure.listTag("Children");
    if (!children) {
      return;
    }
    for (auto const &it : *children) {
      auto c = it->asCompound();
      if (!c) {
        continue;
      }
      auto bb = GetBoundingBox(*c, "BB");
      if (!bb) {
        continue;
      }
      StructurePiece p(bb->fStart, bb->fEnd, type);
      fStructures.add(p);
    }
  }

  static std::optional<Volume> GetBoundingBox(mcfile::nbt::CompoundTag const &tag, std::string const &name) {
    auto bb = tag.intArrayTag(name);
    if (!bb) {
      return std::nullopt;
    }
    auto const &value = bb->value();
    if (value.size() < 6) {
      return std::nullopt;
    }
    Pos3i start(value[0], value[1], value[2]);
    Pos3i end(value[3], value[4], value[5]);
    return Volume(start, end);
  }

public:
  mcfile::Dimension const fDim;

private:
  PortalBlocks fPortalBlocks;
  std::unordered_map<int32_t, std::shared_ptr<mcfile::nbt::CompoundTag>> fMapItems;
  std::vector<std::shared_ptr<mcfile::nbt::CompoundTag>> fAutonomousEntities;
  std::unordered_set<Pos3i, Pos3iHasher> fEndPortalsInEndDimension;
  StructurePieceCollection fStructures;
  Statistics fStat;
  int64_t fMaxChunkLastUpdate = 0;
};

} // namespace je2be::tobe
