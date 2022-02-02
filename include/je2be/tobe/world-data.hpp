#pragma once

namespace je2be::tobe {

class WorldData {
public:
  explicit WorldData(mcfile::Dimension dim) : fDim(dim) {}

  void addStatChunkVersion(uint32_t chunkDataVersion) { fStat.addChunkVersion(chunkDataVersion); }
  void addStat(uint64_t numChunks, uint64_t numBlockEntitites, uint64_t numEntities) { fStat.add(numChunks, numBlockEntitites, numEntities); }

  void addStatError(mcfile::Dimension dim, int32_t chunkX, int32_t chunkZ) { fStat.addError(dim, chunkX, chunkZ); }

  void addPortalBlock(int32_t x, int32_t y, int32_t z, bool xAxis) { fPortalBlocks.add(x, y, z, xAxis); }

  void addMap(int32_t javaMapId, std::shared_ptr<CompoundTag> const &item) { fMapItems[javaMapId] = item; }

  void addAutonomousEntity(std::shared_ptr<CompoundTag> const &entity) { fAutonomousEntities.push_back(entity); }

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

  void drain(LevelData &wd) {
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
    for (StructurePiece const &p : fStructures) {
      wd.fStructures.add(p, fDim);
    }
    wd.fStat.merge(fStat);
    wd.fMaxChunkLastUpdate = std::max(wd.fMaxChunkLastUpdate, fMaxChunkLastUpdate);
  }

  void drain(WorldData &out) {
    assert(fDim == out.fDim);
    if (fDim != out.fDim) {
      return;
    }
    fPortalBlocks.drain(out.fPortalBlocks);
    for (auto const &it : fMapItems) {
      out.fMapItems[it.first] = it.second;
    }
    std::unordered_map<int32_t, std::shared_ptr<CompoundTag>>().swap(fMapItems);
    for (auto const &it : fAutonomousEntities) {
      out.fAutonomousEntities.push_back(it);
    }
    std::vector<std::shared_ptr<CompoundTag>>().swap(fAutonomousEntities);
    for (auto const &pos : fEndPortalsInEndDimension) {
      out.fEndPortalsInEndDimension.insert(pos);
    }
    std::unordered_set<Pos3i, Pos3iHasher>().swap(fEndPortalsInEndDimension);
    for (auto const &piece : fStructures) {
      out.fStructures.add(piece);
    }
    out.fStat.merge(fStat);
    out.fMaxChunkLastUpdate = std::max(out.fMaxChunkLastUpdate, fMaxChunkLastUpdate);
  }

  std::shared_ptr<Tag> toNbt() const {
    using namespace std;
    using namespace mcfile;
    auto tag = make_shared<CompoundTag>();
    CompoundTag &c = *tag;
    c["dim"] = make_shared<ByteTag>(static_cast<uint8_t>(fDim));
    c["maxChunkLastUpdate"] = make_shared<LongTag>(fMaxChunkLastUpdate);
    c["portalBlocks"] = fPortalBlocks.toNbt();

    auto mapItems = make_shared<ListTag>(Tag::Type::Compound);
    for (auto it : fMapItems) {
      int number = it.first;
      auto tag = it.second;
      auto c = make_shared<CompoundTag>();
      (*c)["number"] = make_shared<IntTag>(number);
      (*c)["tag"] = tag->clone();
      mapItems->push_back(c);
    }
    c["mapItems"] = mapItems;

    auto autonomousEntities = make_shared<ListTag>(Tag::Type::Compound);
    for (auto it : fAutonomousEntities) {
      autonomousEntities->push_back(it->clone());
    }
    c["autonomousEntities"] = autonomousEntities;

    auto endPortalsInEndDimension = make_shared<ListTag>(Tag::Type::Compound);
    for (Pos3i const &pos : fEndPortalsInEndDimension) {
      endPortalsInEndDimension->push_back(Pos3iToNbt(pos));
    }
    c["endPortalsInEndDimension"] = endPortalsInEndDimension;

    c["structures"] = fStructures.toNbt();

    return tag;
  }

  static std::shared_ptr<WorldData> FromNbt(Tag const &tag) {
    using namespace std;
    using namespace mcfile;
    auto c = tag.asCompound();
    if (!c) {
      return nullptr;
    }
    auto dim = c->byte("dim");
    if (!dim) {
      return nullptr;
    }
    auto ret = make_shared<WorldData>(static_cast<Dimension>(*dim));

    auto maxChunkLastUpdate = c->int64("maxChunkLastUpdate");
    if (!maxChunkLastUpdate) {
      return nullptr;
    }
    ret->fMaxChunkLastUpdate = *maxChunkLastUpdate;

    auto portalBlocksTag = c->tag("portalBlocks");
    if (!portalBlocksTag) {
      return nullptr;
    }
    auto portalBlocks = PortalBlocks::FromNbt(*portalBlocksTag);
    if (!portalBlocks) {
      return nullptr;
    }
    ret->fPortalBlocks = *portalBlocks;

    auto mapItemsTag = c->listTag("mapItems");
    if (!mapItemsTag) {
      return nullptr;
    }
    for (auto const &it : *mapItemsTag) {
      auto c = it->asCompound();
      if (!c) {
        return nullptr;
      }
      auto number = c->int32("number");
      auto t = c->compoundTag("tag");
      if (!number || !t) {
        return nullptr;
      }
      ret->fMapItems[*number] = t->copy();
    }

    auto autonomousEntitiesTag = c->listTag("autonomousEntities");
    if (!autonomousEntitiesTag) {
      return nullptr;
    }
    for (auto const &it : *autonomousEntitiesTag) {
      auto c = it->asCompound();
      ret->fAutonomousEntities.push_back(c->copy());
    }

    auto endPortalsInEndDimensionTag = c->listTag("endPortalsInEndDimension");
    if (!endPortalsInEndDimensionTag) {
      return nullptr;
    }
    for (auto const &it : *endPortalsInEndDimensionTag) {
      auto pos = Pos3iFromNbt(*it);
      if (!pos) {
        return nullptr;
      }
      ret->fEndPortalsInEndDimension.insert(*pos);
    }

    auto structuresTag = c->tag("structures");
    if (!structuresTag) {
      return nullptr;
    }
    auto structures = StructurePieceCollection::FromNbt(*structuresTag);
    if (!structures) {
      ret->fStructures = *structures;
    }

    return ret;
  }

private:
  void addStructures(CompoundTag const &structure, StructureType type) {
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

  static std::optional<Volume> GetBoundingBox(CompoundTag const &tag, std::string const &name) {
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
  std::unordered_map<int32_t, std::shared_ptr<CompoundTag>> fMapItems;
  std::vector<std::shared_ptr<CompoundTag>> fAutonomousEntities;
  std::unordered_set<Pos3i, Pos3iHasher> fEndPortalsInEndDimension;
  StructurePieceCollection fStructures;
  Statistics fStat;
  int64_t fMaxChunkLastUpdate = 0;
};

} // namespace je2be::tobe
