#pragma once

namespace je2be::toje {

class LevelData {
public:
  struct GameRules {
    static CompoundTagPtr Import(CompoundTag const &b, leveldb::DB &db, mcfile::Endian endian) {
      auto ret = Compound();
      CompoundTag &j = *ret;
#define B(__nameJ, __nameB, __default) j[#__nameJ] = String(b.boolean(#__nameB, __default) ? "true" : "false");
#define I(__nameJ, __nameB, __default) j[#__nameJ] = String(std::to_string(b.int32(#__nameB, __default)));
      // announceAdvancements
      B(commandBlockOutput, commandblockoutput, true);
      // disableElytraMovementCheck
      // disableRaids
      B(doDaylightCycle, dodaylightcycle, true);
      B(doEntityDrops, doentitydrops, true);
      B(doFireTick, dofiretick, true);
      B(doImmediateRespawn, doimmediaterespawn, false);
      B(doInsomnia, doinsomnia, true);
      // doLimitedCrafting
      B(doMobLoot, domobloot, true);
      B(doMobSpawning, domobspawning, true);
      // doPatrolSpawning
      B(doTileDrops, dotiledrops, true);
      // doTraderSpawning
      // doWardenSpawning
      B(doWeatherCycle, doweathercycle, true);
      B(drowningDamage, drowningdamage, true);
      B(fallDamage, falldamage, true);
      B(fireDamage, firedamage, true);
      // forgiveDeadPlayers
      B(freezeDamage, freezedamage, true);
      B(keepInventory, keepinventory, false);
      // logAdminCommands
      I(maxCommandChainLength, maxcommandchainlength, 65536);
      // maxEntityCramming
      B(mobGriefing, mobgriefing, true);
      B(naturalRegeneration, naturalregeneration, true);
      I(randomTickSpeed, randomtickspeed, 3);
      // reducedDebugInfo
      B(sendCommandFeedback, sendcommandfeedback, true);
      B(showDeathMessages, showdeathmessages, true);
      I(spawnRadius, spawnradius, 10);
      // spectatorsGenerateChunks
      // universalAnger
#undef B
#undef I

      std::string mobEventsData;
      if (auto st = db.Get(leveldb::ReadOptions{}, mcfile::be::DbKey::MobEvents(), &mobEventsData); st.ok()) {
        if (auto mobEvents = CompoundTag::Read(mobEventsData, endian); mobEvents) {
          auto doPatrolSpawning = mobEvents->boolean("minecraft:pillager_patrols_event", true);
          auto doTraderSpawning = mobEvents->boolean("minecraft:wandering_trader_event", true);

          ret->set("doPatrolSpawning", String(doPatrolSpawning ? "true" : "false"));
          ret->set("doTraderSpawning", String(doTraderSpawning ? "true" : "false"));
        }
      }

      return ret;
    }
  };

public:
  static std::optional<mcfile::Endian> Read(std::filesystem::path levelDatFile, CompoundTagPtr &result) {
    using namespace std;
    using namespace mcfile::stream;
    auto fis = make_shared<FileInputStream>(levelDatFile);
    if (!fis) {
      return nullopt;
    }
    uint16_t versionLo = 0;
    uint16_t versionHi = 0;
    if (sizeof(versionLo) != fis->read(&versionLo, sizeof(versionLo))) {
      return nullopt;
    }
    if (sizeof(versionHi) != fis->read(&versionHi, sizeof(versionHi))) {
      return nullopt;
    }
    vector<mcfile::Endian> endians;
    if (versionLo == 0 && versionHi == 0) {
      return nullopt;
    } else {
      if (versionLo > 0) {
        endians.push_back(mcfile::Endian::Little);
        endians.push_back(mcfile::Endian::Big);
      } else {
        endians.push_back(mcfile::Endian::Big);
        endians.push_back(mcfile::Endian::Little);
      }
      uint32_t size = 0;
      if (fis->read(&size, sizeof(size)) != sizeof(size)) {
        return nullopt;
      }
    }

    for (mcfile::Endian e : endians) {
      if (!fis->seek(8)) {
        return nullopt;
      }
      InputStreamReader isr(fis, e);
      auto tag = CompoundTag::Read(isr);
      if (tag) {
        result.swap(tag);
        return e;
      }
    }
    return nullopt;
  }

  static CompoundTagPtr Import(CompoundTag const &b, leveldb::DB &db, Options opt, Context &ctx) {
    using namespace std;

    JavaLevelDat::Options o;
    o.fDataVersion = toje::kDataVersion;
    o.fRandomSeed = b.int64("RandomSeed");
    o.fVersionString = kVersionString;
    o.fFlatWorldSettings = FlatWorldSettings(b);
    o.fBonusChestEnabled = b.boolean("bonusChestEnabled");

    auto data = JavaLevelDat::TemplateData(o);

    CompoundTag &j = *data;

    CopyStringValues(b, j, {{"LevelName"}});
    CopyIntValues(b, j, {{"SpawnX"}, {"SpawnY"}, {"SpawnZ"}, {"rainTime"}, {"lightningTime", "thunderTime"}});
    CopyBoolValues(b, j, {{"commandsEnabled", "allowCommands", false}});
    CopyLongValues(b, j, {{"Time", "DayTime"}, {"currentTick", "Time"}});

    j["Difficulty"] = Byte(b.int32("Difficulty", 2));
    j["GameRules"] = GameRules::Import(b, db, ctx.fEndian);
    j["LastPlayed"] = Long(b.int64("LastPlayed", 0) * 1000);
    j["raining"] = Bool(b.float32("rainLevel", 0) >= 1);
    j["thundering"] = Bool(b.float32("lightningLevel", 0) >= 1);
    j["GameType"] = Int(JavaFromGameMode(ctx.fGameMode));

    if (auto dragonFight = DragonFight(db, ctx.fEndian); dragonFight) {
      j["DragonFight"] = dragonFight;
    }

    if (auto playerData = Player(db, ctx, opt.fLocalPlayer); playerData) {
      ctx.setLocalPlayerIds(playerData->fEntityIdBedrock, playerData->fEntityIdJava);
      if (playerData->fShoulderEntityLeft) {
        ctx.setShoulderEntityLeft(*playerData->fShoulderEntityLeft);
      }
      if (playerData->fShoulderEntityRight) {
        ctx.setShoulderEntityRight(*playerData->fShoulderEntityRight);
      }
      j["Player"] = playerData->fEntity;
    }

    auto root = Compound();
    root->set("Data", data);
    return root;
  }

