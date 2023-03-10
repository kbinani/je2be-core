#include <je2be/box360/converter.hpp>

#include <je2be/box360/options.hpp>
#include <je2be/box360/progress.hpp>
#include <je2be/defer.hpp>
#include <je2be/fs.hpp>
#include <je2be/zip-file.hpp>

#include "_dimension-ext.hpp"
#include "_directory-iterator.hpp"
#include "_java-level-dat.hpp"
#include "_nbt-ext.hpp"
#include "_nullable.hpp"
#include "_props.hpp"
#include "box360/_chunk.hpp"
#include "box360/_context.hpp"
#include "box360/_entity.hpp"
#include "box360/_savegame.hpp"
#include "box360/_tile-entity.hpp"
#include "box360/_world.hpp"
#include "toje/_constants.hpp"

namespace je2be::box360 {

class Converter::Impl {
  Impl() = delete;

public:
  static Status Run(std::filesystem::path const &inputSaveBin,
                    std::filesystem::path const &outputDirectory,
                    unsigned int concurrency,
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

    optional<chrono::system_clock::time_point> lastPlayed;
    {
      vector<u8> buffer;
      auto savegameInfo = Savegame::ExtractSavagameFromSaveBin(inputSaveBin, buffer);
      if (!savegameInfo) {
        return JE2BE_ERROR;
      }
      if (auto thumbnail = savegameInfo->fThumbnailImage; thumbnail) {
        auto iconPath = outputDirectory / "icon.png";
        auto icon = make_shared<mcfile::stream::FileOutputStream>(iconPath);
        if (!icon->write(thumbnail->data(), thumbnail->size())) {
          return JE2BE_ERROR;
        }
      }
      lastPlayed = savegameInfo->fCreatedTime;

      if (!Savegame::DecompressSavegame(buffer)) {
        return JE2BE_ERROR;
      }
      if (!Savegame::ExtractFilesFromDecompressedSavegame(buffer, *temp)) {
        return JE2BE_ERROR;
      }
    }
    Context ctx(TileEntity::Convert, Entity::MigrateName);
    if (auto st = CopyMapFiles(*temp, outputDirectory); !st.ok()) {
      return st;
    }
    auto copyPlayersResult = CopyPlayers(*temp, outputDirectory, ctx, options);
    if (!copyPlayersResult) {
      return copyPlayersResult.status();
    }
    if (auto st = CopyLevelDat(*temp, outputDirectory, lastPlayed, copyPlayersResult->fLocalPlayer, ctx); !st.ok()) {
      return st;
    }
    if (auto st = SetupResourcePack(outputDirectory); !st.ok()) {
      return st;
    }
    if (auto st = SetupDataPack(outputDirectory); !st.ok()) {
      return st;
    }
    int progressChunksOffset = 0;
    for (auto dimension : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
      defer {
        progressChunksOffset += World::kProgressWeightPerWorld;
      };
      if (!options.fDimensionFilter.empty()) {
        if (options.fDimensionFilter.find(dimension) == options.fDimensionFilter.end()) {
          continue;
        }
      }
      if (auto st = World::Convert(*temp, outputDirectory, dimension, concurrency, ctx, options, progress, progressChunksOffset); !st.ok()) {
        return st;
      }
    }
    return Status::Ok();
  }

private:
  struct PlayerInfo {
    Uuid fUuidJ;
    CompoundTag fPlayer;
    std::string fUuidX;
  };

  struct CopyPlayersResult {
    std::shared_ptr<PlayerInfo> fLocalPlayer;
  };

