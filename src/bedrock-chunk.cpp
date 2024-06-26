#include "bedrock/_chunk.hpp"

#include <je2be/uuid.hpp>

#include "_poi-blocks.hpp"
#include "_pos3.hpp"
#include "_props.hpp"
#include "bedrock/_block-accessor-wrapper.hpp"
#include "bedrock/_block-entity.hpp"
#include "bedrock/_constants.hpp"
#include "bedrock/_context.hpp"
#include "bedrock/_entity.hpp"
#include "bedrock/_structure-info.hpp"
#include "bedrock/_sub-chunk.hpp"
#include "bedrock/terraform/_beacon.hpp"
#include "bedrock/terraform/_campfire.hpp"
#include "bedrock/terraform/_cave-vines.hpp"
#include "bedrock/terraform/_double_plant.hpp"
#include "bedrock/terraform/_piston.hpp"
#include "bedrock/terraform/_tripwire.hpp"
#include "bedrock/terraform/_twisting-vines.hpp"
#include "bedrock/terraform/_weeping-vines.hpp"
#include "enums/_chunk-conversion-mode.hpp"
#include "java/_versions.hpp"
#include "structure/_structure-piece.hpp"
#include "terraform/_chorus-plant.hpp"
#include "terraform/_door.hpp"
#include "terraform/_fence-connectable.hpp"
#include "terraform/_leaves.hpp"
#include "terraform/_note-block.hpp"
#include "terraform/_redstone-wire.hpp"
#include "terraform/_shape-of-stairs.hpp"
#include "terraform/_snowy.hpp"
#include "terraform/bedrock/_attached-stem.hpp"
#include "terraform/bedrock/_kelp.hpp"

namespace je2be::bedrock {

class Chunk::Impl {
  Impl() = delete;

public:
  static Status Convert(mcfile::Dimension d, int cx, int cz, mcfile::be::Chunk const &b, terraform::bedrock::BlockAccessorBedrock<3, 3> &cache, Context &ctx, std::shared_ptr<mcfile::je::WritableChunk> &out) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::biomes;
    using namespace mcfile::blocks::minecraft;

    int const cy = b.fChunkY;
    auto j = mcfile::je::WritableChunk::MakeEmpty(cx, cy, cz);
    j->fLastUpdate = ctx.fGameTick;

    auto mode = ConversionMode(b);
    int const dataVersion = DataVersion::TargetVersionByMode(mode);
    switch (mode) {
    case ChunkConversionMode::CavesAndCliffs2:
      break;
    case ChunkConversionMode::Legacy:
      j->fLegacyBiomes.resize(1024);
      break;
    }
    j->setDataVersion(dataVersion);

