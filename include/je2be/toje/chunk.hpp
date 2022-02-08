#pragma once

namespace je2be::toje {

class Chunk {
  Chunk() = delete;

public:
  static std::shared_ptr<mcfile::je::WritableChunk> Convert(mcfile::Dimension d, int cx, int cz, mcfile::be::Chunk const &b, ChunkCache<3, 3> &cache, Context &ctx) {
    using namespace std;
    auto j = mcfile::je::WritableChunk::MakeEmpty(cx, cz);

    for (auto const &sectionB : b.fSubChunks) {
      if (!sectionB) {
        continue;
      }

      auto sectionJ = SubChunk::Convert(*sectionB);
      if (!sectionJ) {
        return nullptr;
      }

      int sectionIndex = sectionJ->fY - j->fChunkY;
      if (j->fSections.size() <= sectionIndex) {
        j->fSections.resize(sectionIndex + 1);
      }
      j->fSections[sectionIndex] = sectionJ;
    }

    for (int y = j->minBlockY(); y <= j->maxBlockY(); y += 4) {
      for (int z = j->minBlockZ(); z <= j->maxBlockZ(); z += 4) {
        for (int x = j->minBlockX(); x <= j->maxBlockX(); x += 4) {
          auto biome = b.biomeAt(x, y, z);
          if (biome) {
            j->setBiomeAt(x, y, z, *biome);
          }
        }
      }
    }

    for (auto const &it : b.fBlockEntities) {
      Pos3i const &pos = it.first;
      shared_ptr<CompoundTag> const &tagB = it.second;
      assert(tagB);
      if (!tagB) {
        continue;
      }
      auto const &blockB = b.blockAt(pos);
      if (!blockB) {
        continue;
      }
      auto const &blockJ = j->blockAt(pos);
      if (!blockJ) {
        continue;
      }
      auto result = BlockEntity::FromBlockAndBlockEntity(pos, *blockB, *tagB, *blockJ, ctx);
      if (!result) {
        continue;
      }
      if (result->fTileEntity) {
        j->fTileEntities[pos] = result->fTileEntity;
      }
      if (result->fBlock) {
        mcfile::je::SetBlockOptions o;
        o.fRemoveTileEntity = false;
        j->setBlockAt(pos, result->fBlock, o);
      }
    }

    BlockPropertyAccessor accessor(b);

    Piston::Do(*j, cache, accessor);

    ShapeOfStairs::Do(*j, cache, accessor);
    Kelp::Do(*j, cache, accessor);
    TwistingVines::Do(*j, cache, accessor);
    WeepingVines::Do(*j, cache, accessor);
    AttachedStem::Do(*j, cache, accessor);
    CaveVines::Do(*j, cache, accessor);
    Snowy::Do(*j, cache, accessor);
    ChorusPlant::Do(*j, cache, accessor);
    FenceConnectable::Do(*j, cache, accessor);
    Campfire::Do(*j, cache, accessor);
    NoteBlock::Do(*j, cache, accessor);
    RedstoneWire::Do(*j, cache, accessor);
    Tripwire::Do(*j, cache, accessor);
    Beacon::Do(*j, cache, accessor);

    for (auto const &it : b.fBlockEntities) {
      auto id = it.second->string("id");
      if (!id) {
        continue;
      }
      if (id != "ItemFrame" && id != "GlowItemFrame") {
        continue;
      }
      Pos3i pos = it.first;
      auto blockB = cache.blockAt(pos.fX, pos.fY, pos.fZ);
      if (!blockB) {
        continue;
      }
      auto frameJ = Entity::ItemFrameFromBedrock(d, pos, *blockB, *it.second, ctx);
      if (frameJ) {
        j->fEntities.push_back(frameJ);
      }
    }

    unordered_map<Uuid, shared_ptr<CompoundTag>, UuidHasher, UuidPred> entities;
    for (auto const &entityB : b.fEntities) {
      auto result = Entity::From(*entityB, ctx);
      if (result) {
        entities[result->fUuid] = result->fEntity;
      } else if (entityB->string("identifier") == "minecraft:leash_knot") {
        auto id = entityB->int64("UniqueID");
        auto posf = props::GetPos3f(*entityB, "Pos");
        if (id && posf) {
          int x = (int)roundf(posf->fX - 0.5f);
          int y = (int)roundf(posf->fY - 0.25f);
          int z = (int)roundf(posf->fZ - 0.5f);
          Pos3i posi(x, y, z);
          ctx.fLeashKnots[*id] = posi;
        }
      }
    }

    AttachPassengers(ctx, entities);
    AttachLeash(ctx, entities);

    for (auto const &it : entities) {
      j->fEntities.push_back(it.second);
    }

    return j;
  }

