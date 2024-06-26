#include <je2be/lce/converter.hpp>

#include <je2be/fs.hpp>
#include <je2be/lce/behavior.hpp>
#include <je2be/lce/options.hpp>
#include <je2be/lce/progress.hpp>
#include <je2be/zip-file.hpp>

#include <defer.hpp>

#include "_dimension-ext.hpp"
#include "_directory-iterator.hpp"
#include "_file.hpp"
#include "_java-level-dat.hpp"
#include "_nbt-ext.hpp"
#include "_nullable.hpp"
#include "_props.hpp"
#include "bedrock/_constants.hpp"
#include "lce/_chunk.hpp"
#include "lce/_context.hpp"
#include "lce/_entity.hpp"
#include "lce/_savegame.hpp"
#include "lce/_tile-entity.hpp"
#include "lce/_world.hpp"
#include "xbox360/_save-bin.hpp"

namespace je2be::lce {

class Converter::Impl {
  Impl() = delete;

public:
  static Status Run(std::vector<uint8_t> const &inputSavegame,
                    std::filesystem::path const &outputDirectory,
                    unsigned int concurrency,
                    Behavior const &behavior,
                    Options const &options,
                    Progress *progress = nullptr) {
    using namespace std;
    namespace fs = std::filesystem;

    auto tempRoot = options.getTempDirectory();
    auto temp = mcfile::File::CreateTempDir(tempRoot);
    if (!temp) {
      return JE2BE_ERROR;
    }
    defer {
      Fs::DeleteAll(*temp);
    };

    if (!Savegame::ExtractFilesFromDecompressedSavegame(inputSavegame, *temp)) {
      return JE2BE_ERROR;
    }
    Context ctx(TileEntity::Convert, Entity::MigrateName);
    if (auto st = CopyMapFiles(*temp, outputDirectory); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    auto copyPlayersResult = CopyPlayers(*temp, outputDirectory, behavior, ctx, options);
    if (!copyPlayersResult) {
      return copyPlayersResult.status();
    }
    if (auto st = CopyLevelDat(*temp, outputDirectory, options.fLastPlayed, copyPlayersResult->fLocalPlayer, ctx); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    if (auto st = SetupResourcePack(outputDirectory); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    if (auto st = SetupDataPack(outputDirectory); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    int progressChunksOffset = 0;
    for (auto dimension : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
      defer {
        progressChunksOffset += World::ProgressWeight(dimension);
      };
      if (!options.fDimensionFilter.empty()) {
        if (options.fDimensionFilter.find(dimension) == options.fDimensionFilter.end()) {
          continue;
        }
      }
      if (auto st = World::Convert(*temp, outputDirectory, dimension, concurrency, behavior, ctx, options, progress, progressChunksOffset); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
    }
    return Status::Ok();
  }

private:
  struct PlayerInfo {
    Uuid fUuidJ;
    CompoundTag fPlayer;
    std::u8string fUuidX;
  };

  struct CopyPlayersResult {
    std::shared_ptr<PlayerInfo> fLocalPlayer;
  };

  static Nullable<CopyPlayersResult> CopyPlayers(std::filesystem::path const &inputDirectory,
                                                 std::filesystem::path const &outputDirectory,
                                                 Behavior const &behavior,
                                                 Context &ctx,
                                                 Options const &options) {
    using namespace std;
    namespace fs = std::filesystem;

    auto playersTo = outputDirectory / "playerdata";

    if (!Fs::CreateDirectories(playersTo)) {
      return JE2BE_NULLABLE_NULL;
    }

    CopyPlayersResult r;
    r.fLocalPlayer = nullptr;

    map<fs::path, CompoundTagPtr> tags;
    if (auto st = behavior.loadPlayers(inputDirectory, tags); !st.ok()) {
      return JE2BE_NULLABLE_NULL;
    }

    vector<shared_ptr<PlayerInfo>> players;
    for (auto const &it : tags) {
      if (!it.second) {
        continue;
      }
      if (auto info = CopyPlayer(it.first, *it.second, playersTo, ctx); info) {
        players.push_back(info);
      }
    }
    map<fs::path, CompoundTagPtr>().swap(tags);

    if (players.size() == 1) {
      r.fLocalPlayer = players[0];
      players.clear();
    }

    if (r.fLocalPlayer) {
      if (options.fLocalPlayer) {
        r.fLocalPlayer->fPlayer.set(u8"UUID", options.fLocalPlayer->toIntArrayTag());
        ctx.fPlayers.insert(make_pair(r.fLocalPlayer->fUuidX, *options.fLocalPlayer));
      } else {
        ctx.fPlayers.insert(make_pair(r.fLocalPlayer->fUuidX, r.fLocalPlayer->fUuidJ));
      }
      players.push_back(r.fLocalPlayer);
    }

    for (auto const &player : players) {
      auto filename = player->fUuidJ.toString() + u8".dat";
      auto filePath = playersTo / filename;
      if (!CompoundTag::Write(player->fPlayer, filePath, mcfile::Encoding::Java)) {
        return JE2BE_NULLABLE_NULL;
      }
    }

    return r;
  }

  static std::shared_ptr<PlayerInfo> CopyPlayer(std::filesystem::path const &inputFile, CompoundTag const &inputTag, std::filesystem::path const &outputPlayerdata, Context const &ctx) {
    using namespace std;

    auto r = make_shared<PlayerInfo>();
    r->fPlayer.fValue.swap(inputTag.copy()->fValue);

    r->fPlayer.erase(u8"GamePrivileges");
    r->fPlayer.erase(u8"Sleeping");

    Entity::CopyItems(inputTag, r->fPlayer, ctx, u8"EnderItems");
    Entity::CopyItems(inputTag, r->fPlayer, ctx, u8"Inventory");

    if (auto dimensionB = inputTag.int32(u8"Dimension"); dimensionB) {
      if (auto dimension = DimensionFromXbox360Dimension(*dimensionB); dimension) {
        r->fPlayer.set(u8"Dimension", JavaStringFromDimension(*dimension));
      }
    }

    r->fPlayer.set(u8"DataVersion", Int(Chunk::kTargetDataVersion));

    if (auto rootVehicleB = inputTag.compoundTag(u8"RootVehicle"); rootVehicleB) {
      auto rootVehicleJ = Compound();
      if (auto entityB = rootVehicleB->compoundTag(u8"Entity"); entityB) {
        if (auto entityJ = Entity::Convert(*entityB, ctx); entityJ && entityJ->fEntity) {
          rootVehicleJ->set(u8"Entity", entityJ->fEntity);
        }
      }
      if (auto attachB = rootVehicleB->string(u8"Attach"); attachB) {
        if (auto attachJ = Entity::MigrateUuid(*attachB, ctx); attachJ) {
          rootVehicleJ->set(u8"Attach", attachJ->toIntArrayTag());
        }
      }
      r->fPlayer.set(u8"RootVehicle", rootVehicleJ);
    }

    shared_ptr<Uuid> uuidJ;
    if (auto uuidB = inputTag.string(u8"UUID"); uuidB) {
      if (auto migrated = Entity::MigrateUuid(*uuidB, ctx); migrated) {
        uuidJ = make_shared<Uuid>(*migrated);
        r->fUuidX = *uuidB;
      }
    }
    if (!uuidJ) {
      auto name = inputFile.filename().replace_extension().u8string();
      // TODO: should this be converted to "entNNNN.." format?
      if (auto id = strings::ToU64(name, 10); id) {
        auto generated = Uuid::GenWithU64Seed(*id);
        uuidJ = make_shared<Uuid>(generated);
        r->fUuidX = u8"je2be_internal_" + name;
      }
    }
    if (!uuidJ) {
      return nullptr;
    }
    r->fPlayer.set(u8"UUID", uuidJ->toIntArrayTag());

    r->fUuidJ = *uuidJ;
    return r;
  }

  static void CopyTransparent16x16Png(std::vector<u8> &buffer) {
    using namespace std;
    unsigned char transparent16x16_png[] = {
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
        0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
        0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff, 0x61, 0x00, 0x00, 0x00,
        0x12, 0x49, 0x44, 0x41, 0x54, 0x38, 0xcb, 0x63, 0x60, 0x18, 0x05, 0xa3,
        0x60, 0x14, 0x8c, 0x02, 0x08, 0x00, 0x00, 0x04, 0x10, 0x00, 0x01, 0x85,
        0x3f, 0xaa, 0x72, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae,
        0x42, 0x60, 0x82};
    unsigned int transparent16x16_png_len = 75;
    buffer.clear();
    copy_n(transparent16x16_png, transparent16x16_png_len, back_inserter(buffer));
  }

  static Status SetupResourcePack(std::filesystem::path const &outputDirectory) {
    using namespace std;

    auto resourcesZipPath = outputDirectory / "resources.zip";
    Fs::Delete(resourcesZipPath);

    ZipFile resources(resourcesZipPath);
    {
      vector<u8> forcefield;
      CopyTransparent16x16Png(forcefield);
      if (auto st = resources.store(forcefield, "assets/minecraft/textures/misc/forcefield.png"); !st.fStatus.ok()) {
        return st.fStatus;
      }
    }
    {
      nlohmann::json obj;
      nlohmann::json pack;
      pack["pack_format"] = 6;
      pack["description"] = "invisible worldborder by je2be";
      obj["pack"] = pack;
      auto str = nlohmann::to_string(obj);
      vector<u8> mcmeta;
      copy(str.begin(), str.end(), back_inserter(mcmeta));
      if (auto st = resources.store(mcmeta, "pack.mcmeta"); !st.fStatus.ok()) {
        return st.fStatus;
      }
    }

    return resources.close().fStatus;
  }

  static Status SetupDataPack(std::filesystem::path const &outputDirectory) {
    using namespace nlohmann;

    if (!Fs::CreateDirectories(outputDirectory / "datapacks" / "nether3x" / "data" / "minecraft" / "dimension_type")) {
      return JE2BE_ERROR;
    }

    // pack.mcdata
    {
      json root;
      json pack;
      pack["description"] = "nether3x";
      pack["pack_format"] = 6;
      root["pack"] = pack;
      auto data = to_string(root);
      mcfile::ScopedFile mcmeta(mcfile::File::Open(outputDirectory / "datapacks" / "nether3x" / "pack.mcmeta", mcfile::File::Mode::Write));
      if (!mcmeta) {
        return JE2BE_ERROR;
      }
      if (fwrite(data.c_str(), data.size(), 1, mcmeta.get()) != 1) {
        return JE2BE_ERROR;
      }
    }

    // pack.png
    {
      std::vector<u8> data;
      CopyTransparent16x16Png(data);
      mcfile::ScopedFile png(mcfile::File::Open(outputDirectory / "datapacks" / "nether3x" / "pack.png", mcfile::File::Mode::Write));
      if (!png) {
        return JE2BE_ERROR;
      }
      if (fwrite(data.data(), data.size(), 1, png.get()) != 1) {
        return JE2BE_ERROR;
      }
    }

    // the_nether.json
    {
      json o;
      o.emplace("ultrawarm", true);
      o.emplace("natural", false);
      o["coordinate_scale"] = 3.0;
      o.emplace("has_skylight", false);
      o.emplace("has_ceiling", true);
      o["ambient_light"] = 0.1;
      o["fixed_time"] = 18000;
      o["monster_spawn_light_level"] = 11;
      o["monster_spawn_block_light_limit"] = 15;
      o.emplace("piglin_safe", true);
      o.emplace("bed_works", false);
      o.emplace("respawn_anchor_works", true);
      o.emplace("has_raids", false);
      o["logical_height"] = 128;
      o["min_y"] = 0;
      o["height"] = 256;
      o["infiniburn"] = "#minecraft:infiniburn_nether";
      o["effects"] = "minecraft:the_nether";
      auto data = to_string(o);
      mcfile::ScopedFile json(mcfile::File::Open(outputDirectory / "datapacks" / "nether3x" / "data" / "minecraft" / "dimension_type" / "the_nether.json", mcfile::File::Mode::Write));
      if (!json) {
        return JE2BE_ERROR;
      }
      if (fwrite(data.c_str(), data.size(), 1, json.get()) != 1) {
        return JE2BE_ERROR;
      }
    }

    return Status::Ok();
  }

  static Status CopyLevelDat(std::filesystem::path const &inputDirectory,
                             std::filesystem::path const &outputDirectory,
                             std::optional<std::chrono::system_clock::time_point> lastPlayed,
                             std::shared_ptr<PlayerInfo> const &localPlayer,
                             Context &ctx) {
    using namespace std;
    namespace fs = std::filesystem;
    auto datFrom = inputDirectory / "level.dat";
    auto datTo = outputDirectory / "level.dat";
    if (!Fs::Exists(datFrom)) {
      return JE2BE_ERROR;
    }

    auto inStream = make_shared<mcfile::stream::GzFileInputStream>(datFrom);
    auto inRoot = CompoundTag::Read(inStream, mcfile::Encoding::Java);
    inStream.reset();
    if (!inRoot) {
      return JE2BE_ERROR;
    }
    auto in = inRoot->compoundTag(u8"Data");
    if (!in) {
      return JE2BE_ERROR;
    }
    ctx.fNewSeaLevel = in->boolean(u8"newSeaLevel", false);
    auto flat = in->string(u8"generatorName") == u8"flat";

    CompoundTagPtr flatWorldSettings;
    if (flat) {
      // auto generatorVersion = in->int32(u8"generatorVersion");
      // generatorVersion
      // 0: TU9, TU67, TU74

      flatWorldSettings = FlatWorldSettings();
    }
    vector<u8string> enabledDataPacks;
    enabledDataPacks.push_back(u8"file/nether3x");

    auto data = Compound();
    CompoundTag &j = *data;

    j[u8"DataVersion"] = Int(Chunk::kTargetDataVersion);
    j[u8"version"] = Int(19133);

    bool doWeatherCycle = true;
    auto thunderTime = in->int32(u8"thunderTime");
    auto rainTime = in->int32(u8"rainTime");
    if (thunderTime && rainTime) {
      doWeatherCycle = *thunderTime > 0 || *rainTime > 0;
    }

    bool dyDaylightCycle = true;
    auto dayTime = in->int64(u8"DayTime");
    if (dayTime) {
      dyDaylightCycle = *dayTime >= 0;
    }

    auto gameRules = Compound();
    CompoundTag &gr = *gameRules;
    gr[u8"announceAdvancements"] = String(u8"true");
    gr[u8"commandBlockOutput"] = String(u8"true");
    gr[u8"disableElytraMovementCheck"] = String(u8"false");
    gr[u8"disableRaids"] = String(u8"false");
    gr[u8"doDaylightCycle"] = String(dyDaylightCycle ? u8"true" : u8"false");
    gr[u8"doEntityDrops"] = String(u8"true");
    gr[u8"doFireTick"] = String(u8"true");
    gr[u8"doImmediateRespawn"] = String(u8"false");
    gr[u8"doInsomnia"] = String(u8"true");
    gr[u8"doLimitedCrafting"] = String(u8"false");
    gr[u8"doMobLoot"] = String(u8"true");
    gr[u8"doMobSpawning"] = String(u8"true");
    gr[u8"doPatrolSpawning"] = String(u8"true");
    gr[u8"doTileDrops"] = String(u8"true");
    gr[u8"doTraderSpawning"] = String(u8"true");
    gr[u8"doWeatherCycle"] = String(doWeatherCycle ? u8"true" : u8"false");
    gr[u8"drowningDamage"] = String(u8"true");
    gr[u8"fallDamage"] = String(u8"true");
    gr[u8"fireDamage"] = String(u8"true");
    gr[u8"forgiveDeadPlayers"] = String(u8"true");
    gr[u8"keepInventory"] = String(u8"false");
    gr[u8"logAdminCommands"] = String(u8"true");
    gr[u8"maxCommandChainLength"] = String(u8"65536");
    gr[u8"maxEntityCramming"] = String(u8"24");
    gr[u8"mobGriefing"] = String(u8"true");
    gr[u8"naturalRegeneration"] = String(u8"true");
    gr[u8"randomTickSpeed"] = String(u8"3");
    gr[u8"reducedDebugInfo"] = String(u8"false");
    gr[u8"sendCommandFeedback"] = String(u8"true");
    gr[u8"showDeathMessages"] = String(u8"true");
    gr[u8"spawnRadius"] = String(u8"10");
    gr[u8"spectatorsGenerateChunks"] = String(u8"true");
    gr[u8"universalAnger"] = String(u8"false");
    j[u8"GameRules"] = gameRules;

    {
      auto dataPacks = Compound();
      dataPacks->set(u8"Disabled", List<Tag::Type::String>());
      auto enabled = List<Tag::Type::String>();
      enabled->push_back(String(u8"vanilla"));
      for (auto const &pack : enabledDataPacks) {
        enabled->push_back(String(pack));
      }
      dataPacks->set(u8"Enabled", enabled);
      j[u8"DataPacks"] = dataPacks;
    }
    {
      auto brands = List<Tag::Type::String>();
      brands->push_back(String(u8"vanilla"));
      j[u8"ServerBrands"] = brands;
    }
    {
      auto version = Compound();
      version->set(u8"Id", Int(Chunk::kTargetDataVersion));
      version->set(u8"Name", Chunk::TargetVersionString());
      version->set(u8"Series", u8"main");
      version->set(u8"Snapshot", Bool(false));
      j[u8"Version"] = version;
    }

    auto worldGenSettings = Compound();
    if (in->boolean(u8"spawnBonusChest", false)) {
      worldGenSettings->set(u8"bonus_chest", Bool(true));
    }
    worldGenSettings->set(u8"generate_features", Bool(true));
    if (auto randomSeed = in->int64(u8"RandomSeed"); randomSeed) {
      worldGenSettings->set(u8"seed", Long(*randomSeed));
      auto dimensions = Compound();
      {
        auto overworld = Compound();
        auto generator = Compound();
        if (flatWorldSettings) {
          generator->set(u8"type", u8"minecraft:flat");
          generator->set(u8"settings", flatWorldSettings);
        } else {
          auto biomeSource = Compound();
          biomeSource->set(u8"preset", u8"minecraft:overworld");
          biomeSource->set(u8"type", u8"minecraft:multi_noise");
          generator->set(u8"biome_source", biomeSource);
          generator->set(u8"seed", Long(*randomSeed));
          generator->set(u8"settings", u8"minecraft:overworld");
          generator->set(u8"type", u8"minecraft:noise");
        }
        overworld->set(u8"generator", generator);
        overworld->set(u8"type", u8"minecraft:overworld");
        dimensions->set(u8"minecraft:overworld", overworld);
      }
      {
        auto end = Compound();
        auto generator = Compound();
        auto biomeSource = Compound();
        biomeSource->set(u8"type", u8"minecraft:the_end");
        biomeSource->set(u8"seed", Long(*randomSeed));
        generator->set(u8"biome_source", biomeSource);
        generator->set(u8"seed", Long(*randomSeed));
        generator->set(u8"settings", u8"minecraft:end");
        generator->set(u8"type", u8"minecraft:noise");
        end->set(u8"generator", generator);
        end->set(u8"type", u8"minecraft:the_end");
        dimensions->set(u8"minecraft:the_end", end);
      }
      {
        auto nether = Compound();
        auto generator = Compound();
        auto biomeSource = Compound();
        biomeSource->set(u8"preset", u8"minecraft:nether");
        biomeSource->set(u8"type", u8"minecraft:multi_noise");
        biomeSource->set(u8"seed", Long(*randomSeed));
        generator->set(u8"biome_source", biomeSource);
        generator->set(u8"seed", Long(*randomSeed));
        generator->set(u8"settings", u8"minecraft:nether");
        generator->set(u8"type", u8"minecraft:noise");
        nether->set(u8"generator", generator);
        nether->set(u8"type", u8"minecraft:the_nether");
        dimensions->set(u8"minecraft:the_nether", nether);
      }
      worldGenSettings->set(u8"dimensions", dimensions);
    }
    j[u8"WorldGenSettings"] = worldGenSettings;

    CopyBoolValues(*in, j, {{u8"allowCommands"}, {u8"DifficultyLocked"}, {u8"hardcore"}, {u8"initialized"}, {u8"raining"}, {u8"thundering"}});
    CopyByteValues(*in, j, {{u8"Difficulty"}});
    CopyIntValues(*in, j, {{u8"rainTime"}, {u8"GameType"}, {u8"SpawnX"}, {u8"SpawnY"}, {u8"SpawnZ"}, {u8"thunderTime"}, {u8"clearWeatherTime"}});
    CopyLongValues(*in, j, {{u8"DayTime"}, {u8"Time"}});
    CopyStringValues(*in, j, {{u8"LevelName"}});

    if (lastPlayed) {
      // NOTE: LastPlayed in the level.dat from xbox360 edition is wired. Use timestamp of savegame.dat for it
      i64 ms = chrono::duration_cast<chrono::milliseconds>(lastPlayed->time_since_epoch()).count();
      j[u8"LastPlayed"] = Long(ms);
    }

    j[u8"BorderCenterX"] = Double(0);
    j[u8"BorderCenterZ"] = Double(0);
    j[u8"BorderSize"] = Double(862);
    j[u8"BorderWarningBlocks"] = Double(0);

    if (auto dimensionData = in->compoundTag(u8"DimensionData"); dimensionData) {
      if (auto theEnd = dimensionData->compoundTag(u8"The End"); theEnd) {
        if (auto dragonFightB = theEnd->compoundTag(u8"DragonFight"); dragonFightB) {
          auto dragonFightJ = dragonFightB->copy();
          if (auto dragonUuidB = dragonFightB->string(u8"DragonUUID"); dragonUuidB) {
            if (auto dragonUuidJ = Entity::MigrateUuid(*dragonUuidB, ctx); dragonUuidJ) {
              dragonFightJ->set(u8"Dragon", dragonUuidJ->toIntArrayTag());
              dragonFightJ->erase(u8"DragonUUID");
            }
          }
          j[u8"DragonFight"] = dragonFightJ;
        }
      }
    }

    if (localPlayer) {
      j[u8"Player"] = localPlayer->fPlayer.copy();
    }

    auto outRoot = Compound();
    outRoot->set(u8"Data", data);
    auto outStream = make_shared<mcfile::stream::GzFileOutputStream>(datTo);
    if (CompoundTag::Write(*outRoot, outStream, mcfile::Encoding::Java)) {
      return Status::Ok();
    } else {
      return JE2BE_ERROR;
    }
  }

  static CompoundTagPtr FlatWorldSettings() {
    auto flatSettings = Compound();
    flatSettings->set(u8"biome", u8"minecraft:plains");
    flatSettings->set(u8"features", Bool(false));
    flatSettings->set(u8"lakes", Bool(false));
    auto structures = Compound();
    structures->set(u8"structures", Compound());
    flatSettings->set(u8"structures", structures);

    auto layers = List<Tag::Type::Compound>();

    auto bedrock = Compound();
    bedrock->set(u8"block", u8"minecraft:bedrock");
    bedrock->set(u8"height", Int(1));
    layers->push_back(bedrock);

    auto dirt = Compound();
    dirt->set(u8"block", u8"minecraft:dirt");
    dirt->set(u8"height", Int(2));
    layers->push_back(dirt);

    auto grass = Compound();
    grass->set(u8"block", u8"minecraft:grass_block");
    grass->set(u8"height", Int(1));
    layers->push_back(grass);

    flatSettings->set(u8"layers", layers);

    return flatSettings;
  }

  static Status CopyMapFiles(std::filesystem::path const &inputDirectory, std::filesystem::path const &outputDirectory) {
    namespace fs = std::filesystem;

    auto dataFrom = inputDirectory / "data";
    auto dataTo = outputDirectory / "data";
    if (!Fs::Exists(dataFrom)) {
      return Status::Ok();
    }
    if (!Fs::CreateDirectories(dataTo)) {
      return JE2BE_ERROR;
    }
    for (DirectoryIterator it(dataFrom); it.valid(); it.next()) {
      if (!it->is_regular_file()) {
        continue;
      }
      auto fileName = it->path().filename();
      auto fileNameString = fileName.u8string();
      if (!fileNameString.starts_with(u8"map_") || !fileNameString.ends_with(u8".dat")) {
        continue;
      }
      auto numberString = strings::RemovePrefixAndSuffix(u8"map_", fileNameString, u8".dat");
      auto number = strings::ToI32(numberString);
      if (!number) {
        continue;
      }
      std::error_code ec1;
      fs::copy_options o = fs::copy_options::overwrite_existing;
      fs::copy_file(dataFrom / fileNameString, dataTo / fileNameString, o, ec1);
      if (ec1) {
        return JE2BE_ERROR_WHAT(ec1.message());
      }
    }
    return Status::Ok();
  }
};

Status Converter::Run(std::vector<uint8_t> const &inputSaveBin,
                      std::filesystem::path const &outputDirectory,
                      unsigned int concurrency,
                      Behavior const &behavior,
                      Options const &options,
                      Progress *progress) {
  return Impl::Run(inputSaveBin, outputDirectory, concurrency, behavior, options, progress);
}

} // namespace je2be::lce
