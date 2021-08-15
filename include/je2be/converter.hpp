#pragma once

namespace j2b {

class Converter {
public:
  Converter(std::string const &input, InputOption io, std::string const &output, OutputOption oo) : fInput(input), fOutput(output), fInputOption(io), fOutputOption(oo) {}

  std::optional<Statistics> run(unsigned int concurrency, Progress *progress = nullptr) {
    using namespace std;
    namespace fs = std::filesystem;
    using namespace mcfile;

    double const numTotalChunks = getTotalNumChunks();

    auto rootPath = fs::path(fOutput);
    auto dbPath = rootPath / "db";

    fs::create_directory(rootPath);
    fs::create_directory(dbPath);

    auto data = LevelData::Read(fInputOption.getLevelDatFilePath(fs::path(fInput)));
    if (!data)
      return nullopt;
    LevelData levelData = LevelData::Import(*data);
    levelData.write(fOutput + string("/level.dat"));

    bool ok = true;
    auto worldData = std::make_unique<WorldData>(fs::path(fInput), fInputOption);
    {
      RawDb db(dbPath.string(), concurrency);
      if (!db.valid()) {
        return nullopt;
      }

      uint32_t done = 0;
      for (auto dim : {Dimension::Overworld, Dimension::Nether, Dimension::End}) {
        auto dir = fInputOption.getWorldDirectory(fInput, dim);
        World world(dir.string());
        bool complete = convertWorld(world, dim, db, *worldData, concurrency, progress, done, numTotalChunks);
        ok &= complete;
        if (!complete) {
          break;
        }
      }

      auto localPlayerData = LocalPlayerData(*data, *worldData);
      if (localPlayerData) {
        auto k = Key::LocalPlayer();
        db.put(k, *localPlayerData);
      }

      if (ok) {
        worldData->put(db, *data);
      }

      if (ok) {
        if (progress) {
          progress->report(Progress::Phase::LevelDbCompaction, 0, 1);
        }
      } else {
        db.abandon();
      }
    }

    if (ok && progress) {
      progress->report(Progress::Phase::LevelDbCompaction, 1, 1);
    }

    return worldData->fStat;
  }

#if defined(DOCTEST_VERSION)
public:
#else
private:
#endif
  static std::optional<std::string> LocalPlayerData(mcfile::nbt::CompoundTag const &tag, WorldData &wd) {
    using namespace mcfile::stream;
    using namespace mcfile::nbt;

    auto root = tag.compoundTag("");
    if (!root)
      return std::nullopt;
    auto data = root->compoundTag("Data");
    if (!data)
      return std::nullopt;
    auto player = data->compoundTag("Player");
    if (!player)
      return std::nullopt;

    DimensionDataFragment ddf(Dimension::Overworld);
    auto entity = Entity::LocalPlayer(*player, wd.fJavaEditionMap, ddf);
    if (!entity) {
      return std::nullopt;
    }
    ddf.drain(wd);

    auto s = std::make_shared<ByteStream>();
    OutputStreamWriter w(s, {.fLittleEndian = true});
    w.write((uint8_t)Tag::TAG_Compound);
    w.write(std::string());
    entity->write(w);
    w.write((uint8_t)Tag::TAG_End);

    std::vector<uint8_t> buffer;
    s->drain(buffer);
    std::string ret((char const *)buffer.data(), buffer.size());
    return ret;
  }