  static Nullable<CopyPlayersResult> CopyPlayers(std::filesystem::path const &inputDirectory,
                                                 std::filesystem::path const &outputDirectory,
                                                 Context &ctx,
                                                 Options const &options) {
    using namespace std;
    namespace fs = std::filesystem;

    auto playersFrom = inputDirectory / "players";
    auto playersTo = outputDirectory / "playerdata";

    if (!Fs::CreateDirectories(playersTo)) {
      return JE2BE_NULLABLE_NULL;
    }

    CopyPlayersResult r;
    vector<shared_ptr<PlayerInfo>> players;

    for (DirectoryIterator it(playersFrom); it.valid(); it.next()) {
      if (!it->is_regular_file()) {
        continue;
      }
      auto player = CopyPlayer(it->path(), playersTo, ctx);
      if (!player) {
        continue;
      }
      players.push_back(player);
    }
    if (players.size() == 1) {
      r.fLocalPlayer = players[0];
      players.clear();
    }

    if (r.fLocalPlayer) {
      if (options.fLocalPlayer) {
        r.fLocalPlayer->fPlayer.set("UUID", options.fLocalPlayer->toIntArrayTag());
        ctx.fPlayers.insert(make_pair(r.fLocalPlayer->fUuidX, *options.fLocalPlayer));
      } else {
        ctx.fPlayers.insert(make_pair(r.fLocalPlayer->fUuidX, r.fLocalPlayer->fUuidJ));
      }
      players.push_back(r.fLocalPlayer);
    }

    for (auto const &player : players) {
      auto filename = player->fUuidJ.toString() + ".dat";
      auto filePath = playersTo / filename;
      if (!CompoundTag::Write(player->fPlayer, filePath, mcfile::Endian::Big)) {
        return JE2BE_NULLABLE_NULL;
      }
    }

    return r;
  }

