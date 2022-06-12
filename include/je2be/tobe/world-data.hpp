#pragma once

namespace je2be::tobe {

class WorldData {
public:
  explicit WorldData(mcfile::Dimension dim) : fDim(dim) {}

  void addError(Status::ErrorData error) {
    if (!fError) {
      fError = error;
    }
  }

  void addPortalBlock(int32_t x, int32_t y, int32_t z, bool xAxis) { fPortalBlocks.add(x, y, z, xAxis); }

  void addMap(int32_t javaMapId, CompoundTagPtr const &item) { fMapItems[javaMapId] = item; }

  void addAutonomousEntity(CompoundTagPtr const &entity) { fAutonomousEntities.push_back(entity); }

  void addEndPortal(int32_t x, int32_t y, int32_t z) {
    Pos3i p(x, y, z);
    fEndPortalsInEndDimension.insert(p);
  }

  void addStructures(mcfile::je::Chunk const &chunk) {
    if (!chunk.fStructures) {
      return;
    }
    auto start = chunk.fStructures->compoundTag("starts"); // 1.18~
    if (!start) {
      start = chunk.fStructures->compoundTag("Starts"); // ~1.17
    }
    if (!start) {
      return;
    }
    auto fortress = start->compoundTag("fortress");
    if (!fortress) {
      fortress = start->compoundTag("Fortress");
    }
    if (fortress) {
      addStructures(*fortress, StructureType::Fortress);
    }
    auto monument = start->compoundTag("monument"); // 1.16~
    if (!monument) {
      monument = start->compoundTag("Monument"); // ~1.15
    }
    if (monument) {
      addStructures(*monument, StructureType::Monument);
    }
    auto outpost = start->compoundTag("pillager_outpost");
    if (!outpost) {
      outpost = start->compoundTag("Pillager_Outpost");
    }
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
    if (!wd.fError && fError) {
      wd.fError = fError;
    }
    wd.fMaxChunkLastUpdate = std::max(wd.fMaxChunkLastUpdate, fMaxChunkLastUpdate);
  }

  void drain(WorldData &out) {
    using namespace std;
    assert(fDim == out.fDim);
    if (fDim != out.fDim) {
      return;
    }
    fPortalBlocks.drain(out.fPortalBlocks);

    for (auto const &it : fMapItems) {
      out.fMapItems[it.first] = it.second;
    }
    unordered_map<int32_t, shared_ptr<CompoundTag>>().swap(fMapItems);

    for (auto const &it : fAutonomousEntities) {
      out.fAutonomousEntities.push_back(it);
    }
    vector<shared_ptr<CompoundTag>>().swap(fAutonomousEntities);

    for (auto const &pos : fEndPortalsInEndDimension) {
      out.fEndPortalsInEndDimension.insert(pos);
    }
    unordered_set<Pos3i, Pos3iHasher>().swap(fEndPortalsInEndDimension);

    for (auto const &piece : fStructures) {
      out.fStructures.add(piece);
    }
    if (!out.fError && fError) {
      out.fError = fError;
    }
    out.fMaxChunkLastUpdate = std::max(out.fMaxChunkLastUpdate, fMaxChunkLastUpdate);
  }

  std::shared_ptr<CompoundTag> toNbt() const {
    using namespace std;
    using namespace mcfile;
    auto tag = Compound();
    CompoundTag &c = *tag;
    c["dim"] = Byte(static_cast<uint8_t>(fDim));
    c["maxChunkLastUpdate"] = Long(fMaxChunkLastUpdate);
    c["portalBlocks"] = fPortalBlocks.toNbt();

    auto mapItems = List<Tag::Type::Compound>();
    for (auto it : fMapItems) {
      int number = it.first;
      auto tag = it.second;
      auto c = Compound();
      (*c)["number"] = Int(number);
      (*c)["tag"] = tag->clone();
      mapItems->push_back(c);
    }
    c["mapItems"] = mapItems;

    auto autonomousEntities = List<Tag::Type::Compound>();
    for (auto it : fAutonomousEntities) {
      autonomousEntities->push_back(it->clone());
    }
    c["autonomousEntities"] = autonomousEntities;

    auto endPortalsInEndDimension = List<Tag::Type::Compound>();
    for (Pos3i const &pos : fEndPortalsInEndDimension) {
      endPortalsInEndDimension->push_back(Pos3iToNbt(pos));
    }
    c["endPortalsInEndDimension"] = endPortalsInEndDimension;

    c["structures"] = fStructures.toNbt();

    return tag;
  }

  static std::shared_ptr<WorldData> FromNbt(CompoundTag const &tag) {
    using namespace std;
    using namespace mcfile;
    auto dim = tag.byte("dim");
    if (!dim) {
      return nullptr;
    }
    auto ret = make_shared<WorldData>(static_cast<Dimension>(*dim));

    auto maxChunkLastUpdate = tag.int64("maxChunkLastUpdate");
    if (!maxChunkLastUpdate) {
      return nullptr;
    }
    ret->fMaxChunkLastUpdate = *maxChunkLastUpdate;

    auto portalBlocksTag = tag.tag("portalBlocks");
    if (!portalBlocksTag) {
      return nullptr;
    }
    auto portalBlocks = PortalBlocks::FromNbt(*portalBlocksTag);
    if (!portalBlocks) {
      return nullptr;
    }
    ret->fPortalBlocks = *portalBlocks;

    auto mapItemsTag = tag.listTag("mapItems");
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

    auto autonomousEntitiesTag = tag.listTag("autonomousEntities");
    if (!autonomousEntitiesTag) {
      return nullptr;
    }
    for (auto const &it : *autonomousEntitiesTag) {
      auto c = it->asCompound();
      ret->fAutonomousEntities.push_back(c->copy());
    }

    auto endPortalsInEndDimensionTag = tag.listTag("endPortalsInEndDimension");
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

    auto structuresTag = tag.tag("structures");
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
  std::unordered_map<int32_t, CompoundTagPtr> fMapItems;
  std::vector<CompoundTagPtr> fAutonomousEntities;
  std::unordered_set<Pos3i, Pos3iHasher> fEndPortalsInEndDimension;
  StructurePieceCollection fStructures;
  std::optional<Status::ErrorData> fError;
  int64_t fMaxChunkLastUpdate = 0;
};

} // namespace je2be::tobe
