#pragma once

#include "_java-level-dat.hpp"
#include "bedrock/_block-data.hpp"
#include "bedrock/_entity.hpp"
#include "java/_versions.hpp"

#include <iostream>

namespace je2be::bedrock {

class LevelData {
public:
  struct GameRules {
    static CompoundTagPtr Import(CompoundTag const &b, mcfile::be::DbInterface &db, mcfile::Encoding encoding) {
      auto ret = Compound();
      CompoundTag &j = *ret;
#define B(__nameJ, __nameB, __default) j[u8"" #__nameJ] = String(b.boolean(u8"" #__nameB, __default) ? u8"true" : u8"false");
#define I(__nameJ, __nameB, __default) j[u8"" #__nameJ] = String(mcfile::String::ToString(b.int32(u8"" #__nameB, __default)));
      // announceAdvancements
      B(commandBlockOutput, commandblockoutput, true);
      // disableElytraMovementCheck
      // disableRaids
      B(doDaylightCycle, dodaylightcycle, true);
      B(doEntityDrops, doentitydrops, true);
      B(doFireTick, dofiretick, true);
      B(doImmediateRespawn, doimmediaterespawn, false);
      B(doInsomnia, doinsomnia, true);
      B(doLimitedCrafting, dolimitedcrafting, false);
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
      // NOTE: showcoordinates and reducedDebugInfo are not identical, but converting here for player convenience
      auto showcoordinates = b.boolean(u8"showcoordinates", true);
      j[u8"reducedDebugInfo"] = String(showcoordinates ? u8"false" : u8"true");
      B(sendCommandFeedback, sendcommandfeedback, true);
      B(showDeathMessages, showdeathmessages, true);
      I(spawnRadius, spawnradius, 10);
      // serverChunkTickRange
      // spectatorsGenerateChunks
      // universalAnger
      I(playersSleepingPercentage, playerssleepingpercentage, 100);
      B(projectilesCanBreakBlocks, projectilescanbreakblocks, true);
#undef B
#undef I

      if (auto mobEventsData = db.get(mcfile::be::DbKey::MobEvents()); mobEventsData) {
        if (auto mobEvents = CompoundTag::Read(*mobEventsData, encoding); mobEvents) {
          auto doPatrolSpawning = mobEvents->boolean(u8"minecraft:pillager_patrols_event", true);
          auto doTraderSpawning = mobEvents->boolean(u8"minecraft:wandering_trader_event", true);

          ret->set(u8"doPatrolSpawning", doPatrolSpawning ? u8"true" : u8"false");
          ret->set(u8"doTraderSpawning", doTraderSpawning ? u8"true" : u8"false");
        }
      }

      ret->set(u8"enderPearlsVanishOnDeath", u8"true");

      return ret;
    }
  };

public:
  static bool Read(std::filesystem::path levelDatFile, CompoundTagPtr &result) {
    using namespace std;
    using namespace mcfile::stream;
    auto fis = make_shared<FileInputStream>(levelDatFile);
    if (!fis->valid()) {
      return false;
    }
    u16 versionLo = 0;
    u16 versionHi = 0;
    if (sizeof(versionLo) != fis->read(&versionLo, sizeof(versionLo))) {
      return false;
    }
    if (sizeof(versionHi) != fis->read(&versionHi, sizeof(versionHi))) {
      return false;
    }
    u32 size = 0;
    if (fis->read(&size, sizeof(size)) != sizeof(size)) {
      return false;
    }

    if (!fis->seek(8)) {
      return false;
    }
    InputStreamReader isr(fis, mcfile::Encoding::LittleEndian);
    auto tag = CompoundTag::Read(isr);
    if (tag) {
      result.swap(tag);
      return true;
    }
    return false;
  }