  static std::shared_ptr<PlayerInfo> CopyPlayer(std::filesystem::path const &inputFile, std::filesystem::path const &outputPlayerdata, Context const &ctx) {
    using namespace std;
    auto stream = make_shared<mcfile::stream::FileInputStream>(inputFile);
    auto in = CompoundTag::Read(stream, mcfile::Endian::Big);
    stream.reset();
    if (!in) {
      // Not an nbt format. just skip this
      return nullptr;
    }

    auto r = make_shared<PlayerInfo>();
    r->fPlayer.fValue.swap(in->copy()->fValue);

    r->fPlayer.erase("GamePrivileges");
    r->fPlayer.erase("Sleeping");

    Entity::CopyItems(*in, r->fPlayer, ctx, "EnderItems");
    Entity::CopyItems(*in, r->fPlayer, ctx, "Inventory");

    if (auto dimensionB = in->int32("Dimension"); dimensionB) {
      if (auto dimension = DimensionFromXbox360Dimension(*dimensionB); dimension) {
        r->fPlayer.set("Dimension", String(JavaStringFromDimension(*dimension)));
      }
    }

    r->fPlayer.set("DataVersion", Int(Chunk::kTargetDataVersion));

    if (auto rootVehicleB = in->compoundTag("RootVehicle"); rootVehicleB) {
      auto rootVehicleJ = Compound();
      if (auto entityB = rootVehicleB->compoundTag("Entity"); entityB) {
        if (auto entityJ = Entity::Convert(*entityB, ctx); entityJ && entityJ->fEntity) {
          rootVehicleJ->set("Entity", entityJ->fEntity);
        }
      }
      if (auto attachB = rootVehicleB->string("Attach"); attachB) {
        if (auto attachJ = Entity::MigrateUuid(*attachB, ctx); attachJ) {
          rootVehicleJ->set("Attach", attachJ->toIntArrayTag());
        }
      }
      r->fPlayer.set("RootVehicle", rootVehicleJ);
    }

    shared_ptr<Uuid> uuidJ;
    if (auto uuidB = in->string("UUID"); uuidB) {
      if (auto migrated = Entity::MigrateUuid(*uuidB, ctx); migrated) {
        uuidJ = make_shared<Uuid>(*migrated);
        r->fUuidX = *uuidB;
      }
    }
    if (!uuidJ) {
      auto name = inputFile.filename().replace_extension().string();
      // TODO: should this be converted to "entNNNN.." format?
      if (auto id = strings::ToU64(name, 10); id) {
        auto generated = Uuid::GenWithU64Seed(*id);
        uuidJ = make_shared<Uuid>(generated);
        r->fUuidX = "je2be_internal_" + name;
      }
    }
    if (!uuidJ) {
      return nullptr;
    }
    r->fPlayer.set("UUID", uuidJ->toIntArrayTag());

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
      pack["pack_format"] = 7;
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
      pack["pack_format"] = 11;
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
      o["ultrawarm"] = true;
      o["natural"] = false;
      o["coordinate_scale"] = 3.0;
      o["has_skylight"] = false;
      o["has_ceiling"] = true;
      o["ambient_light"] = 0.1;
      o["fixed_time"] = 18000;
      o["monster_spawn_light_level"] = 11;
      o["monster_spawn_block_light_limit"] = 15;
      o["piglin_safe"] = true;
      o["bed_works"] = false;
      o["respawn_anchor_works"] = true;
      o["has_raids"] = false;
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
                             Context const &ctx) {
    using namespace std;
    namespace fs = std::filesystem;
    auto datFrom = inputDirectory / "level.dat";
    auto datTo = outputDirectory / "level.dat";
    if (!Fs::Exists(datFrom)) {
      return JE2BE_ERROR;
    }

    auto inStream = make_shared<mcfile::stream::GzFileInputStream>(datFrom);
    auto inRoot = CompoundTag::Read(inStream, mcfile::Endian::Big);
    inStream.reset();
    if (!inRoot) {
      return JE2BE_ERROR;
    }
    auto in = inRoot->compoundTag("Data");
    if (!in) {
      return JE2BE_ERROR;
    }
    bool newSeaLevel = in->boolean("newSeaLevel", false);
    auto flat = in->string("generatorName") == "flat";

    JavaLevelDat::Options o;
    o.fBonusChestEnabled = in->boolean("spawnBonusChest");
    o.fDataVersion = Chunk::kTargetDataVersion;
    o.fRandomSeed = in->int64("RandomSeed");
    o.fVersionString = Chunk::TargetVersionString();
    if (flat) {
      // auto generatorVersion = in->int32("generatorVersion");
      // generatorVersion
      // 0: TU9, TU67, TU74

      o.fFlatWorldSettings = FlatWorldSettings();
    } else {
      o.fFlatWorldSettings = FlatWorldSettingsForOverworldOuterRegion(newSeaLevel);
    }
    o.fEnabledDataPacks.push_back("file/nether3x");
    auto out = JavaLevelDat::TemplateData(o);

    CopyBoolValues(*in, *out, {{"allowCommands"}, {"DifficultyLocked"}, {"hardcore"}, {"initialized"}, {"raining"}, {"thundering"}});
    CopyByteValues(*in, *out, {{"Difficulty"}});
    CopyIntValues(*in, *out, {{"rainTime"}, {"GameType"}, {"SpawnX"}, {"SpawnY"}, {"SpawnZ"}, {"thunderTime"}, {"clearWeatherTime"}});
    CopyLongValues(*in, *out, {{"DayTime"}, {"Time"}});
    CopyStringValues(*in, *out, {{"LevelName"}});

    if (lastPlayed) {
      // NOTE: LastPlayed in the level.dat from xbox360 edition is wired. Use timestamp of savegame.dat for it
      i64 ms = chrono::duration_cast<chrono::milliseconds>(lastPlayed->time_since_epoch()).count();
      out->set("LastPlayed", Long(ms));
    }

    out->set("BorderCenterX", Double(0));
    out->set("BorderCenterZ", Double(0));
    out->set("BorderSize", Double(992));
    out->set("BorderWarningBlocks", Double(0));

    if (auto dimensionData = in->compoundTag("DimensionData"); dimensionData) {
      if (auto theEnd = dimensionData->compoundTag("The End"); theEnd) {
        if (auto dragonFightB = theEnd->compoundTag("DragonFight"); dragonFightB) {
          auto dragonFightJ = dragonFightB->copy();
          if (auto dragonUuidB = dragonFightB->string("DragonUUID"); dragonUuidB) {
            if (auto dragonUuidJ = Entity::MigrateUuid(*dragonUuidB, ctx); dragonUuidJ) {
              dragonFightJ->set("Dragon", dragonUuidJ->toIntArrayTag());
              dragonFightJ->erase("DragonUUID");
            }
          }
          out->set("DragonFight", dragonFightJ);
        }
      }
    }

    if (localPlayer) {
      out->set("Player", localPlayer->fPlayer.copy());
    }

    auto outRoot = Compound();
    outRoot->set("Data", out);
    auto outStream = make_shared<mcfile::stream::GzFileOutputStream>(datTo);
    if (CompoundTag::Write(*outRoot, outStream, mcfile::Endian::Big)) {
      return Status::Ok();
    } else {
      return JE2BE_ERROR;
    }
  }