  static CompoundTagPtr FlatWorldSettings(CompoundTag const &b) {
    using namespace std;
    if (b.int32("Generator") != 2) {
      return nullptr;
    }
    auto settings = Compound();
    settings->set("features", Bool(false));
    settings->set("lakes", Bool(false));
    auto flatWorldLayers = b.string("FlatWorldLayers");
    if (!flatWorldLayers) {
      return nullptr;
    }
    auto parsed = props::ParseAsJson(*flatWorldLayers);
    if (!parsed) {
      return nullptr;
    }
    auto const &json = *parsed;
    auto biomeB = json["biome_id"];
    if (!biomeB.is_number_unsigned()) {
      return nullptr;
    }
    auto biomeJ = mcfile::be::Biome::FromUint32(biomeB.get<uint32_t>());
    if (biomeJ == mcfile::biomes::unknown) {
      return nullptr;
    }
    settings->set("biome", String(mcfile::biomes::Name(biomeJ, toje::kDataVersion)));
    auto layersB = json["block_layers"];
    if (!layersB.is_array()) {
      return nullptr;
    }
    auto layersJ = List<Tag::Type::Compound>();
    for (auto const &it : layersB) {
      auto blockName = it["block_name"];
      auto count = it["count"];
      if (!blockName.is_string() || !count.is_number_unsigned()) {
        return nullptr;
      }
      mcfile::be::Block blockB(blockName.get<string>(), Compound(), je2be::tobe::kBlockDataVersion);
      auto blockJ = BlockData::From(blockB);
      if (!blockJ) {
        return nullptr;
      }
      auto layerJ = Compound();
      layerJ->set("block", String(blockJ->fName));
      layerJ->set("height", Int(count.get<uint32_t>()));
      layersJ->push_back(layerJ);
    }
    settings->set("layers", layersJ);
    auto structures = Compound();
    structures->set("structures", Compound());
    settings->set("structures", structures);
    return settings;
  }

  static std::optional<Entity::LocalPlayerData> Player(leveldb::DB &db, Context &ctx, std::optional<Uuid> uuid) {
    std::string str;
    if (auto st = db.Get(leveldb::ReadOptions{}, mcfile::be::DbKey::LocalPlayer(), &str); !st.ok()) {
      return std::nullopt;
    }

    auto tag = CompoundTag::Read(str, ctx.fEndian);
    if (!tag) {
      return std::nullopt;
    }

    return Entity::LocalPlayer(*tag, ctx, uuid);
  }

  static CompoundTagPtr DragonFight(leveldb::DB &db, mcfile::Endian endian) {
    std::string str;
    if (auto st = db.Get(leveldb::ReadOptions{}, mcfile::be::DbKey::TheEnd(), &str); !st.ok()) {
      return nullptr;
    }
    auto tag = CompoundTag::Read(str, endian);
    if (!tag) {
      return nullptr;
    }
    auto data = tag->compoundTag("data");
    if (!data) {
      return nullptr;
    }
    auto fightB = data->compoundTag("DragonFight");
    if (!fightB) {
      return nullptr;
    }
    auto fightJ = Compound();
    if (auto exitPortalLocation = props::GetPos3iFromListTag(*fightB, "ExitPortalLocation"); exitPortalLocation) {
      auto exitPortalLocationJ = Compound();
      exitPortalLocationJ->set("X", Int(exitPortalLocation->fX));
      exitPortalLocationJ->set("Y", Int(exitPortalLocation->fY));
      exitPortalLocationJ->set("Z", Int(exitPortalLocation->fZ));
      fightJ->set("ExitPortalLocation", exitPortalLocationJ);
    }
    CopyBoolValues(*fightB, *fightJ, {{"DragonKilled"}, {"PreviouslyKilled"}});
    if (auto dragonUidB = fightB->int64("DragonUUID"); dragonUidB) {
      auto dragonUidJ = Uuid::GenWithI64Seed(*dragonUidB);
      fightJ->set("Dragon", dragonUidJ.toIntArrayTag());
    }
    if (auto gateways = fightB->listTag("Gateways"); gateways) {
      fightJ->set("Gateways", gateways);
    }
    fightJ->set("NeedsStateScanning", Bool(false));
    return fightJ;
  }

  [[nodiscard]] static bool Write(CompoundTag const &tag, std::filesystem::path file) {
    auto gzs = std::make_shared<mcfile::stream::GzFileOutputStream>(file);
    return CompoundTag::Write(tag, gzs, mcfile::Endian::Big);
  }
};

} // namespace je2be::toje