  bool convertWorld(mcfile::World const &w, Dimension dim, DbInterface &db, WorldData &wd, unsigned int concurrency, Progress *progress, uint32_t &done, double const numTotalChunks) {
    using namespace std;
    using namespace mcfile;

    ::ThreadPool pool(concurrency);
    pool.init();
    struct Result {
      Result() : fData(nullptr), fOk(false) {}
      shared_ptr<DimensionDataFragment> fData;
      bool fOk;
    };
    deque<future<Result>> futures;

    bool completed = w.eachRegions([this, dim, &db, &pool, &futures, concurrency, &wd, &done, progress, numTotalChunks](shared_ptr<Region> const &region) {
      JavaEditionMap const &mapInfo = wd.fJavaEditionMap;
      for (int cx = region->minChunkX(); cx <= region->maxChunkX(); cx++) {
        for (int cz = region->minChunkZ(); cz <= region->maxChunkZ(); cz++) {
          if (futures.size() > 10 * size_t(concurrency)) {
            for (unsigned int i = 0; i < 5 * concurrency; i++) {
              Result const &result = futures.front().get();
              futures.pop_front();
              done++;
              if (!result.fData)
                continue;
              result.fData->drain(wd);
              if (!result.fOk) {
                return false;
              }
            }
          }

          if (progress) {
            bool continue_ = progress->report(Progress::Phase::Convert, done, numTotalChunks);
            if (!continue_) {
              return false;
            }
          }

          futures.push_back(move(pool.submit([this, dim, &db, region, cx, cz, mapInfo]() -> Result {
            try {
              auto const &chunk = region->chunkAt(cx, cz);
              Result r;
              r.fOk = true;
              if (!chunk) {
                return r;
              }
              if (chunk->fDataVersion >= 2724) {
                vector<shared_ptr<nbt::CompoundTag>> entities;
                if (region->entitiesAt(cx, cz, entities)) {
                  chunk->fEntities.swap(entities);
                }
              }
              PreprocessChunk(chunk, *region);
              r.fData = putChunk(*chunk, dim, db, mapInfo);
              return r;
            } catch (...) {
              Result r;
              r.fData = make_shared<DimensionDataFragment>(dim);
              r.fData->addStatError(dim, cx, cz);
              r.fOk = false;
              return r;
            }
          })));
        }
      }
      return true;
    });

    for (auto &f : futures) {
      Result const &result = f.get();
      done++;
      if (progress) {
        progress->report(Progress::Phase::Convert, done, numTotalChunks);
      }
      if (!result.fData)
        continue;
      result.fData->drain(wd);
    }

    pool.shutdown();

    return completed;
  }

  static void PreprocessChunk(std::shared_ptr<mcfile::Chunk> const &chunk, mcfile::Region region) {
    using namespace mcfile;
    using namespace mcfile::nbt;
    using namespace std;
    using namespace props;

    CachedChunkLoader loader(region);
    SetBlockOptions withoutRemovingTileEntity;
    withoutRemovingTileEntity.fRemoveTileEntity = false;

    unordered_map<Pos3i, shared_ptr<CompoundTag>, Pos3iHasher> tileEntityReplacement;

    for (auto it : chunk->fTileEntities) {
      Pos3i pos = it.first;
      auto const &item = it.second;
      auto id = item->string("id");
      if (!id) {
        continue;
      }
      if (*id == "minecraft:piston") {
        auto extending = item->boolean("extending");
        if (!extending) {
          continue;
        }
        auto source = item->boolean("source");
        if (!source) {
          continue;
        }
        auto facing = item->int32("facing");
        if (!facing) {
          continue;
        }
        if (*source) {
          if (*extending) {
            // extending = 1, source = 1
            auto block = chunk->blockAt(pos);
            if (!block) {
              continue;
            }

            map<string, string> props;
            props["facing_direction"] = to_string(*facing);
            string name = block->fName == "minecraft:sticky_piston" ? "j2b:stickyPistonArmCollision" : "j2b:pistonArmCollision";
            auto newBlock = make_shared<Block>(name, props);
            chunk->setBlockAt(pos, newBlock, withoutRemovingTileEntity);

            tileEntityReplacement[pos] = nullptr;
          } else {
            // extending = 0, source = 1
            auto block = chunk->blockAt(pos);
            if (!block) {
              continue;
            }

            auto sticky = block->property("type", "normal") == "sticky";
            string name = sticky ? "minecraft:sticky_piston" : "minecraft:piston";
            auto newBlock = make_shared<Block>(name, block->fProperties);
            chunk->setBlockAt(pos, newBlock, withoutRemovingTileEntity);

            unordered_set<Pos3i, Pos3iHasher> attachedBlocks;
            LookupAttachedBlocks(loader, pos, *extending, *facing, attachedBlocks);

            auto pistonArm = make_shared<CompoundTag>();
            vector<int32_t> buffer;
            for (auto pos : attachedBlocks) {
              buffer.push_back(pos.fX);
              buffer.push_back(pos.fY);
              buffer.push_back(pos.fZ);
            }
            pistonArm->set("AttachedBlocks", make_shared<IntArrayTag>(buffer));
            pistonArm->set("BreakBlocks", make_shared<IntArrayTag>());
            pistonArm->set("LastProgress", Float(0.5));
            pistonArm->set("NewState", Byte(3));
            pistonArm->set("Progress", Float(0));
            pistonArm->set("State", Byte(3));
            pistonArm->set("Sticky", Bool(sticky));
            pistonArm->set("id", String("j2b:PistonArm"));
            pistonArm->set("isMovable", Bool(false));
            pistonArm->set("x", Int(pos.fX));
            pistonArm->set("y", Int(pos.fY));
            pistonArm->set("z", Int(pos.fZ));

            tileEntityReplacement[pos] = pistonArm;
          }
        } else {
          // extending = *, source = 0
          auto e = MovingBlockEntityFromPistonTileEntity(pos, item);
          tileEntityReplacement[pos] = e;
        }
      }
    }

    for (auto it : tileEntityReplacement) {
      Pos3i pos = it.first;
      auto tileEntity = it.second;
      if (tileEntity) {
        chunk->fTileEntities[pos] = tileEntity;
      } else {
        chunk->fTileEntities.erase(pos);
      }
    }
  }