  static CompoundTagPtr Import(CompoundTag const &b, mcfile::be::DbInterface &db, Options opt, Context &ctx) {
    using namespace std;

    JavaLevelDat::Options o;
    o.fDataVersion = bedrock::kDataVersion;
    o.fRandomSeed = b.int64(u8"RandomSeed");
    o.fVersionString = kVersionString;
    o.fFlatWorldSettings = FlatWorldSettings(b);
    o.fBonusChestEnabled = b.boolean(u8"bonusChestEnabled");

    if (auto experiments = b.compoundTag(u8"experiments"); experiments) {
      if (experiments->boolean(u8"villager_trades_rebalance")) {
        ctx.fDataPackTradeRebalance = true;
      }
    }

    auto data = JavaLevelDat::TemplateData(o);

    CompoundTag &j = *data;

    CopyStringValues(b, j, {{u8"LevelName"}});
    CopyIntValues(b, j, {{u8"SpawnX"}, {u8"SpawnY"}, {u8"SpawnZ"}, {u8"rainTime"}, {u8"lightningTime", u8"thunderTime"}});
    CopyBoolValues(b, j, {{u8"commandsEnabled", u8"allowCommands", false}, {u8"IsHardcore", u8"hardcore"}});
    CopyLongValues(b, j, {{u8"Time", u8"DayTime"}, {u8"currentTick", u8"Time"}});

    j[u8"Difficulty"] = Byte(b.int32(u8"Difficulty", 2));
    j[u8"GameRules"] = GameRules::Import(b, db, ctx.fEncoding);
    j[u8"LastPlayed"] = Long(b.int64(u8"LastPlayed", 0) * 1000);
    j[u8"raining"] = Bool(b.float32(u8"rainLevel", 0) >= 1);
    j[u8"thundering"] = Bool(b.float32(u8"lightningLevel", 0) >= 1);
    j[u8"GameType"] = Int(JavaFromGameMode(ctx.fGameMode));
    j[u8"initialized"] = Bool(true);

    if (auto dragonFight = DragonFight(db, ctx.fEncoding); dragonFight) {
      j[u8"DragonFight"] = dragonFight;
    }

    if (auto playerData = Player(db, ctx, opt.fLocalPlayer.get()); playerData) {
      ctx.setLocalPlayerIds(playerData->fEntityIdBedrock, playerData->fEntityIdJava);
      if (playerData->fShoulderEntityLeft) {
        ctx.setShoulderEntityLeft(*playerData->fShoulderEntityLeft);
      }
      if (playerData->fShoulderEntityRight) {
        ctx.setShoulderEntityRight(*playerData->fShoulderEntityRight);
      }
      j[u8"Player"] = playerData->fEntity;
    }

    auto root = Compound();
    root->set(u8"Data", data);
    return root;
  }

  static void UpdateDataPacksAndEnabledFeatures(CompoundTag &root, Context const &ctx) {
    auto data = root.compoundTag(u8"Data");
    if (!data) {
      data = Compound();
      root[u8"Data"] = data;
    }
    auto enabledFeatures = List<Tag::Type::String>();
    if (ctx.fDataPackBundle) {
      enabledFeatures->push_back(String(u8"minecraft:bundle"));
    }
    if (ctx.fDataPackTradeRebalance) {
      enabledFeatures->push_back(String(u8"minecraft:trade_rebalance"));
    }
    enabledFeatures->push_back(String(u8"minecraft:vanilla"));
    data->set(u8"enabled_features", enabledFeatures);

    if (auto dataPacks = data->compoundTag(u8"DataPacks"); dataPacks) {
      if (auto enabledDataPacks = dataPacks->listTag(u8"Enabled"); enabledDataPacks) {
        if (ctx.fDataPackBundle) {
          enabledDataPacks->push_back(String(u8"bundle"));
        }
        if (ctx.fDataPackTradeRebalance) {
          enabledDataPacks->push_back(String(u8"trade_rebalance"));
        }
      }
      if (auto disabledDataPacks = dataPacks->listTag(u8"Disabled"); disabledDataPacks) {
        if (!ctx.fDataPackBundle) {
          disabledDataPacks->push_back(String(u8"bundle"));
        }
        if (!ctx.fDataPackTradeRebalance) {
          disabledDataPacks->push_back(String(u8"trade_rebalance"));
        }
      }
    }
  }