    int maxChunkY = cy;
    for (auto const &sectionB : b.fSubChunks) {
      if (!sectionB) {
        continue;
      }

      auto sectionJ = SubChunk::Convert(*sectionB, d, mode, dataVersion);
      if (!sectionJ) {
        return JE2BE_ERROR;
      }

      int sectionIndex = sectionJ->y() - j->fChunkY;
      if (j->fSections.size() <= sectionIndex) {
        j->fSections.resize(sectionIndex + 1);
      }
      j->fSections[sectionIndex] = sectionJ;
      maxChunkY = (std::max)(maxChunkY, (int)sectionB->fChunkY);

      bool hasPoiBlock = false;
      sectionJ->eachBlockPalette([&hasPoiBlock](shared_ptr<mcfile::je::Block const> const &block, size_t) {
        if (PoiBlocks::Interest(*block)) {
          hasPoiBlock = true;
          return false;
        } else {
          return true;
        }
      });
      if (hasPoiBlock) {
        for (int y = 0; y < 16; y++) {
          for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
              auto block = sectionJ->blockAt(x, y, z);
              if (!block) {
                continue;
              }
              ctx.addToPoiIfItIs(d, Pos3i(cx * 16 + x, sectionJ->y() * 16 + y, cz * 16 + z), *block);
            }
          }
        }
      }
    }

    BiomeId defaultBiome;
    switch (d) {
    case mcfile::Dimension::Nether:
      defaultBiome = minecraft::nether_wastes;
      break;
    case Dimension::End:
      defaultBiome = minecraft::the_end;
      break;
    case Dimension::Overworld:
    default:
      defaultBiome = minecraft::plains;
      break;
    }

    for (int by = j->minBlockY(); by <= j->maxBlockY(); by += 4) {
      for (int bx = j->minBlockX(); bx <= j->maxBlockX(); bx += 4) {
        for (int bz = j->minBlockZ(); bz <= j->maxBlockZ(); bz += 4) {
          mcfile::biomes::BiomeId biome = defaultBiome;
          if (auto biomeB = b.biomeAt(bx, by, bz); biomeB) {
            biome = *biomeB;
          } else {
            int lowerSectionY = mcfile::Coordinate::ChunkFromBlock(by) - 1;
            if (auto lowerBiomeJ = j->biomeAt(bx, lowerSectionY * 16 + 15, bz); lowerBiomeJ != mcfile::biomes::unknown) {
              biome = lowerBiomeJ;
            }
          }
          j->setBiomeAt(bx, by, bz, biome);
        }
      }
    }

    for (auto const &it : b.blockEntities()) {
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
      auto result = BlockEntity::FromBlockAndBlockEntity(pos, *blockB, *tagB, *blockJ, ctx, dataVersion, false);
      if (!result) {
        continue;
      }
      if (result->fTileEntity) {
        if (result->fTakeItemsFrom) {
          Pos3i pair = *result->fTakeItemsFrom;
          if (auto pairTileEntity = cache.blockEntityAt(pair); pairTileEntity) {
            auto items = BlockEntity::ContainerItems(*pairTileEntity, u8"Items", ctx, dataVersion, false);
            if (items) {
              result->fTileEntity->set(u8"Items", items);
            } else {
              result->fTileEntity->erase(u8"Items");
            }
          }
        }

        j->fTileEntities[pos] = result->fTileEntity;
      }
      if (result->fBlock) {
        mcfile::je::SetBlockOptions o;
        o.fRemoveTileEntity = false;
        j->setBlockAt(pos, result->fBlock, o);
      }
    }

    for (auto const &it : b.blockEntities()) {
      auto id = it.second->string(u8"id");
      if (!id) {
        continue;
      }
      if (id != u8"ItemFrame" && id != u8"GlowItemFrame") {
        continue;
      }
      Pos3i pos = it.first;
      auto blockB = cache.blockAt(pos.fX, pos.fY, pos.fZ);
      if (!blockB) {
        continue;
      }
      auto frameJ = Entity::ItemFrameFromBedrock(d, pos, *blockB, *it.second, ctx, dataVersion);
      if (frameJ) {
        j->fEntities.push_back(frameJ);
      }
    }

    Terraform(b, *j, cache);

    unordered_map<Uuid, shared_ptr<CompoundTag>, UuidHasher, UuidPred> entities;
    for (auto const &entityB : b.entities()) {
      auto result = Entity::From(*entityB, ctx, dataVersion);
      if (result) {
        Pos2i pos(cx, cz);
        entities[result->fUuid] = result->fEntity;
        if (result->fLeasherId) {
          if (auto localPlayer = ctx.mapLocalPlayerId(*result->fLeasherId); localPlayer) {
            // Nop here.
          } else {
            ctx.fLeashedEntities[result->fUuid] = {.fChunk = pos, .fLeasherId = *result->fLeasherId};
          }
        }
        if (!result->fPassengers.empty()) {
          Context::VehicleEntity ve;
          ve.fChunk = pos;
          ve.fPassengers.swap(result->fPassengers);
          ctx.fVehicleEntities[result->fUuid] = ve;
        }
        ctx.fEntities[result->fUuid] = pos;
      } else if (entityB->string(u8"identifier") == u8"minecraft:leash_knot") {
        auto id = entityB->int64(u8"UniqueID");
        auto posf = props::GetPos3f(*entityB, u8"Pos");
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
    AttachLeash(ctx, entities, dataVersion);

    for (auto const &it : entities) {
      if (!ctx.isRootVehicle(it.first)) {
        j->fEntities.push_back(it.second);
      }
    }

    for (mcfile::be::PendingTick const &pt : b.pendingTicks()) {
      mcfile::je::TickingBlock tb;
      auto nameB = pt.fBlockState->fName;
      tb.fX = pt.fX;
      tb.fY = pt.fY;
      tb.fZ = pt.fZ;
      tb.fT = pt.fTime - b.currentTick();
      tb.fP = 0;
      if (nameB == u8"minecraft:lava" || nameB == u8"minecraft:water" || nameB == u8"minecraft:flowing_water" || nameB == u8"minecraft:flowing_lava") {
        tb.fI = nameB;
        j->fLiquidTicks.push_back(tb);

        // Copy liquid_depth to block if exists
        if (auto depth = pt.fBlockState->fStates->int32(u8"liquid_depth"); depth) {
          if (auto blockJ = j->blockAt(pt.fX, pt.fY, pt.fZ); blockJ) {
            if (blockJ->property(u8"level") != u8"") {
              auto replace = blockJ->applying({{u8"level", mcfile::String::ToString(*depth)}});
              mcfile::je::SetBlockOptions sbo;
              sbo.fRemoveTileEntity = false;
              j->setBlockAt(pt.fX, pt.fY, pt.fZ, replace, sbo);
            }
          }
        }
      } else {
        auto blockJ = BlockData::From(*pt.fBlockState, dataVersion);
        if (blockJ) {
          tb.fI = blockJ->fName;
          j->fTileTicks.push_back(tb);
        }
      }
    }

    vector<StructureInfo::Structure> structures;
    ctx.structures(d, Pos2i(cx, cz), structures);

    auto structuresTag = Compound();
    auto startsTag = Compound();
    auto referencesTag = Compound();
    for (auto const &s : structures) {
      switch (s.fType) {
      case StructureType::Monument: {
        if (s.fStartChunk == Pos2i(cx, cz)) {
          auto monument = Compound();
          monument->set(u8"ChunkX", Int(cx));
          monument->set(u8"ChunkZ", Int(cz));
          monument->set(u8"id", u8"minecraft:monument");
          monument->set(u8"references", Int(0));
          auto children = List<Tag::Type::Compound>();
          auto child = Compound();
          child->set(u8"id", u8"minecraft:omb");
          child->set(u8"GD", Int(0));
          child->set(u8"O", Int(0));
          // IdentifyFacingOfOceanMonument https://github.com/kbinani/je2be/commit/6ca28383bc557bcf60b8203e655fdbb7a87d39d7
          // O=0: north
          // O=1: east
          // O=3: west
          vector<int> bbv;
          bbv.push_back(s.fBounds.fStart.fX);
          bbv.push_back(s.fBounds.fStart.fY);
          bbv.push_back(s.fBounds.fStart.fZ);
          bbv.push_back(s.fBounds.fEnd.fX);
          bbv.push_back(s.fBounds.fEnd.fY);
          bbv.push_back(s.fBounds.fEnd.fZ);
          auto bb = make_shared<IntArrayTag>(bbv);
          child->set(u8"BB", bb);
          children->push_back(child);
          monument->set(u8"Children", children);
          startsTag->set(u8"minecraft:monument", monument);
        }
        vector<i64> references;
        references.push_back(StructureInfo::PackStructureStartsReference(s.fStartChunk.fX, s.fStartChunk.fZ));
        referencesTag->set(u8"minecraft:monument", make_shared<LongArrayTag>(references));
        break;
      }
      default:
        break;
      }
    }
    if (!startsTag->empty()) {
      structuresTag->set(u8"starts", startsTag);
    }
    if (!referencesTag->empty()) {
      structuresTag->set(u8"References", referencesTag);
    }
    if (!structuresTag->empty()) {
      j->fStructures = structuresTag;
    }

    out.swap(j);
    return Status::Ok();
  }

  static void Terraform(mcfile::be::Chunk const &b, mcfile::je::Chunk &j, terraform::bedrock::BlockAccessorBedrock<3, 3> &cache) {
    using namespace je2be::terraform;
    using namespace je2be::terraform::bedrock;
    BlockPropertyAccessorBedrock accessor(b);
    BlockAccessorWrapper blockAccessor(cache, j.getDataVersion());

    Piston::Do(j, cache, accessor);

    ShapeOfStairs::Do(j, blockAccessor, accessor);
    Kelp::Do(j, accessor);
    TwistingVines::Do(j, cache, accessor);
    WeepingVines::Do(j, cache, accessor);
    AttachedStem::Do(j, cache, accessor);
    CaveVines::Do(j, cache, accessor);
    Snowy::Do(j, blockAccessor, accessor);
    ChorusPlant::Do(j, blockAccessor, accessor);
    FenceConnectable::Do(j, blockAccessor, accessor);
    Campfire::Do(j, cache, accessor);
    NoteBlock::Do(j, blockAccessor, accessor);
    RedstoneWire::Do(j, blockAccessor, accessor);
    Tripwire::Do(j, cache, accessor);
    Beacon::Do(j, cache, accessor);
    Door::Do(j, accessor);
    DoublePlant::Do(j, accessor);
  }

  static void AttachLeash(Context &ctx, std::unordered_map<Uuid, CompoundTagPtr, UuidHasher, UuidPred> &entities, int dataVersion) {
    using namespace std;
    unordered_set<Uuid, UuidHasher, UuidPred> resolvedLeashedEntities;
    for (auto &it : ctx.fLeashedEntities) {
      Uuid leashedEntityUuid = it.first;
      Context::LeashedEntity const &le = it.second;
      i64 leasherId = le.fLeasherId;
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
      if (dataVersion >= kJavaDataVersionComponentIntroduced) {
        leashedEntity->set(u8"leash", IntArrayFromPos3i(leashPos));
      } else {
        auto leashTag = Compound();
        leashTag->set(u8"X", Int(leashPos.fX));
        leashTag->set(u8"Y", Int(leashPos.fY));
        leashTag->set(u8"Z", Int(leashPos.fZ));
        leashedEntity->set(u8"Leash", leashTag);
      }

      resolvedLeashedEntities.insert(leashedEntityUuid);
    }
    for (Uuid const &resolvedLeashedEntity : resolvedLeashedEntities) {
      ctx.fLeashedEntities.erase(resolvedLeashedEntity);
    }
  }

  static void AttachPassengers(Context &ctx,
                               std::unordered_map<Uuid, CompoundTagPtr, UuidHasher, UuidPred> &entities) {
    using namespace std;
    unordered_set<Uuid, UuidHasher, UuidPred> resolvedVehicleEntities;
    for (auto &it : ctx.fVehicleEntities) {
      Uuid vehicleUuid = it.first;
      Context::VehicleEntity &ve = it.second;
      std::map<size_t, Uuid> &passengers = ve.fPassengers;
      auto found = entities.find(vehicleUuid);
      if (found == entities.end()) {
        continue;
      }
      shared_ptr<CompoundTag> vehicle = found->second;
      shared_ptr<ListTag> passengersTag = vehicle->listTag(u8"Passengers");
      if (!passengersTag) {
        passengersTag = List<Tag::Type::Compound>();
        vehicle->set(u8"Passengers", passengersTag);
      }
      if (vehicle->string(u8"id") == u8"minecraft:chicken") {
        vehicle->set(u8"IsChickenJockey", Bool(true));
      }
      unordered_set<size_t> resolvedPassengers;
      for (auto const &passenger : passengers) {
        size_t passengerIndex = passenger.first;
        Uuid passengerUuid = passenger.second;
        if (ctx.isLocalPlayerId(passengerUuid)) {
          resolvedPassengers.insert(passengerIndex);
          continue;
        }
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
        if (ctx.isRootVehicle(vehicleUuid)) {
          ctx.setRootVehicle(vehicleUuid);
          ctx.setRootVehicleEntity(vehicle);
          entities.erase(vehicleUuid);
        }
      }
    }
    for (Uuid const &resolvedVehicleEntity : resolvedVehicleEntities) {
      ctx.fVehicleEntities.erase(resolvedVehicleEntity);
    }
  }

  static ChunkConversionMode ConversionMode(mcfile::be::Chunk const &b) {
    if (b.fVersion > java::kChunkVersionMaxLegacy) {
      return ChunkConversionMode::CavesAndCliffs2;
    }
    if (b.fChunkY < 0) {
      return ChunkConversionMode::CavesAndCliffs2;
    }
    if (b.fChunkY > 15) {
      return ChunkConversionMode::CavesAndCliffs2;
    }
    return ChunkConversionMode::Legacy;
  }
};

Status Chunk::Convert(mcfile::Dimension d, int cx, int cz, mcfile::be::Chunk const &b, terraform::bedrock::BlockAccessorBedrock<3, 3> &cache, Context &ctx, std::shared_ptr<mcfile::je::WritableChunk> &out) {
  return Impl::Convert(d, cx, cz, b, cache, ctx, out);
}

} // namespace je2be::bedrock
