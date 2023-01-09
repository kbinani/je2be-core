#include "toje/_world.hpp"

#include <je2be/defer.hpp>

#include "_data2d.hpp"
#include "_file.hpp"
#include "_parallel.hpp"
#include "_props.hpp"
#include "_queue2d.hpp"
#include "terraform/_leaves.hpp"
#include "terraform/java/_block-accessor-java-directory.hpp"
#include "toje/_region.hpp"

using namespace std;
namespace fs = std::filesystem;

namespace je2be::toje {

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
                        std::atomic_uint64_t &numConvertedChunks) {
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

    atomic_bool ok = true;
    auto ctx = Parallel::Reduce<pair<Pos2i, Context::ChunksInRegion>, shared_ptr<Context>>(
        regions,
        concurrency,
        [&parentContext]() { return parentContext.make(); },
        [d, &db, dir, &parentContext, reportProgress, &ok, &numConvertedChunks, concurrency](pair<Pos2i, Context::ChunksInRegion> const &work) -> shared_ptr<Context> {
          auto ctx = parentContext.make();
          if (!ok) {
            return ctx;
          }
          Pos2i region = work.first;
          auto result = Region::Convert(d, work.second.fChunks, region, concurrency, &db, dir, *ctx, reportProgress, numConvertedChunks);
          if (result) {
            return result;
          } else {
            ok.store(false);
            return ctx;
          }
        },
        [](shared_ptr<Context> const &from, shared_ptr<Context> to) -> void {
          from->mergeInto(*to);
        });

    if (cancelRequested.load() || !ok) {
      return JE2BE_ERROR;
    }

    if (auto st = TerraformRegionBoundaries(dir / "region", regions, concurrency); !st.ok()) {
      return st;
    }

    unordered_map<Pos2i, unordered_set<Pos2i, Pos2iHasher>, Pos2iHasher> chunksInRegion;

    unordered_map<Pos2i, unordered_map<Uuid, int64_t, UuidHasher, UuidPred>, Pos2iHasher> leashedEntities;
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
      unordered_set<Pos2i, Pos2iHasher> const &chunks = it.second;
      for (Pos2i const &chunk : chunks) {
        if (auto st = AttachLeashAndPassengers(chunk, entitiesDir, *editor, *ctx, leashedEntities, vehicleEntities); !st.ok()) {
          return st;
        }
      }
    }

    resultContext.swap(ctx);
    return Status::Ok();
  }

  static Status TerraformRegionBoundary(int cx, int cz, mcfile::je::McaEditor &editor, fs::path directory, std::shared_ptr<terraform::java::BlockAccessorJavaDirectory<3, 3>> &blockAccessor) {
    int rx = mcfile::Coordinate::RegionFromChunk(cx);
    int rz = mcfile::Coordinate::RegionFromChunk(cz);

    int x = cx - rx * 32;
    int z = cz - rz * 32;

    auto current = editor.extract(x, z);
    if (!current) {
      return Status::Ok();
    }

    if (!blockAccessor) {
      blockAccessor.reset(new terraform::java::BlockAccessorJavaDirectory<3, 3>(cx - 1, cz - 1, directory));
    }
    if (blockAccessor->fChunkX != cx - 1 || blockAccessor->fChunkZ != cz - 1) {
      auto next = blockAccessor->makeRelocated(cx - 1, cz - 1);
      blockAccessor.reset(next);
    }
    blockAccessor->loadAllWith(editor, mcfile::Coordinate::RegionFromChunk(cx), mcfile::Coordinate::RegionFromChunk(cz));

    auto copy = current->copy();
    auto ch = mcfile::je::Chunk::MakeChunk(cx, cz, current);
    if (!ch) {
      return JE2BE_ERROR;
    }
    blockAccessor->set(cx, cz, ch);

    auto writable = mcfile::je::WritableChunk::MakeChunk(cx, cz, copy);
    if (!writable) {
      return JE2BE_ERROR;
    }

    terraform::BlockPropertyAccessorJava propertyAccessor(*ch);
    terraform::Leaves::Do(*writable, *blockAccessor, propertyAccessor);

    auto tag = writable->toCompoundTag();
    if (!tag) {
      return JE2BE_ERROR;
    }
    if (!editor.insert(x, z, *tag)) {
      return JE2BE_ERROR;
    }

    return Status::Ok();
  }

  static Status TerraformRegionBoundaries(fs::path const &directory, std::vector<std::pair<Pos2i, Context::ChunksInRegion>> const &regions, unsigned int concurrency) {
    if (regions.empty()) {
      return Status::Ok();
    }

    optional<Pos2i> min;
    optional<Pos2i> max;
    for (auto const &it : regions) {
      Pos2i const &region = it.first;
      if (min && max) {
        int minRx = (std::min)(min->fX, region.fX);
        int maxRx = (std::max)(max->fX, region.fX);
        int minRz = (std::min)(min->fZ, region.fZ);
        int maxRz = (std::max)(max->fZ, region.fZ);
        min = Pos2i(minRx, minRz);
        max = Pos2i(maxRx, maxRz);
      } else {
        min = region;
        max = region;
      }
    }
    assert(min && max);

    int width = max->fX - min->fX + 1;
    int height = max->fZ - min->fZ + 1;
    Queue2d queue(*min, width, height, 1);
    for (int x = min->fX; x <= max->fX; x++) {
      for (int z = min->fZ; z <= max->fZ; z++) {
        queue.unsafeSetDone({x, z}, true);
      }
    }
    for (auto const &it : regions) {
      Pos2i const &region = it.first;
      queue.unsafeSetDone(region, false);
    }

    int numThreads = concurrency - 1;
    std::latch latch(concurrency);
    atomic_bool ok(true);

    auto action = [&latch, &queue, min, max, directory, &ok, &regions]() {
      shared_ptr<terraform::java::BlockAccessorJavaDirectory<3, 3>> blockAccessor;

      while (ok) {
        optional<Pos2i> region = queue.next();
        if (!region) {
          break;
        }

        auto mca = directory / mcfile::je::Region::GetDefaultRegionFileName(region->fX, region->fZ);
        auto editor = mcfile::je::McaEditor::Open(mca);
        if (!editor) {
          ok = false;
          break;
        }

        // north, south
        for (int z : {0, 1, 30, 31}) {
          for (int x = 0; x < 32; x++) {
            int cx = x + region->fX * 32;
            int cz = z + region->fZ * 32;
            if (auto st = TerraformRegionBoundary(cx, cz, *editor, directory, blockAccessor); !st.ok()) {
              ok = false;
              break;
            }
          }
          if (!ok) {
            break;
          }
        }

        // west, east
        for (int x : {0, 1, 30, 31}) {
          for (int z = 2; z < 30; z++) {
            int cx = x + region->fX * 32;
            int cz = z + region->fZ * 32;
            if (auto st = TerraformRegionBoundary(cx, cz, *editor, directory, blockAccessor); !st.ok()) {
              ok = false;
              break;
            }
          }
          if (!ok) {
            break;
          }
        }

        if (!editor->write(mca)) {
          ok = false;
        }

        queue.finish(*region);
      }
      latch.count_down();
    };

    vector<thread> threads;
    for (int i = 0; i < numThreads; i++) {
      threads.push_back(thread(action));
    }
    action();
    latch.wait();
    for (auto &th : threads) {
      th.join();
    }
    return Status::Ok();
  }

  static Status AttachLeashAndPassengers(
      Pos2i chunk,
      std::filesystem::path const &entitiesDir,
      mcfile::je::McaEditor &editor,
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
    int localX = chunk.fX - rx * 32;
    int localZ = chunk.fZ - rz * 32;

    auto content = editor.extract(localX, localZ);
    if (!content) {
      return JE2BE_ERROR;
    }
    auto entities = content->listTag("Entities");
    if (!entities) {
      return JE2BE_ERROR;
    }
    AttachLeash(chunk, ctx, *entities, leashedEntities);
    if (auto st = AttachPassengers(chunk, ctx, entities, entitiesDir, editor, vehicleEntities); !st.ok()) {
      return st;
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
          fs::path nbt = entitiesDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
          CompoundTagPtr tag;
          if (mcfile::je::McaEditor::Load(nbt, cx - rx * 32, cz - rz * 32, tag); tag) {
            entitiesInChunk = tag->listTag("Entities");
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
          data->set("Entities", entitiesInChunk);
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

  static CompoundTagPtr FindEntity(ListTag const &entities, Uuid const &entityId) {
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
};

Status World::Convert(mcfile::Dimension d,
                      std::vector<std::pair<Pos2i, Context::ChunksInRegion>> const &regions,
                      mcfile::be::DbInterface &db,
                      std::filesystem::path root,
                      unsigned concurrency,
                      Context const &parentContext,
                      std::shared_ptr<Context> &resultContext,
                      std::function<bool(void)> progress,
                      std::atomic_uint64_t &numConvertedChunks) {
  return Impl::Convert(d, regions, db, root, concurrency, parentContext, resultContext, progress, numConvertedChunks);
}

} // namespace je2be::toje