  static CompoundTagPtr FlatWorldSettings() {
    auto flatSettings = Compound();
    flatSettings->set("biome", String("minecraft:plains"));
    flatSettings->set("features", Bool(false));
    flatSettings->set("lakes", Bool(false));
    auto structures = Compound();
    structures->set("structures", Compound());
    flatSettings->set("structures", structures);

    auto layers = List<Tag::Type::Compound>();

    auto air = Compound();
    air->set("block", String("minecraft:air"));
    air->set("height", Int(64));
    layers->push_back(air);

    auto bedrock = Compound();
    bedrock->set("block", String("minecraft:bedrock"));
    bedrock->set("height", Int(1));
    layers->push_back(bedrock);

    auto dirt = Compound();
    dirt->set("block", String("minecraft:dirt"));
    dirt->set("height", Int(2));
    layers->push_back(dirt);

    auto grass = Compound();
    grass->set("block", String("minecraft:grass_block"));
    grass->set("height", Int(1));
    layers->push_back(grass);

    flatSettings->set("layers", layers);

    return flatSettings;
  }

  static CompoundTagPtr FlatWorldSettingsForOverworldOuterRegion(bool newSeaLevel) {
    auto flatSettings = Compound();
    flatSettings->set("biome", String("minecraft:ocean"));
    flatSettings->set("features", Bool(false));
    flatSettings->set("lakes", Bool(false));
    auto structures = Compound();
    structures->set("structures", Compound());
    flatSettings->set("structures", structures);

    auto layers = List<Tag::Type::Compound>();

    auto air = Compound();
    air->set("block", String("minecraft:air"));
    air->set("height", Int(64));
    layers->push_back(air);

    auto bedrock = Compound();
    bedrock->set("block", String("minecraft:bedrock"));
    bedrock->set("height", Int(1));
    layers->push_back(bedrock);

    auto stone = Compound();
    stone->set("block", String("minecraft:stone"));
    stone->set("height", Int(53));
    layers->push_back(stone);

    auto water = Compound();
    water->set("block", String("minecraft:water"));
    water->set("height", Int(newSeaLevel ? 9 : 10));
    layers->push_back(water);

    flatSettings->set("layers", layers);

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
      auto fileNameString = fileName.string();
      if (!fileNameString.starts_with("map_") || !fileNameString.ends_with(".dat")) {
        continue;
      }
      auto numberString = strings::RTrim(strings::LTrim(fileNameString, "map_"), ".dat");
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

Status Converter::Run(std::filesystem::path const &inputSaveBin,
                      std::filesystem::path const &outputDirectory,
                      unsigned int concurrency,
                      Options const &options,
                      Progress *progress) {
  return Impl::Run(inputSaveBin, outputDirectory, concurrency, options, progress);
}

} // namespace je2be::box360