  static void AttachLeash(Context &ctx, std::unordered_map<Uuid, std::shared_ptr<CompoundTag>, UuidHasher, UuidPred> &entities) {
    using namespace std;
    unordered_set<Uuid, UuidHasher, UuidPred> resolvedLeashedEntities;
    for (auto &it : ctx.fLeashedEntities) {
      Uuid leashedEntityUuid = it.first;
      int64_t leasherId = it.second;
      auto foundLeashedEntity = entities.find(leashedEntityUuid);
      if (foundLeashedEntity == entities.end()) {
        continue;
      }
      shared_ptr<CompoundTag> const &leashedEntity = foundLeashedEntity->second;
      auto foundLeash = ctx.fLeashKnots.find(leasherId);
      if (foundLeash == ctx.fLeashKnots.end()) {
        continue;
      }
      Pos3i leashPos = foundLeash->second;
      auto leashTag = make_shared<CompoundTag>();
      leashTag->set("X", props::Int(leashPos.fX));
      leashTag->set("Y", props::Int(leashPos.fY));
      leashTag->set("Z", props::Int(leashPos.fZ));
      leashedEntity->set("Leash", leashTag);

      ctx.fLeashKnots.erase(leasherId);
      resolvedLeashedEntities.insert(leashedEntityUuid);
    }
    for (Uuid resolvedLeashedEntity : resolvedLeashedEntities) {
      ctx.fLeashedEntities.erase(resolvedLeashedEntity);
    }
  }

  static void AttachPassengers(Context &ctx, std::unordered_map<Uuid, std::shared_ptr<CompoundTag>, UuidHasher, UuidPred> &entities) {
    using namespace std;
    unordered_set<Uuid, UuidHasher, UuidPred> resolvedVehicleEntities;
    for (auto &it : ctx.fVehicleEntities) {
      Uuid vehicleUuid = it.first;
      std::map<size_t, Uuid> &passengers = it.second;
      auto found = entities.find(vehicleUuid);
      if (found == entities.end()) {
        continue;
      }
      shared_ptr<CompoundTag> vehicle = found->second;
      shared_ptr<ListTag> passengersTag = vehicle->listTag("Passengers");
      if (!passengersTag) {
        passengersTag = make_shared<ListTag>(Tag::Type::Compound);
        vehicle->set("Passengers", passengersTag);
      }
      unordered_set<size_t> resolvedPassengers;
      for (auto const &passenger : passengers) {
        size_t passengerIndex = passenger.first;
        Uuid passengerUuid = passenger.second;
        auto f = entities.find(passengerUuid);
        if (f == entities.end()) {
          continue;
        }
        auto p = f->second;
        passengersTag->push_back(p);
        entities.erase(f);
        resolvedPassengers.insert(passengerIndex);
      }
      for (size_t resolvedPassenger : resolvedPassengers) {
        passengers.erase(resolvedPassenger);
      }
      if (passengers.empty()) {
        resolvedVehicleEntities.insert(vehicleUuid);
      }
    }
    for (Uuid resolvedVehicleEntity : resolvedVehicleEntities) {
      ctx.fVehicleEntities.erase(resolvedVehicleEntity);
    }
  }
};

} // namespace je2be::toje
