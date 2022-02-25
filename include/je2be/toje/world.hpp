#pragma once

namespace je2be::toje {

class World {
public:
  static std::shared_ptr<Context> Convert(mcfile::Dimension d,
                                          std::unordered_map<Pos2i, Context::ChunksInRegion, Pos2iHasher> const &regions,
                                          leveldb::DB &db,
                                          std::filesystem::path root,
                                          unsigned concurrency,
                                          Context const &parentContext,
                                          std::function<bool(void)> progress) {
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

    hwm::task_queue queue(concurrency);
    deque<future<shared_ptr<Context>>> futures;
    auto ctx = parentContext.make();

    atomic<bool> cancelRequested = false;
    auto reportProgress = [progress, &cancelRequested]() -> bool {
      if (progress) {
        bool ok = progress();
        if (!ok) {
          cancelRequested = true;
        }
        return ok;
      } else {
        return true;
      }
    };

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
        if (cancelRequested.load() || !ok) {
          break;
        }
      }
      if (cancelRequested.load() || !ok) {
        break;
      }
      futures.emplace_back(queue.enqueue(Region::Convert, d, region.second.fChunks, r.fX, r.fZ, &db, dir, *ctx, reportProgress));
    }

    for (auto &f : futures) {
      auto result = f.get();
      if (result) {
        result->mergeInto(*ctx);
      } else {
        ok = false;
      }
    }

    if (cancelRequested.load() || !ok) {
      return nullptr;
    }

    unordered_set<Pos2i, Pos2iHasher> chunks;

    unordered_map<Pos2i, unordered_map<Uuid, int64_t, UuidHasher, UuidPred>, Pos2iHasher> leashedEntities;
    for (auto const &it : ctx->fLeashedEntities) {
      leashedEntities[it.second.fChunk][it.first] = it.second.fLeasherId;
      chunks.insert(it.second.fChunk);
    }

    unordered_map<Pos2i, unordered_map<Uuid, std::map<size_t, Uuid>, UuidHasher, UuidPred>, Pos2iHasher> vehicleEntities;
    for (auto const &it : ctx->fVehicleEntities) {
      if (!it.second.fPassengers.empty()) {
        vehicleEntities[it.second.fChunk][it.first] = it.second.fPassengers;
        chunks.insert(it.second.fChunk);
      }
    }

