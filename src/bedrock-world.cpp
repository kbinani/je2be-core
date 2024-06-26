#include "bedrock/_world.hpp"

#include <defer.hpp>

#include "_file.hpp"
#include "_parallel.hpp"
#include "_props.hpp"
#include "bedrock/_region.hpp"

using namespace std;
namespace fs = std::filesystem;

namespace je2be::bedrock {

class World::Impl {
  Impl() = delete;

public:
  static Status Convert(mcfile::Dimension d,
                        std::vector<std::pair<Pos2i, Context::ChunksInRegion>> const &regions,
                        mcfile::be::DbInterface &db,
                        std::filesystem::path root,
                        unsigned concurrency,
                        Context const &parentContext,
                        std::shared_ptr<Context> &resultContext,
                        std::function<bool(void)> progress,
                        std::atomic_uint64_t &numConvertedChunks,
                        std::filesystem::path terrainTempDir) {
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
      return JE2BE_ERROR;
    }

    error_code ec;
    fs::create_directories(dir / "region", ec);
    if (ec) {
      return JE2BE_ERROR_WHAT(ec.message());
    }
    fs::create_directories(dir / "entities", ec);
    if (ec) {
      return JE2BE_ERROR_WHAT(ec.message());
    }

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

    auto [ctx, status] = Parallel::Reduce<pair<Pos2i, Context::ChunksInRegion>, shared_ptr<Context>>(
        regions,
        concurrency,
        [&parentContext]() { return parentContext.make(); },
        [d, &db, dir, &parentContext, reportProgress, &numConvertedChunks, concurrency, terrainTempDir](pair<Pos2i, Context::ChunksInRegion> const &work) -> pair<shared_ptr<Context>, Status> {
          auto ctx = parentContext.make();
          Pos2i region = work.first;
          shared_ptr<Context> result;
          if (auto st = Region::Convert(d, work.second.fChunks, region, concurrency, &db, dir, *ctx, reportProgress, numConvertedChunks, terrainTempDir, result); !st.ok()) {
            return make_pair(ctx, JE2BE_ERROR_PUSH(st));
          } else if (result) {
            return make_pair(result, Status::Ok());
          } else {
            return make_pair(ctx, JE2BE_ERROR);
          }
        },
        [](shared_ptr<Context> const &from, shared_ptr<Context> to) -> void {
          from->mergeInto(*to);
        });
    if (!status.ok()) {
      return JE2BE_ERROR_PUSH(status);
    }
    if (cancelRequested.load()) {
      return JE2BE_ERROR;
    }

    unordered_map<Pos2i, Pos2iSet, Pos2iHasher> chunksInRegion;

    unordered_map<Pos2i, unordered_map<Uuid, i64, UuidHasher, UuidPred>, Pos2iHasher> leashedEntities;
    for (auto const &it : ctx->fLeashedEntities) {
      leashedEntities[it.second.fChunk][it.first] = it.second.fLeasherId;
      int rx = mcfile::Coordinate::RegionFromChunk(it.second.fChunk.fX);
      int rz = mcfile::Coordinate::RegionFromChunk(it.second.fChunk.fZ);
      chunksInRegion[{rx, rz}].insert(it.second.fChunk);
    }

    unordered_map<Pos2i, unordered_map<Uuid, std::map<size_t, Uuid>, UuidHasher, UuidPred>, Pos2iHasher> vehicleEntities;
    for (auto const &it : ctx->fVehicleEntities) {
      if (!it.second.fPassengers.empty()) {
        vehicleEntities[it.second.fChunk][it.first] = it.second.fPassengers;
        int rx = mcfile::Coordinate::RegionFromChunk(it.second.fChunk.fX);
        int rz = mcfile::Coordinate::RegionFromChunk(it.second.fChunk.fZ);
        chunksInRegion[{rx, rz}].insert(it.second.fChunk);
      }
    }

    for (auto const &it : chunksInRegion) {
      Pos2i r = it.first;
      fs::path entitiesDir = dir / "entities";
      fs::path entitiesMca = entitiesDir / mcfile::je::Region::GetDefaultRegionFileName(r.fX, r.fZ);
      auto editor = mcfile::je::McaEditor::Open(entitiesMca);
      if (!editor) {
        return JE2BE_ERROR;
      }
      Pos2iSet const &chunks = it.second;
      for (Pos2i const &chunk : chunks) {
        if (auto st = AttachLeashAndPassengers(chunk, entitiesDir, *editor, *ctx, leashedEntities, vehicleEntities); !st.ok()) {
          return JE2BE_ERROR_PUSH(st);
        }
      }
    }

    resultContext.swap(ctx);
    return Status::Ok();
  }

  static Status AttachLeashAndPassengers(
      Pos2i chunk,
      std::filesystem::path const &entitiesDir,
      mcfile::je::McaEditor &editor,
      Context &ctx,
      std::unordered_map<Pos2i, std::unordered_map<Uuid, i64, UuidHasher, UuidPred>, Pos2iHasher> const &leashedEntities,
      std::unordered_map<Pos2i, std::unordered_map<Uuid, std::map<size_t, Uuid>, UuidHasher, UuidPred>, Pos2iHasher> const &vehicleEntities) {
    using namespace std;
    using namespace mcfile::stream;
    namespace fs = std::filesystem;
    int cx = chunk.fX;
    int cz = chunk.fZ;
    int rx = mcfile::Coordinate::RegionFromChunk(cx);
    int rz = mcfile::Coordinate::RegionFromChunk(cz);
    int localX = chunk.fX - rx * 32;
    int localZ = chunk.fZ - rz * 32;

    auto content = editor.extract(localX, localZ);
    if (!content) {
      return JE2BE_ERROR;
    }
    auto dataVersion = content->int32(u8"DataVersion");
    if (!dataVersion) {
      return JE2BE_ERROR;
    }
    auto entities = content->listTag(u8"Entities");
    if (!entities) {
      return JE2BE_ERROR;
    }
    AttachLeash(chunk, *dataVersion, ctx, *entities, leashedEntities);
    if (auto st = AttachPassengers(chunk, ctx, entities, entitiesDir, editor, vehicleEntities); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }

    if (editor.insert(localX, localZ, *content)) {
      return Status::Ok();
    } else {
      return JE2BE_ERROR;
    }
  }