  static std::shared_ptr<mcfile::nbt::CompoundTag> MovingBlockEntityFromPistonTileEntity(Pos3i pos, std::shared_ptr<mcfile::nbt::CompoundTag const> const &item) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::nbt;
    using namespace props;

    auto e = make_shared<CompoundTag>();
    e->set("id", String("j2b:MovingBlock"));
    e->set("isMovable", Bool(true));

    auto blockState = item->compoundTag("blockState");
    if (!blockState) {
      return nullptr;
    }
    auto name = blockState->string("Name");
    if (!name) {
      return nullptr;
    }
    auto properties = blockState->compoundTag("Properties");
    map<string, string> props;
    if (properties) {
      for (auto p : properties->fValue) {
        string key = p.first;
        StringTag const *s = p.second->asString();
        if (s == nullptr) {
          continue;
        }
        props[key] = s->fValue;
      }
    }
    auto block = make_shared<Block const>(*name, props);
    auto movingBlock = BlockData::From(block);
    if (!movingBlock) {
      return nullptr;
    }
    e->set("movingBlock", movingBlock);

    auto movingBlockExtra = make_shared<CompoundTag>();
    movingBlockExtra->set("name", String("minecraft:air"));
    movingBlockExtra->set("states", make_shared<CompoundTag>());
    movingBlockExtra->set("version", Int(BlockData::kBlockDataVersion));
    e->set("movingBlockExtra", movingBlockExtra);

    e->set("x", Int(pos.fX));
    e->set("y", Int(pos.fY));
    e->set("z", Int(pos.fZ));

    return e;
  }

  static void LookupAttachedBlocks(mcfile::CachedChunkLoader &loader, Pos3i center, bool extendingExpected, int facingExpected, std::unordered_set<Pos3i, Pos3iHasher> &attachedBlocks) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::nbt;

    attachedBlocks.clear();

    static int constexpr kMaxMovableBlocksByAPiston = 12;

    unordered_map<Pos3i, shared_ptr<CompoundTag const>, Pos3iHasher> testedBlocks;
    unordered_set<Pos3i, Pos3iHasher> testBlocks;

