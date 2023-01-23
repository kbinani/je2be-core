#pragma once

#include "structure/_structure-piece.hpp"
#include "tobe/_level-data.hpp"
#include "tobe/structure/_structure-piece-collection.hpp"

namespace je2be::tobe {

class WorldData {
public:
  explicit WorldData(mcfile::Dimension dim) : fDim(dim) {}

  void addError(Status::ErrorData error) {
    if (!fError) {
      fError = error;
    }
  }

  void addPortalBlock(i32 x, i32 y, i32 z, bool xAxis) { fPortalBlocks.add(x, y, z, xAxis); }

  void addMap(i32 javaMapId, CompoundTagPtr const &item) { fMapItems[javaMapId] = item; }

  void addAutonomousEntity(CompoundTagPtr const &entity) { fAutonomousEntities.push_back(entity); }

  void addEndPortal(i32 x, i32 y, i32 z) {
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
    if (start->empty()) {
      return;
    }
    if (auto fortress = Get(*start, "fortress"); fortress) {
      addStructures(*fortress, StructureType::Fortress);
    }
    if (auto monument = Get(*start, "monument"); monument) {
      addStructures(*monument, StructureType::Monument);
    }
    if (auto outpost = Get(*start, "pillager_outpost"); outpost) {
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
    unordered_map<i32, shared_ptr<CompoundTag>>().swap(fMapItems);

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
    for (auto const &it : fEntityFiles) {
      copy(it.second.begin(), it.second.end(), back_inserter(out.fEntityFiles[it.first]));
    }
    unordered_map<Pos2i, vector<std::filesystem::path>, Pos2iHasher>().swap(fEntityFiles);
  }

  void addEntityFile(Pos2i chunk, std::filesystem::path const &file) {
    fEntityFiles[chunk].push_back(file);
  }

  void drainEntityFiles(std::unordered_map<Pos2i, std::vector<std::filesystem::path>, Pos2iHasher> &out) {
    out.swap(fEntityFiles);
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

  static CompoundTagPtr Get(CompoundTag const &src, std::string const &name) {
    if (auto ret = src.compoundTag("minecraft:" + name); ret) {
      return ret;
    }
    if (auto ret = src.compoundTag(name); ret) {
      return ret;
    }
    return src.compoundTag(strings::CapitalizeSnake(name));
  }

public:
  mcfile::Dimension const fDim;

private:
  PortalBlocks fPortalBlocks;
  std::unordered_map<i32, CompoundTagPtr> fMapItems;
  std::vector<CompoundTagPtr> fAutonomousEntities;
  std::unordered_set<Pos3i, Pos3iHasher> fEndPortalsInEndDimension;
  StructurePieceCollection fStructures;
  std::optional<Status::ErrorData> fError;
  i64 fMaxChunkLastUpdate = 0;
  std::unordered_map<Pos2i, std::vector<std::filesystem::path>, Pos2iHasher> fEntityFiles;
};

} // namespace je2be::tobe