  static CompoundTagPtr FlatWorldSettings(CompoundTag const &b) {
    using namespace std;
    if (b.int32(u8"Generator") != 2) {
      return nullptr;
    }
    auto settings = Compound();
    settings->set(u8"features", Bool(false));
    settings->set(u8"lakes", Bool(false));
    auto flatWorldLayers = b.string(u8"FlatWorldLayers");
    if (!flatWorldLayers) {
      return nullptr;
    }
    auto parsed = props::ParseAsJson(*flatWorldLayers);
    if (!parsed) {
      return nullptr;
    }
    auto const &json = *parsed;
    auto encodingVersion = json.find("encoding_version");
    if (encodingVersion == json.end()) {
      return nullptr;
    }
    if (!encodingVersion->is_number_integer()) {
      return nullptr;
    }
    int encodingVersionV = encodingVersion->get<int>();
    auto biomeB = json.find("biome_id");
    if (biomeB == json.end()) {
      return nullptr;
    }
    if (!biomeB->is_number_unsigned()) {
      return nullptr;
    }
    auto biomeJ = mcfile::be::Biome::FromUint32(biomeB->get<u32>());
    assert(biomeJ != mcfile::biomes::unknown);

    settings->set(u8"biome", mcfile::biomes::Biome::Name(biomeJ, bedrock::kDataVersion, u8"minecraft:plains"));
    auto layersB = json.find("block_layers");
    if (layersB == json.end()) {
      return nullptr;
    }
    if (!layersB->is_array()) {
      return nullptr;
    }
    auto layersJ = List<Tag::Type::Compound>();
    if (encodingVersionV < 6) {
      auto under0LayerJ = Compound();
      under0LayerJ->set(u8"block", u8"minecraft:air");
      under0LayerJ->set(u8"height", Int(64));
      layersJ->push_back(under0LayerJ);
    }
    for (auto const &it : *layersB) {
      auto count = it.find("count");
      if (count == it.end()) {
        return nullptr;
      }
      if (!count->is_number_unsigned()) {
        return nullptr;
      }
      shared_ptr<mcfile::be::Block> blockB;
      auto blockName = it.find("block_name");
      auto blockId = it.find("block_id");
      if (blockId == it.end() && blockName == it.end()) {
        return nullptr;
      }
      if (blockName == it.end()) {
        if (!blockId->is_number_unsigned()) {
          return nullptr;
        }
        auto id = blockId->get<uint64_t>();
        if (id >= 256) {
          return nullptr;
        }
        auto name = mcfile::be::Block::BlockNameById((uint8_t)id);
        optional<int16_t> val;
        auto blockData = it.find("block_data");
        if (blockData != it.end() && blockData->is_number_integer()) {
          val = blockData->get<int16_t>();
        }
        blockB = make_shared<mcfile::be::Block>(name, Compound(), nullopt, val);
      } else {
        if (!blockName->is_string()) {
          return nullptr;
        }
        blockB = make_shared<mcfile::be::Block>(props::GetJsonStringValue(*blockName), Compound(), je2be::java::kBlockDataVersion);
      }
      auto blockJ = BlockData::From(*blockB, bedrock::kDataVersion);
      if (!blockJ) {
        return nullptr;
      }
      auto layerJ = Compound();
      layerJ->set(u8"block", u8string(blockJ->fName));
      layerJ->set(u8"height", Int(count->get<u32>()));
      layersJ->push_back(layerJ);
    }
    settings->set(u8"layers", layersJ);
    auto structures = Compound();
    structures->set(u8"structures", Compound());
    settings->set(u8"structures", structures);
    return settings;
  }

  static std::optional<Entity::LocalPlayerData> Player(mcfile::be::DbInterface &db, Context &ctx, Uuid const *uuid) {
    auto str = db.get(mcfile::be::DbKey::LocalPlayer());
    if (!str) {
      return std::nullopt;
    }

    auto tag = CompoundTag::Read(*str, ctx.fEncoding);
    if (!tag) {
      return std::nullopt;
    }

    return Entity::LocalPlayer(*tag, ctx, uuid, bedrock::kDataVersion);
  }

  static CompoundTagPtr DragonFight(mcfile::be::DbInterface &db, mcfile::Encoding encoding) {
    auto str = db.get(mcfile::be::DbKey::TheEnd());
    if (!str) {
      return nullptr;
    }
    auto tag = CompoundTag::Read(*str, encoding);
    if (!tag) {
      return nullptr;
    }
    auto data = tag->compoundTag(u8"data");
    if (!data) {
      return nullptr;
    }
    auto fightB = data->compoundTag(u8"DragonFight");
    if (!fightB) {
      return nullptr;
    }
    auto fightJ = Compound();
    if (auto exitPortalLocation = props::GetPos3iFromListTag(*fightB, u8"ExitPortalLocation"); exitPortalLocation) {
      auto exitPortalLocationJ = Compound();
      exitPortalLocationJ->set(u8"X", Int(exitPortalLocation->fX));
      exitPortalLocationJ->set(u8"Y", Int(exitPortalLocation->fY));
      exitPortalLocationJ->set(u8"Z", Int(exitPortalLocation->fZ));
      fightJ->set(u8"ExitPortalLocation", exitPortalLocationJ);
    }
    CopyBoolValues(*fightB, *fightJ, {{u8"DragonKilled"}, {u8"PreviouslyKilled"}});
    if (auto dragonUidB = fightB->int64(u8"DragonUUID"); dragonUidB) {
      auto dragonUidJ = Uuid::GenWithI64Seed(*dragonUidB);
      fightJ->set(u8"Dragon", dragonUidJ.toIntArrayTag());
    }
    if (auto gatewaysB = fightB->listTag(u8"Gateways"); gatewaysB) {
      auto gatewaysJ = std::make_shared<IntArrayTag>();
      for (auto const &it : *gatewaysB) {
        auto gatewayJ = it->asInt();
        if (!gatewayJ) {
          gatewaysJ.reset();
          break;
        }
        gatewaysJ->fValue.push_back(gatewayJ->fValue);
      }
      if (gatewaysJ) {
        fightJ->set(u8"Gateways", gatewaysJ);
      }
    }
    fightJ->set(u8"NeedsStateScanning", Bool(false));
    return fightJ;
  }

  [[nodiscard]] static bool Write(CompoundTag const &tag, std::filesystem::path file) {
    auto gzs = std::make_shared<mcfile::stream::GzFileOutputStream>(file);
    return CompoundTag::Write(tag, gzs, mcfile::Encoding::Java);
  }
};

} // namespace je2be::bedrock