    Pos3i const startPos = center + VectorOfFacing(facingExpected);
    shared_ptr<CompoundTag const> startBlock = loader.tileEntityAt(startPos);
    if (!startBlock) {
      return;
    }
    attachedBlocks.insert(startPos);
    testedBlocks.insert(make_pair(startPos, startBlock));
    testedBlocks.insert(make_pair(center, loader.tileEntityAt(center)));
    testBlocks.insert(startPos);

    int const cx = Coordinate::ChunkFromBlock(center.fX);
    int const cz = Coordinate::ChunkFromBlock(center.fZ);

    while (!testBlocks.empty() && attachedBlocks.size() <= kMaxMovableBlocksByAPiston) {
      vector<pair<Pos3i, shared_ptr<CompoundTag const>>> testing;
      for (Pos3i pos : testBlocks) {
        auto base = testedBlocks.find(pos);
        if (base == testedBlocks.end()) {
          continue;
        }
        for (int facing = 0; facing < 6; facing++) {
          Pos3i p = pos + VectorOfFacing(facing);
          if (testedBlocks.find(p) != testedBlocks.end()) {
            continue;
          }
          testing.push_back(make_pair(p, base->second));
        }
      }
      testBlocks.clear();

      sort(testing.begin(), testing.end(), [cx, cz](auto a, auto b) {
        int const acx = Coordinate::ChunkFromBlock(a.first.fX);
        int const acz = Coordinate::ChunkFromBlock(a.first.fZ);
        int const bcx = Coordinate::ChunkFromBlock(b.first.fX);
        int const bcz = Coordinate::ChunkFromBlock(b.first.fZ);
        int const aCost = abs(acx - cx) + abs(acz - cz);
        int const bCost = abs(bcx - cx) + abs(bcz - cz);
        return aCost < bCost;
      });

      for (auto it : testing) {
        Pos3i pos = it.first;
        shared_ptr<CompoundTag const> base = it.second;
        shared_ptr<CompoundTag const> target = loader.tileEntityAt(pos);
        testedBlocks.insert(make_pair(pos, target));
        if (!target) {
          continue;
        }
        if (!IsBaseStickyAgainstTarget(*base, *target)) {
          continue;
        }
        testBlocks.insert(pos);
        attachedBlocks.insert(pos);
      }
    }
  }

  static bool IsBaseStickyAgainstTarget(mcfile::nbt::CompoundTag const &base, mcfile::nbt::CompoundTag const &target) {
    static std::string const slimeBlock = "minecraft:slime_block";
    static std::string const honeyBlock = "minecraft:honey_block";

    auto baseFacing = base.int32("facing");
    auto targetFacing = target.int32("facing");
    if (!baseFacing || !targetFacing) {
      return false;
    }
    if (*baseFacing != *targetFacing) {
      return false;
    }

    auto targetSource = target.boolean("source");
    if (!targetSource) {
      return false;
    }
    if (*targetSource) {
      return false;
    }

    auto baseName = NameFromPistonTileEntity(base);
    auto targetName = NameFromPistonTileEntity(target);
    if (!baseName || !targetName) {
      return false;
    }

    if (baseName == slimeBlock) {
      return targetName != honeyBlock;
    } else if (baseName == honeyBlock) {
      return targetName != slimeBlock;
    } else {
      return false;
    }
  }

  static std::optional<std::string> NameFromPistonTileEntity(mcfile::nbt::CompoundTag const &tag) {
    auto id = tag.string("id");
    if (!id) {
      return std::nullopt;
    }
    if (*id != "minecraft:piston") {
      return std::nullopt;
    }
    auto state = tag.compoundTag("blockState");
    if (!state) {
      return std::nullopt;
    }
    return state->string("Name");
  }

  static Pos3i VectorOfFacing(int facing) {
    if (facing == 1) {
      // up
      return Pos3i(0, 1, 0);
    } else if (facing == 2) {
      // north
      return Pos3i(0, 0, -1);
    } else if (facing == 3) {
      // south
      return Pos3i(0, 0, 1);
    } else if (facing == 4) {
      // west
      return Pos3i(-1, 0, 0);
    } else if (facing == 5) {
      // east
      return Pos3i(1, 0, 0);
    } else {
      // 0, down
      return Pos3i(0, -1, 0);
    }
  }

  std::shared_ptr<DimensionDataFragment> putChunk(mcfile::Chunk &chunk, Dimension dim, DbInterface &db, JavaEditionMap const &mapInfo) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;
    using namespace mcfile::nbt;

    auto ret = make_shared<DimensionDataFragment>(dim);

    ChunkDataPackage cdp;
    ChunkData cd(chunk.fChunkX, chunk.fChunkZ, dim);

    for (int chunkY = 0; chunkY < 16; chunkY++) {
      putSubChunk(chunk, dim, chunkY, cd, cdp, *ret);
    }

    ret->addStructures(chunk);

    cdp.build(chunk, mapInfo, *ret);
    cdp.serialize(cd);

    cd.put(db);

    return ret;
  }

  void putSubChunk(mcfile::Chunk const &chunk, Dimension dim, int chunkY, ChunkData &cd, ChunkDataPackage &cdp, DimensionDataFragment &ddf) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::nbt;
    using namespace props;
    using namespace leveldb;

    size_t const kNumBlocksInSubChunk = 16 * 16 * 16;

    vector<uint16_t> indices(kNumBlocksInSubChunk);
    vector<bool> waterloggedIndices(kNumBlocksInSubChunk);

    // subchunk format
    // 0: version (0x8)
    // 1: num storage blocks
    // 2~: storage blocks repeated

    // storage block format
    // 0: bbbbbbbv (bit, b = bits per block, v = version(0x0))
    // 1-
    // 1~4: paltte size (uint32 LE)
    //

    BlockPalette palette;

    int idx = 0;
    bool empty = true;
    bool hasWaterlogged = false;

    shared_ptr<ChunkSection> section;
    for (int i = 0; i < chunk.fSections.size(); i++) {
      auto const &s = chunk.fSections[i];
      if (!s) {
        continue;
      }
      if (s->y() == chunkY) {
        section = s;
        break;
      }
    }
    if (section != nullptr) {
      auto const &sectionPalette = section->palette();
      vector<uint16_t> mapping(sectionPalette.size());
      int i = 0;
      for (auto const &it : sectionPalette) {
        auto const &paletteKey = it->toString();
        if (paletteKey == "minecraft:air") {
          mapping[i] = 0;
        } else {
          auto tag = BlockData::From(it);
          palette.push(paletteKey, tag);
          mapping[i] = palette.size() - 1;
        }
        i++;
      }
      for (int x = 0; x < 16; x++) {
        int const bx = chunk.minBlockX() + x;
        for (int z = 0; z < 16; z++) {
          int const bz = chunk.minBlockZ() + z;
          for (int y = 0; y < 16; y++, idx++) {
            int const by = chunkY * 16 + y;
            auto rawIndex = section->paletteIndexAt(x, y, z);
            if (!rawIndex) {
              continue;
            }
            if (*rawIndex < 0 || palette.size() <= *rawIndex) {
              continue;
            }
            auto block = sectionPalette[*rawIndex];
            uint16_t index = mapping[*rawIndex];
            empty = false;
            if (index != 0 && !IsAir(block->fName)) {
              cdp.updateAltitude(x, by, z);
            }
            static string const nether_portal("minecraft:nether_portal");
            static string const end_portal("minecraft:end_portal");
            if (TileEntity::IsTileEntity(block->fName)) {
              cdp.addTileBlock(bx, by, bz, block);
            } else if (strings::Equal(block->fName, nether_portal)) {
              bool xAxis = block->property("axis", "x") == "x";
              ddf.addPortalBlock(bx, by, bz, xAxis);
            }
            if (strings::Equal(block->fName, end_portal) && dim == Dimension::End) {
              ddf.addEndPortal(bx, by, bz);
            }

            indices[idx] = index;

            bool waterlogged = block ? IsWaterLogged(*block) : false;
            waterloggedIndices[idx] = waterlogged;
            hasWaterlogged |= waterlogged;
          }
        }
      }
    }

    for (auto it : chunk.fTileEntities) {
      Pos3i pos = it.first;
      shared_ptr<CompoundTag> const &e = it.second;
      if (!TileEntity::IsStandaloneTileEntity(e)) {
        continue;
      }
      auto ret = TileEntity::StandaloneTileEntityBlockdData(pos, e);
      if (!ret) {
        continue;
      }
      auto [tag, paletteKey] = *ret;

      if (pos.fY < chunkY * 16 || chunkY * 16 + 16 <= pos.fY) {
        continue;
      }

      int32_t x = pos.fX - chunk.fChunkX * 16;
      int32_t y = pos.fY - chunkY * 16;
      int32_t z = pos.fZ - chunk.fChunkZ * 16;
      if (x < 0 || 16 <= x || y < 0 || 16 <= y || z < 0 || 16 <= z)
        continue;
      idx = (x * 16 + z) * 16 + y;

      empty = false;

      indices[idx] = palette.add(paletteKey, tag);
    }

    for (auto const &e : chunk.fEntities) {
      if (!Entity::IsTileEntity(*e)) {
        continue;
      }
      auto converted = Entity::ToTileEntityBlock(*e);
      if (!converted) {
        continue;
      }
      auto [pos, tag, paletteKey] = *converted;
      if (pos.fY < chunkY * 16 || chunkY * 16 + 16 <= pos.fY) {
        continue;
      }

      int32_t x = pos.fX - chunk.fChunkX * 16;
      int32_t y = pos.fY - chunkY * 16;
      int32_t z = pos.fZ - chunk.fChunkZ * 16;
      if (x < 0 || 16 <= x || y < 0 || 16 <= y || z < 0 || 16 <= z)
        continue;
      idx = (x * 16 + z) * 16 + y;

      if (indices[idx] != 0) {
        // Not an air block. Avoid replacing current block
        continue;
      }

      empty = false;

      indices[idx] = palette.add(paletteKey, tag);
    }

    if (empty) {
      return;
    }

    int const numStorageBlocks = hasWaterlogged ? 2 : 1;

    auto stream = make_shared<mcfile::stream::ByteStream>();
    mcfile::stream::OutputStreamWriter w(stream, {.fLittleEndian = true});

    w.write(kBlockStorageVersion);
    w.write((uint8_t)numStorageBlocks);

    {
      // layer 0
      int const numPaletteEntries = (int)palette.size();
      uint8_t bitsPerBlock;
      int blocksPerWord;
      if (numPaletteEntries <= 2) {
        bitsPerBlock = 1;
        blocksPerWord = 32;
      } else if (numPaletteEntries <= 4) {
        bitsPerBlock = 2;
        blocksPerWord = 16;
      } else if (numPaletteEntries <= 8) {
        bitsPerBlock = 3;
        blocksPerWord = 10;
      } else if (numPaletteEntries <= 16) {
        bitsPerBlock = 4;
        blocksPerWord = 8;
      } else if (numPaletteEntries <= 32) {
        bitsPerBlock = 5;
        blocksPerWord = 6;
      } else if (numPaletteEntries <= 64) {
        bitsPerBlock = 6;
        blocksPerWord = 5;
      } else if (numPaletteEntries <= 256) {
        bitsPerBlock = 8;
        blocksPerWord = 4;
      } else {
        bitsPerBlock = 16;
        blocksPerWord = 2;
      }

      w.write((uint8_t)(bitsPerBlock * 2));
      uint32_t const mask = ~((~((uint32_t)0)) << bitsPerBlock);
      for (size_t i = 0; i < kNumBlocksInSubChunk; i += blocksPerWord) {
        uint32_t v = 0;
        for (size_t j = 0; j < blocksPerWord && i + j < kNumBlocksInSubChunk; j++) {
          uint32_t const index = (uint32_t)indices[i + j];
          v = v | ((mask & index) << (j * bitsPerBlock));
        }
        w.write(v);
      }

      w.write((uint32_t)numPaletteEntries);

      for (int i = 0; i < palette.size(); i++) {
        shared_ptr<CompoundTag> const &tag = palette[i];
        w.write((uint8_t)Tag::TAG_Compound);
        w.write(std::string());
        tag->write(w);
      }
    }

    if (hasWaterlogged) {
      // layer 1
      int const numPaletteEntries = 2; // air or water
      uint8_t bitsPerBlock = 1;
      int blocksPerWord = 32;

      uint32_t const paletteAir = 0;
      uint32_t const paletteWater = 1;

      w.write((uint8_t)(bitsPerBlock * 2));
      for (size_t i = 0; i < kNumBlocksInSubChunk; i += blocksPerWord) {
        uint32_t v = 0;
        for (size_t j = 0; j < blocksPerWord && i + j < kNumBlocksInSubChunk; j++) {
          bool waterlogged = waterloggedIndices[i + j];
          uint32_t const index = waterlogged ? paletteWater : paletteAir;
          v = v | (index << (j * bitsPerBlock));
        }
        w.write(v);
      }

      w.write((uint32_t)numPaletteEntries);

      auto air = BlockData::Air();
      w.write((uint8_t)Tag::TAG_Compound);
      w.write(string());
      air->write(w);

      auto water = BlockData::Make("water");
      auto states = make_shared<CompoundTag>();
      states->set("liquid_depth", Int(0));
      water->set("states", states);
      w.write((uint8_t)Tag::TAG_Compound);
      w.write(string());
      water->write(w);
    }

    stream->drain(cd.fSubChunks[chunkY]);
  }

  static bool IsWaterLogged(mcfile::Block const &block) {
    using namespace std;
    static string const seagrass("minecraft:seagrass");
    static string const tall_seagrass("minecraft:tall_seagrass");
    static string const kelp("minecraft:kelp");
    static string const kelp_plant("minecraft:kelp_plant");
    static string const bubble_column("minecraft:bubble_column");
    if (!block.fProperties.empty()) {
      auto waterlogged = block.property("waterlogged", "");
      if (strings::Equal(waterlogged, "true")) {
        return true;
      }
    }
    auto const &name = block.fName;
    return strings::Equal(name, seagrass) || strings::Equal(name, tall_seagrass) || strings::Equal(name, kelp) || strings::Equal(name, kelp_plant) || strings::Equal(name, bubble_column);
  }

  static bool IsAir(std::string const &name) {
    static std::string const air("minecraft:air");
    static std::string const cave_air("minecraft:cave_air");
    static std::string const void_air("minecraft:void_air");
    return strings::Equal(name, air) || strings::Equal(name, cave_air) || strings::Equal(name, void_air);
  }

  double getTotalNumChunks() const {
    namespace fs = std::filesystem;
    uint32_t num = 0;
    for (auto dim : {Dimension::Overworld, Dimension::Nether, Dimension::End}) {
      auto dir = fInputOption.getWorldDirectory(fInput, dim) / "region";
      if (!fs::exists(dir)) {
        continue;
      }
      for (auto const &e : fs::directory_iterator(dir)) {
        if (!fs::is_regular_file(e.path())) {
          continue;
        }
        auto name = e.path().filename().string();
        if (strings::StartsWith(name, "r.") && strings::EndsWith(name, ".mca")) {
          num++;
        }
      }
    }
    return num * 32.0 * 32.0;
  }

private:
  std::string const fInput;
  std::string const fOutput;
  InputOption const fInputOption;
  OutputOption const fOutputOption;

private:
  uint8_t const kBlockStorageVersion = 8;
};

} // namespace j2b
