#pragma once

namespace je2be::toje {

class Chunk {
  Chunk() = delete;

public:
  static std::shared_ptr<mcfile::je::WritableChunk> Convert(mcfile::Dimension d, int cx, int cz, mcfile::be::Chunk const &b, terraform::bedrock::BlockAccessorBedrock<3, 3> &cache, Context &ctx) {
    using namespace std;
    using namespace mcfile::blocks::minecraft;

    int const cy = b.fChunkY;
    auto j = mcfile::je::WritableChunk::MakeEmpty(cx, cy, cz);
    j->fLastUpdate = ctx.fGameTick;

    int maxChunkY = cy;
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
      maxChunkY = (std::max)(maxChunkY, (int)sectionB->fChunkY);

      bool hasPoiBlock = false;
      sectionJ->fBlocks.eachValue([&hasPoiBlock](shared_ptr<mcfile::je::Block const> const &block) {
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
              ctx.addToPoiIfItIs(d, Pos3i(cx * 16 + x, sectionJ->fY * 16 + y, cz * 16 + z), *block);
            }
          }
        }
      }

      if (!ctx.fDataPack1_20Update) {
        sectionJ->fBlocks.eachValue([&ctx](shared_ptr<mcfile::je::Block const> const &block) {
          switch (block->fId) {
          case acacia_hanging_sign:
          case acacia_wall_hanging_sign:
          case bamboo_block:
          case bamboo_button:
          case bamboo_door:
          case bamboo_fence:
          case bamboo_fence_gate:
          case bamboo_hanging_sign:
          case bamboo_mosaic:
          case bamboo_mosaic_slab:
          case bamboo_mosaic_stairs:
          case bamboo_planks:
          case bamboo_pressure_plate:
          case bamboo_sign:
          case bamboo_slab:
          case bamboo_stairs:
          case bamboo_trapdoor:
          case bamboo_wall_hanging_sign:
          case bamboo_wall_sign:
          case birch_hanging_sign:
          case birch_wall_hanging_sign:
          case chiseled_bookshelf:
          case crimson_hanging_sign:
          case crimson_wall_hanging_sign:
          case dark_oak_hanging_sign:
          case dark_oak_wall_hanging_sign:
          case jungle_hanging_sign:
          case jungle_wall_hanging_sign:
          case mangrove_hanging_sign:
          case mangrove_wall_hanging_sign:
          case oak_hanging_sign:
          case oak_wall_hanging_sign:
          case piglin_head:
          case piglin_wall_head:
          case spruce_hanging_sign:
          case spruce_wall_hanging_sign:
          case stripped_bamboo_block:
          case warped_hanging_sign:
          case warped_wall_hanging_sign:
            ctx.fDataPack1_20Update = true;
            return false;
          }
          return true;
        });
      }
    }

    for (int y = cy * 16; y <= maxChunkY * 16 + 15; y += 4) {
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
        if (result->fTakeItemsFrom) {
          Pos3i pair = *result->fTakeItemsFrom;
          if (auto pairTileEntity = cache.blockEntityAt(pair); pairTileEntity) {
            auto items = BlockEntity::ContainerItems(*pairTileEntity, "Items", ctx);
            if (items) {
              result->fTileEntity->set("Items", items);
            } else {
              result->fTileEntity->erase("Items");
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

    Terraform(b, *j, cache);

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
      if (!ctx.isRootVehicle(it.first)) {
        j->fEntities.push_back(it.second);
      }
    }

    for (mcfile::be::PendingTick const &pt : b.fPendingTicks) {
      mcfile::je::TickingBlock tb;
      auto nameB = pt.fBlockState->fName;
      tb.fX = pt.fX;
      tb.fY = pt.fY;
      tb.fZ = pt.fZ;
      tb.fT = pt.fTime - b.fCurrentTick;
      tb.fP = 0;
      if (nameB == "minecraft:lava" || nameB == "minecraft:water" || nameB == "minecraft:flowing_water" || nameB == "minecraft:flowing_lava") {
        tb.fI = nameB;
        j->fLiquidTicks.push_back(tb);

        // Copy liquid_depth to block if exists
        if (auto depth = pt.fBlockState->fStates->int32("liquid_depth"); depth) {
          if (auto blockJ = j->blockAt(pt.fX, pt.fY, pt.fZ); blockJ) {
            if (blockJ->property("level") != "") {
              map<string, string> props(blockJ->fProperties);
              props["level"] = to_string(*depth);
              auto replace = make_shared<mcfile::je::Block const>(blockJ->fName, props);
              mcfile::je::SetBlockOptions sbo;
              sbo.fRemoveTileEntity = false;
              j->setBlockAt(pt.fX, pt.fY, pt.fZ, replace, sbo);
            }
          }
        }
      } else {
        auto blockJ = BlockData::From(*pt.fBlockState);
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
          monument->set("ChunkX", Int(cx));
          monument->set("ChunkZ", Int(cz));
          monument->set("id", String("minecraft:monument"));
          monument->set("references", Int(0));
          auto children = List<Tag::Type::Compound>();
          auto child = Compound();
          child->set("id", String("minecraft:omb"));
          child->set("GD", Int(0));
          child->set("O", Int(0));
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
          child->set("BB", bb);
          children->push_back(child);
          monument->set("Children", children);
          startsTag->set("monument", monument);
        }
        vector<int64_t> references;
        references.push_back(StructureInfo::PackStructureStartsReference(s.fStartChunk.fX, s.fStartChunk.fZ));
        referencesTag->set("monument", make_shared<LongArrayTag>(references));
        break;
      }
      default:
        break;
      }
    }
    if (!startsTag->empty()) {
      structuresTag->set("starts", startsTag);
    }
    if (!referencesTag->empty()) {
      structuresTag->set("References", referencesTag);
    }
    if (!structuresTag->empty()) {
      j->fStructures = structuresTag;
    }

    return j;
  }

  static void Terraform(mcfile::be::Chunk const &b, mcfile::je::Chunk &j, terraform::bedrock::BlockAccessorBedrock<3, 3> &cache) {
    using namespace je2be::terraform;
    using namespace je2be::terraform::bedrock;
    BlockPropertyAccessorBedrock accessor(b);
    BlockAccessorWrapper blockAccessor(cache);

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
    Door::Do(j, cache, accessor);
    Leaves::Do(j, blockAccessor, accessor);
  }

  static void AttachLeash(Context &ctx, std::unordered_map<Uuid, CompoundTagPtr, UuidHasher, UuidPred> &entities) {
    using namespace std;
    unordered_set<Uuid, UuidHasher, UuidPred> resolvedLeashedEntities;
    for (auto &it : ctx.fLeashedEntities) {
      Uuid leashedEntityUuid = it.first;
      Context::LeashedEntity const &le = it.second;
      int64_t leasherId = le.fLeasherId;
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
      auto leashTag = Compound();
      leashTag->set("X", Int(leashPos.fX));
      leashTag->set("Y", Int(leashPos.fY));
      leashTag->set("Z", Int(leashPos.fZ));
      leashedEntity->set("Leash", leashTag);

      resolvedLeashedEntities.insert(leashedEntityUuid);
    }
    for (Uuid resolvedLeashedEntity : resolvedLeashedEntities) {
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
      shared_ptr<ListTag> passengersTag = vehicle->listTag("Passengers");
      if (!passengersTag) {
        passengersTag = List<Tag::Type::Compound>();
        vehicle->set("Passengers", passengersTag);
      }
      if (vehicle->string("id") == "minecraft:chicken") {
        vehicle->set("IsChickenJockey", Bool(true));
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
    for (Uuid resolvedVehicleEntity : resolvedVehicleEntities) {
      ctx.fVehicleEntities.erase(resolvedVehicleEntity);
    }
  }
};

} // namespace je2be::toje
