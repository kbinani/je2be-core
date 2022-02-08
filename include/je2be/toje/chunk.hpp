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
        if (id) {
          ctx.fLeashKnots.insert(make_pair(*id, entityB));
        }
      }
    }
    for (auto const &it : ctx.fPassengers) {
      Uuid vehicleUuid = it.first;
      auto found = entities.find(vehicleUuid);
      if (found == entities.end()) {
        continue;
      }
      shared_ptr<CompoundTag> vehicle = found->second;
      auto passengers = make_shared<ListTag>(Tag::Type::Compound);
      for (auto const &passenger : it.second) {
        size_t passengerIndex = passenger.first;
        Uuid passengerUuid = passenger.second;
        auto f = entities.find(passengerUuid);
        if (f == entities.end()) {
          continue;
        }
        auto p = f->second;
        passengers->push_back(p);
        entities.erase(f);
      }
      vehicle->set("Passengers", passengers);
    }
    for (auto const &it : entities) {
      j->fEntities.push_back(it.second);
    }

    return j;
  }
};

} // namespace je2be::toje