  static Status AttachPassengers(
      Pos2i chunk,
      Context &ctx,
      ListTagPtr const &entities,
      std::filesystem::path const &entitiesDir,
      mcfile::je::McaEditor &editor,
      std::unordered_map<Pos2i, std::unordered_map<Uuid, std::map<size_t, Uuid>, UuidHasher, UuidPred>, Pos2iHasher> const &vehicleEntities) {
    using namespace std;
    namespace fs = std::filesystem;
    auto vehicleEntitiesFound = vehicleEntities.find(chunk);
    if (vehicleEntitiesFound == vehicleEntities.end()) {
      return Status::Ok();
    }
    for (auto const &vehicleEntityIt : vehicleEntitiesFound->second) {
      Uuid vehicleId = vehicleEntityIt.first;
      map<size_t, Uuid> const &passengers = vehicleEntityIt.second;

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
          fs::path nbt = entitiesDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
          CompoundTagPtr tag;
          if (mcfile::je::McaEditor::Load(nbt, cx - rx * 32, cz - rz * 32, tag); tag) {
            entitiesInChunk = tag->listTag(u8"Entities");
          }
        }
        if (!entitiesInChunk) {
          continue;
        }
        for (Uuid const &passengerId : j.second) {
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
          data->set(u8"Entities", entitiesInChunk);
          int cx = passengerChunk.fX;
          int cz = passengerChunk.fZ;
          int rx = mcfile::Coordinate::RegionFromChunk(cx);
          int rz = mcfile::Coordinate::RegionFromChunk(cz);
          fs::path nbt = entitiesDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
          auto secondaryEditor = mcfile::je::McaEditor::Open(nbt);
          if (!secondaryEditor) {
            return JE2BE_ERROR;
          }
          int localX = cx - 32 * rx;
          int localZ = cz - 32 * rz;
          secondaryEditor->extract(localX, localZ);
          if (!secondaryEditor->insert(localX, localZ, *data)) {
            return JE2BE_ERROR;
          }
        }
      }

      // Attach passengers to vehicle
      shared_ptr<ListTag> passengersTag = vehicle->listTag(u8"Passengers");
      if (!passengersTag) {
        passengersTag = List<Tag::Type::Compound>();
        vehicle->set(u8"Passengers", passengersTag);
      }
      if (vehicle->string(u8"id") == u8"minecraft:chicken") {
        vehicle->set(u8"IsChickenJockey", Bool(true));
      }
      for (auto const &it : collectedPassengers) {
        size_t index = it.first;
        auto passenger = it.second;
        if (index >= passengersTag->size()) {
          for (size_t i = passengersTag->size(); i < index + 1; i++) {
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
    return Status::Ok();
  }

  static void AttachLeash(Pos2i chunk,
                          int dataVersion,
                          Context &ctx,
                          ListTag const &entities,
                          std::unordered_map<Pos2i, std::unordered_map<Uuid, i64, UuidHasher, UuidPred>, Pos2iHasher> const &leashedEntities) {
    auto leashedEntitiesFound = leashedEntities.find(chunk);
    if (leashedEntitiesFound == leashedEntities.end()) {
      return;
    }
    for (auto const &it : leashedEntitiesFound->second) {
      Uuid entityId = it.first;
      i64 leasherId = it.second;
      auto leasherFound = ctx.fLeashKnots.find(leasherId);
      if (leasherFound == ctx.fLeashKnots.end()) {
        continue;
      }
      auto entity = FindEntity(entities, entityId);
      if (entity) {
        Pos3i leashPos = leasherFound->second;
        if (dataVersion >= kJavaDataVersionComponentIntroduced) {
          entity->set(u8"leash", IntArrayFromPos3i(leashPos));
        } else {
          auto leashTag = Compound();
          leashTag->set(u8"X", Int(leashPos.fX));
          leashTag->set(u8"Y", Int(leashPos.fY));
          leashTag->set(u8"Z", Int(leashPos.fZ));
          entity->set(u8"Leash", leashTag);
        }
      }
    }
  }

  static CompoundTagPtr FindEntity(ListTag const &entities, Uuid const &entityId) {
    for (auto &entity : entities) {
      auto entityCompound = std::dynamic_pointer_cast<CompoundTag>(entity);
      if (!entityCompound) {
        continue;
      }
      auto uuid = props::GetUuidWithFormatIntArray(*entityCompound, u8"UUID");
      if (!uuid) {
        continue;
      }
      if (UuidPred{}(entityId, *uuid)) {
        return entityCompound;
      }
    }
    return nullptr;
  }
};

Status World::Convert(mcfile::Dimension d,
                      std::vector<std::pair<Pos2i, Context::ChunksInRegion>> const &regions,
                      mcfile::be::DbInterface &db,
                      std::filesystem::path root,
                      unsigned concurrency,
                      Context const &parentContext,
                      std::shared_ptr<Context> &resultContext,
                      std::function<bool(void)> progress,
                      std::atomic_uint64_t &numConvertedChunks,
                      std::filesystem::path terrainTempDir) {
  return Impl::Convert(d, regions, db, root, concurrency, parentContext, resultContext, progress, numConvertedChunks, terrainTempDir);
}

} // namespace je2be::bedrock