    for (Pos2i const &chunk : chunks) {
      AttachLeashAndPassengers(chunk, dir, *ctx, leashedEntities, vehicleEntities);
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

  static bool AttachLeashAndPassengers(Pos2i chunk,
                                       std::filesystem::path dir,
                                       Context &ctx,
                                       std::unordered_map<Pos2i, std::unordered_map<Uuid, int64_t, UuidHasher, UuidPred>, Pos2iHasher> const &leashedEntities,
                                       std::unordered_map<Pos2i, std::unordered_map<Uuid, std::map<size_t, Uuid>, UuidHasher, UuidPred>, Pos2iHasher> const &vehicleEntities) {
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
    auto content = CompoundTag::ReadCompressed(buffer, std::endian::big);
    if (!content) {
      return false;
    }
    auto entities = content->listTag("Entities");
    if (!entities) {
      return false;
    }
    AttachLeash(chunk, ctx, *entities, leashedEntities);
    if (!AttachPassengers(chunk, ctx, entities, dir, vehicleEntities)) {
      return false;
    }

    return CompoundTag::WriteCompressed(*content, nbt, std::endian::big);
  }

  static bool AttachPassengers(Pos2i chunk,
                               Context &ctx,
                               std::shared_ptr<ListTag> const &entities,
                               std::filesystem::path dir,
                               std::unordered_map<Pos2i, std::unordered_map<Uuid, std::map<size_t, Uuid>, UuidHasher, UuidPred>, Pos2iHasher> const &vehicleEntities) {
    using namespace std;
    namespace fs = std::filesystem;
    auto vehicleEntitiesFound = vehicleEntities.find(chunk);
    if (vehicleEntitiesFound == vehicleEntities.end()) {
      return true;
    }
    for (auto const &it : vehicleEntitiesFound->second) {
      Uuid vehicleId = it.first;
      map<size_t, Uuid> const &passengers = it.second;

      auto vehicle = FindEntity(*entities, vehicleId);
      if (!vehicle) {
        continue;
      }

      // Classify passengers per chunk
      unordered_map<Pos2i, vector<Uuid>, Pos2iHasher> passengersInChunk;
      for (auto const &j : passengers) {
        Uuid passengerId = j.second;
        auto passengerFound = ctx.fEntities.find(passengerId);
        if (passengerFound == ctx.fEntities.end()) {
          continue;
        }
        Pos2i passengerChunk = passengerFound->second;
        passengersInChunk[passengerChunk].push_back(passengerId);
      }

      // Collect passenger entities
      map<size_t, shared_ptr<CompoundTag>> collectedPassengers;
      for (auto const &j : passengersInChunk) {
        Pos2i passengerChunk = j.first;
        shared_ptr<ListTag> entitiesInChunk;
        if (chunk == passengerChunk) {
          entitiesInChunk = entities;
        } else {
          int cx = passengerChunk.fX;
          int cz = passengerChunk.fZ;
          int rx = mcfile::Coordinate::RegionFromChunk(cx);
          int rz = mcfile::Coordinate::RegionFromChunk(cz);
          fs::path entitiesDir = dir / "entities" / ("r." + to_string(rx) + "." + to_string(rz));
          fs::path nbt = entitiesDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
          vector<uint8_t> buffer;
          if (file::GetContents(nbt, buffer)) {
            buffer.clear();
            if (auto tag = CompoundTag::ReadCompressed(buffer, std::endian::big); tag) {
              entitiesInChunk = tag->listTag("Entities");
            }
          }
        }
        if (!entitiesInChunk) {
          continue;
        }
        for (Uuid passengerId : j.second) {
          auto passenger = FindEntity(*entitiesInChunk, passengerId);
          if (!passenger) {
            continue;
          }
          entitiesInChunk->fValue.erase(remove(entitiesInChunk->fValue.begin(), entitiesInChunk->fValue.end(), passenger));
          for (auto const &k : passengers) {
            if (UuidPred{}(k.second, passengerId)) {
              collectedPassengers[k.first] = passenger;
              break;
            }
          }
        }
        if (chunk != passengerChunk) {
          auto data = Compound();
          data->set("Entities", entitiesInChunk);
          int cx = passengerChunk.fX;
          int cz = passengerChunk.fZ;
          int rx = mcfile::Coordinate::RegionFromChunk(cx);
          int rz = mcfile::Coordinate::RegionFromChunk(cz);
          fs::path entitiesDir = dir / "entities" / ("r." + to_string(rx) + "." + to_string(rz));
          fs::path nbt = entitiesDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
          if (!CompoundTag::WriteCompressed(*data, nbt, std::endian::big)) {
            return false;
          }
        }
      }

      // Attach passengers to vehicle
      shared_ptr<ListTag> passengersTag = vehicle->listTag("Passengers");
      if (!passengersTag) {
        passengersTag = List<Tag::Type::Compound>();
        vehicle->set("Passengers", passengersTag);
      }
      if (vehicle->string("id") == "minecraft:chicken") {
        vehicle->set("IsChickenJockey", Bool(true));
      }
      for (auto const &it : collectedPassengers) {
        size_t index = it.first;
        auto passenger = it.second;
        if (index >= passengersTag->size()) {
          for (int i = passengersTag->size(); i < index + 1; i++) {
            passengersTag->push_back(Compound());
          }
        }
        passengersTag->fValue[index] = passenger;
      }

      // Remove vehicle from entities if it is a root vehicle
      if (ctx.isRootVehicle(vehicleId)) {
        entities->fValue.erase(remove(entities->fValue.begin(), entities->fValue.end(), vehicle));
        ctx.setRootVehicleEntity(vehicle);
      }
    }
    return true;
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
        auto leashTag = Compound();
        leashTag->set("X", Int(leashPos.fX));
        leashTag->set("Y", Int(leashPos.fY));
        leashTag->set("Z", Int(leashPos.fZ));
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
