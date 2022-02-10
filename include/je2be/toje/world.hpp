#pragma once

namespace je2be::toje {

class World {
public:
  static std::shared_ptr<Context> Convert(mcfile::Dimension d, leveldb::DB &db, std::filesystem::path root, unsigned concurrency, Context const &parentContext) {
    using namespace std;
    using namespace mcfile;
    namespace fs = std::filesystem;

    fs::path dir;
    switch (d) {
    case Dimension::Overworld:
      dir = root;
      break;
    case Dimension::Nether:
      dir = root / "DIM-1";
      break;
    case Dimension::End:
      dir = root / "DIM1";
      break;
    default:
      return nullptr;
    }

    error_code ec;
    fs::create_directories(dir / "region", ec);
    if (ec) {
      return nullptr;
    }
    fs::create_directories(dir / "entities", ec);
    if (ec) {
      return nullptr;
    }

    unordered_map<Pos2i, je2be::toje::Region, Pos2iHasher> regions;
    mcfile::be::Chunk::ForAll(&db, d, [&regions](int cx, int cz) {
      int rx = Coordinate::RegionFromChunk(cx);
      int rz = Coordinate::RegionFromChunk(cz);
      if (fabs(cx) > 1 || fabs(cz) > 1) {
        return;
      }
      Pos2i c(cx, cz);
      Pos2i r(rx, rz);
      regions[r].fChunks.insert(c);
    });

    hwm::task_queue queue(concurrency);
    deque<future<shared_ptr<Context>>> futures;
    auto ctx = parentContext.make();

    bool ok = true;
    for (auto const &region : regions) {
      Pos2i r = region.first;
      vector<future<shared_ptr<Context>>> drain;
      FutureSupport::Drain(concurrency + 1, futures, drain);
      for (auto &d : drain) {
        auto result = d.get();
        if (result) {
          result->mergeInto(*ctx);
        } else {
          ok = false;
        }
      }
      if (!ok) {
        break;
      }
      futures.emplace_back(queue.enqueue(Region::Convert, d, region.second.fChunks, r.fX, r.fZ, &db, dir, *ctx));
    }

    for (auto &f : futures) {
      auto result = f.get();
      if (result) {
        result->mergeInto(*ctx);
      } else {
        ok = false;
      }
    }

    if (!ok) {
      return nullptr;
    }

    unordered_set<Pos2i, Pos2iHasher> chunks;

    unordered_map<Pos2i, unordered_map<Uuid, int64_t, UuidHasher, UuidPred>, Pos2iHasher> leashedEntities;
    for (auto const &it : ctx->fLeashedEntities) {
      leashedEntities[it.second.fChunk][it.first] = it.second.fLeasherId;
      chunks.insert(it.second.fChunk);
    }

    for (Pos2i const &chunk : chunks) {
      AttachLeash(chunk, dir, *ctx, leashedEntities);
    }

    for (auto const &region : regions) {
      int rx = region.first.fX;
      int rz = region.first.fZ;
      fs::path entitiesDir = dir / "entities" / ("r." + to_string(rx) + "." + to_string(rz));
      fs::path entitiesMca = dir / "entities" / mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
      defer {
        error_code ec1;
        fs::remove_all(entitiesDir, ec1);
      };
      if (!mcfile::je::Region::ConcatCompressedNbt(rx, rz, entitiesDir, entitiesMca)) {
        return nullptr;
      }
    }

    return ctx;
  }

  static bool AttachLeash(Pos2i chunk,
                          std::filesystem::path dir,
                          Context &ctx,
                          std::unordered_map<Pos2i, std::unordered_map<Uuid, int64_t, UuidHasher, UuidPred>, Pos2iHasher> const &leashedEntities) {
    using namespace std;
    using namespace mcfile::stream;
    namespace fs = std::filesystem;
    int cx = chunk.fX;
    int cz = chunk.fZ;
    int rx = mcfile::Coordinate::RegionFromChunk(cx);
    int rz = mcfile::Coordinate::RegionFromChunk(cz);
    fs::path entitiesDir = dir / "entities" / ("r." + to_string(rx) + "." + to_string(rz));
    fs::path nbt = entitiesDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);

    vector<uint8_t> buffer;
    if (!je2be::file::GetContents(nbt, buffer)) {
      return false;
    }
    auto rootTag = CompoundTag::ReadCompressed(buffer);
    if (!rootTag) {
      return false;
    }
    auto content = rootTag->compoundTag("");
    if (!content) {
      return false;
    }
    auto entities = content->listTag("Entities");
    if (!entities) {
      return false;
    }
    AttachLeash(chunk, ctx, *entities, leashedEntities);

    return CompoundTag::WriteCompressed(*rootTag, nbt);
  }

  static void AttachLeash(Pos2i chunk,
                          Context &ctx,
                          ListTag const &entities,
                          std::unordered_map<Pos2i, std::unordered_map<Uuid, int64_t, UuidHasher, UuidPred>, Pos2iHasher> const &leashedEntities) {
    auto leashedEntitiesFound = leashedEntities.find(chunk);
    if (leashedEntitiesFound == leashedEntities.end()) {
      return;
    }
    for (auto const &it : leashedEntitiesFound->second) {
      Uuid entityId = it.first;
      int64_t leasherId = it.second;
      auto leasherFound = ctx.fLeashKnots.find(leasherId);
      if (leasherFound == ctx.fLeashKnots.end()) {
        continue;
      }
      auto entity = FindEntity(entities, entityId);
      if (entity) {
        Pos3i leashPos = leasherFound->second;
        auto leashTag = std::make_shared<CompoundTag>();
        leashTag->set("X", props::Int(leashPos.fX));
        leashTag->set("Y", props::Int(leashPos.fY));
        leashTag->set("Z", props::Int(leashPos.fZ));
        entity->set("Leash", leashTag);
      }
    }
  }

  static std::shared_ptr<CompoundTag> FindEntity(ListTag const &entities, Uuid entityId) {
    for (auto &entity : entities) {
      auto entityCompound = std::dynamic_pointer_cast<CompoundTag>(entity);
      if (!entityCompound) {
        continue;
      }
      auto uuid = props::GetUuidWithFormatIntArray(*entityCompound, "UUID");
      if (!uuid) {
        continue;
      }
      if (UuidPred{}(entityId, *uuid)) {
        return entityCompound;
      }
    }
    return nullptr;
  }

private:
  World() = delete;
};

} // namespace je2be::toje
