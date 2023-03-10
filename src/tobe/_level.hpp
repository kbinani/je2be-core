#pragma once

#include "_props.hpp"
#include "_version.hpp"
#include "enums/_game-mode.hpp"
#include "tobe/_flat-world-layers.hpp"
#include "tobe/_player-abilities.hpp"
#include "tobe/_uuid-registrar.hpp"
#include "tobe/_versions.hpp"

namespace je2be::tobe {

class Level {
public:
  PlayerAbilities fAbilities;
  Version fMinimumCompatibleClientVersion = kMinimumCompatibleClientVersion;
  std::string fBaseGameVersion = "*";
  std::string fBiomeOverride = "";
  bool fBonusChestEnabled = false;
  bool fBonusChestSpawned = false;
  i8 fCenterMapsToOrigin = -7;
  bool fCommandBlockOutput = true;
  bool fCommandblocksEnabled = true;
  bool fCommandsEnabled = false;
  bool fConfirmedPlatformLockedContent = false;
  i64 fCurrentTick = 0;
  i32 fDifficulty = 2;
  std::string fFlatWorldLayers = "null\x0a";
  bool fDoDaylightCycle = true;
  bool fDoEntityDrops = true;
  bool fDoFireTick = true;
  bool fDoImmediateRespawn = false;
  bool fDoInsomnia = true;
  bool fDoMobLoot = true;
  bool fDoMobSpawning = true;
  bool fDoTileDrops = true;
  bool fDoWeatherCycle = true;
  bool fDrowningDamage = true;
  bool fEducationFeaturesEnabled = false;
  i32 fEduOffer = 0;
  std::unordered_map<std::string, bool> fExperiments;
  bool fFallDamage = true;
  bool fFireDamage = true;
  bool fFreezeDamage = true;
  bool fForceGameType = false;
  i32 fFunctionCommandLimit = 10000;
  GameMode fGameType = GameMode::Survival;
  i32 fGenerator = 1;
  bool fHasBeenLoadedInCreative = false;
  bool fHasLockedBehaviorPack = false;
  bool fHasLockedResourcePack = false;
  bool fImmutableWorld = false;
  std::string fInventoryVersion = kInventoryVersion;
  bool fLANBroadcast = true;
  bool fLANBroadcastIntent = true;
  i64 fLastPlayed = 0;
  std::string fLevelName = "";
  i32 fLimitedWorldOriginX = 1852;
  i32 fLimitedWorldOriginY = 32767;
  i32 fLimitedWorldOriginZ = 4;
  bool fMultiplayerGame = true;
  bool fMultiplayerGameIntent = true;
  bool fNaturalRegeneration = true;
  i32 fNetherScale = 8;
  i32 fNetworkVersion = kNetworkVersion;
  i32 fPlatform = 2;
  i32 fPlatformBroadcastIntent = 3;
  std::string fPrid = "";
  bool fPvp = true;
  float fRainLevel = 0;
  i32 fRainTime = 0;
  i64 fRandomSeed = 0;
  i32 fRandomTickSpeed = 1;
  bool fRequiresCopiedPackRemovalCheck = false;
  bool fSendCommandFeedback = true;
  i32 fServerChunkTickRange = 0;
  bool fShowCoordinates = false;
  bool fShowDeathMessages = true;
  bool fShowTags = true;
  bool fSpawnMobs = true;
  i32 fSpawnRadius = 5;
  bool fSpawnV1Villagers = false;
  i32 fSpawnX = 0;
  i32 fSpawnY = 0;
  i32 fSpawnZ = 0;
  bool fStartWithMapEnabled = false;
  i32 fStorageVersion = kStorageVersion;
  bool fTexturePacksRequired = false;
  i64 fTime = 0;
  bool fTntExplodes = true;
  bool fUseMsaGamertagsOnly = false;
  i64 fWorldStartCount = 0;
  i32 fXBLBroadcastIntent = 3;
  bool fIsFromLockedTemplate = false;
  bool fIsFromWorldTemplate = false;
  bool fIsSingleUseWorld = false;
  bool fIsWorldTemplateOptionLocked = false;
  bool fKeepInventory = false;
  bool fMobGriefing = true;
  Version fLastOpenedWithVersion = kSupportVersion;
  float fLightningLevel = 0;
  i32 fLightningTime = 0;
  i32 fLimitedWorldDepth = 16;
  i32 fLimitedWorldWidth = 16;
  i32 fMaxCommandChainLength = 65535;

  CompoundTagPtr toCompoundTag() const {
    auto root = Compound();
    root->insert({
        {"abilities", fAbilities.toCompoundTag()},
        {"MinimumCompatibleClientVersion", fMinimumCompatibleClientVersion.toListTag()},
        {"baseGameVersion", String(fBaseGameVersion)},
        {"BiomeOverride", String(fBiomeOverride)},
        {"bonusChestEnabled", Bool(fBonusChestEnabled)},
        {"bonusChestSpawned", Bool(fBonusChestSpawned)},
        {"CenterMapsToOrigin", Byte(fCenterMapsToOrigin)},
        {"commandblockoutput", Bool(fCommandBlockOutput)},
        {"commandblocksenabled", Bool(fCommandblocksEnabled)},
        {"commandsEnabled", Bool(fCommandsEnabled)},
        {"ConfirmedPlatformLockedContent", Bool(fConfirmedPlatformLockedContent)},
        {"currentTick", Long(fCurrentTick)},
        {"Difficulty", Int(fDifficulty)},
        {"FlatWorldLayers", String(fFlatWorldLayers)},
        {"dodaylightcycle", Bool(fDoDaylightCycle)},
        {"doentitydrops", Bool(fDoEntityDrops)},
        {"dofiretick", Bool(fDoFireTick)},
        {"doimmediaterespawn", Bool(fDoImmediateRespawn)},
        {"doinsomnia", Bool(fDoInsomnia)},
        {"domobloot", Bool(fDoMobLoot)},
        {"domobspawning", Bool(fDoMobSpawning)},
        {"dotiledrops", Bool(fDoTileDrops)},
        {"doweathercycle", Bool(fDoWeatherCycle)},
        {"drowningdamage", Bool(fDrowningDamage)},
        {"educationFeaturesEnabled", Bool(fEducationFeaturesEnabled)},
        {"eduOffer", Int(fEduOffer)},
        {"falldamage", Bool(fFallDamage)},
        {"firedamage", Bool(fFireDamage)},
        {"freezedamage", Bool(fFreezeDamage)},
        {"ForceGameType", Bool(fForceGameType)},
        {"functioncommandlimit", Int(fFunctionCommandLimit)},
        {"GameType", Int(BedrockFromGameMode(fGameType))},
        {"Generator", Int(fGenerator)},
        {"hasBeenLoadedInCreative", Bool(fHasBeenLoadedInCreative)},
        {"hasLockedBehaviorPack", Bool(fHasLockedBehaviorPack)},
        {"hasLockedResourcePack", Bool(fHasLockedResourcePack)},
        {"immutableWorld", Bool(fImmutableWorld)},
        {"InventoryVersion", String(fInventoryVersion)},
        {"LANBroadcast", Bool(fLANBroadcast)},
        {"LANBroadcastIntent", Bool(fLANBroadcastIntent)},
        {"LastPlayed", Long(fLastPlayed)},
        {"LevelName", String(fLevelName)},
        {"LimitedWorldOriginX", Int(fLimitedWorldOriginX)},
        {"LimitedWorldOriginY", Int(fLimitedWorldOriginY)},
        {"LimitedWorldOriginZ", Int(fLimitedWorldOriginZ)},
        {"MultiplayerGame", Bool(fMultiplayerGame)},
        {"MultiplayerGameIntent", Bool(fMultiplayerGameIntent)},
        {"naturalregeneration", Bool(fNaturalRegeneration)},
        {"NetherScale", Int(fNetherScale)},
        {"NetworkVersion", Int(fNetworkVersion)},
        {"Platform", Int(fPlatform)},
        {"PlatformBroadcastIntent", Int(fPlatformBroadcastIntent)},
        {"prid", String(fPrid)},
        {"pvp", Bool(fPvp)},
        {"rainLevel", Float(fRainLevel)},
        {"rainTime", Int(fRainTime)},
        {"RandomSeed", Long(fRandomSeed)},
        {"randomtickspeed", Int(fRandomTickSpeed)},
        {"requiresCopiedPackRemovalCheck", Bool(fRequiresCopiedPackRemovalCheck)},
        {"sendcommandfeedback", Bool(fSendCommandFeedback)},
        {"serverChunkTickRange", Int(fServerChunkTickRange)},
        {"showcoordinates", Bool(fShowCoordinates)},
        {"showdeathmessages", Bool(fShowDeathMessages)},
        {"showtags", Bool(fShowTags)},
        {"spawnMobs", Bool(fSpawnMobs)},
        {"spawnradius", Int(fSpawnRadius)},
        {"SpawnV1Villagers", Bool(fSpawnV1Villagers)},
        {"SpawnX", Int(fSpawnX)},
        {"SpawnY", Int(fSpawnY)},
        {"SpawnZ", Int(fSpawnZ)},
        {"startWithMapEnabled", Bool(fStartWithMapEnabled)},
        {"StorageVersion", Int(fStorageVersion)},
        {"texturePacksRequired", Bool(fTexturePacksRequired)},
        {"Time", Long(fTime)},
        {"tntexplodes", Bool(fTntExplodes)},
        {"useMsaGamertagsOnly", Bool(fUseMsaGamertagsOnly)},
        {"worldStartCount", Long(fWorldStartCount)},
        {"XBLBroadcastIntent", Int(fXBLBroadcastIntent)},
        {"isFromLockedTemplate", Bool(fIsFromLockedTemplate)},
        {"isFromWorldTemplate", Bool(fIsFromWorldTemplate)},
        {"isSingleUseWorld", Bool(fIsSingleUseWorld)},
        {"isWorldTemplateOptionLocked", Bool(fIsWorldTemplateOptionLocked)},
        {"keepinventory", Bool(fKeepInventory)},
        {"mobgriefing", Bool(fMobGriefing)},
        {"lastOpenedWithVersion", fLastOpenedWithVersion.toListTag()},
        {"lightningLevel", Float(fLightningLevel)},
        {"lightningTime", Int(fLightningTime)},
        {"limitedWorldDepth", Int(fLimitedWorldDepth)},
        {"limitedWorldWidth", Int(fLimitedWorldWidth)},
        {"maxcommandchainlength", Int(fMaxCommandChainLength)},
    });
    if (!fExperiments.empty()) {
      auto experiments = Compound();
      experiments->set("experiments_ever_used", Bool(true));
      experiments->set("saved_with_toggled_experiments", Bool(true));
      for (auto const &it : fExperiments) {
        experiments->set(it.first, Bool(it.second));
      }
      root->set("experiments", experiments);
    }
    return root;
  }

  [[nodiscard]] bool write(std::filesystem::path path) const {
    auto stream = std::make_shared<mcfile::stream::FileOutputStream>(path);
    mcfile::stream::OutputStreamWriter w(stream, mcfile::Endian::Little);
    if (!w.write((u32)8)) {
      return false;
    }
    if (!w.write((u32)0)) {
      return false;
    }
    auto tag = this->toCompoundTag();
    if (!CompoundTag::Write(*tag, w)) {
      return false;
    }
    u64 pos = stream->pos();
    if (!stream->seek(4)) {
      return false;
    }
    return w.write((u32)pos - 8);
  }

  static Level Import(CompoundTag const &tag) {
    Level ret;
    auto data = tag.compoundTag("Data");
    if (!data) {
      return ret;
    }

#define I(__name, __type, __key) ret.__name = data->__type(__key, ret.__name)
    I(fCommandsEnabled, boolean, "allowCommands");
    I(fTime, int64, "DayTime");
    I(fDifficulty, byte, "Difficulty");
    I(fLevelName, string, "LevelName");
    I(fSpawnX, int32, "SpawnX");
    I(fSpawnY, int32, "SpawnY");
    I(fSpawnZ, int32, "SpawnZ");
    I(fCurrentTick, int64, "Time");
    ret.fLastPlayed = data->int64("LastPlayed", ret.fLastPlayed * 1000) / 1000;
#undef I

    auto gameRules = data->compoundTag("GameRules");
    if (gameRules) {
#define S(__name, __var)                       \
  auto __name = gameRules->stringTag(#__name); \
  if (__name) {                                \
    __var = __name->fValue == "true";          \
  }
#define I(__name, __var)                       \
  auto __name = gameRules->stringTag(#__name); \
  if (__name) {                                \
    auto v = strings::ToI32(__name->fValue);   \
    if (v) {                                   \
      __var = *v;                              \
    }                                          \
  }
      // announceAdvancements
      S(commandBlockOutput, ret.fCommandBlockOutput);
      // disableElytraMovementCheck
      // disableRaids
      S(doDaylightCycle, ret.fDoDaylightCycle);
      S(doEntityDrops, ret.fDoEntityDrops);
      S(doFireTick, ret.fDoFireTick);
      S(doImmediateRespawn, ret.fDoImmediateRespawn);
      S(doInsomnia, ret.fDoInsomnia);
      // doLimitedCrafting
      S(doMobLoot, ret.fDoMobLoot);
      S(doMobSpawning, ret.fDoMobSpawning);
      S(doTileDrops, ret.fDoTileDrops);
      // doWardenSpawning
      S(doWeatherCycle, ret.fDoWeatherCycle);
      S(drowningDamage, ret.fDrowningDamage);
      S(fallDamage, ret.fFallDamage);
      S(fireDamage, ret.fFireDamage);
      S(freezeDamage, ret.fFreezeDamage);
      // forgiveDeadPlayers
      S(keepInventory, ret.fKeepInventory);
      // logAdminCommands
      I(maxCommandChainLength, ret.fMaxCommandChainLength);
      // maxEntityCramming
      S(mobGriefing, ret.fMobGriefing);
      S(naturalRegeneration, ret.fNaturalRegeneration);
      I(randomTickSpeed, ret.fRandomTickSpeed);
      // reducedDebugInfo
      S(sendCommandFeedback, ret.fSendCommandFeedback);
      S(showDeathMessages, ret.fShowDeathMessages);
      I(spawnRadius, ret.fSpawnRadius);
      // spectatorsGenerateChunks
      // universalAnger
#undef S
#undef I
    }

    auto worldGenSettings = data->compoundTag("WorldGenSettings");
    if (worldGenSettings) {
      ret.fRandomSeed = worldGenSettings->int64("seed", ret.fRandomSeed);
    }

    auto flatWorldLayers = FlatWorldLayers::FromLevelData(*data);
    if (flatWorldLayers) {
      ret.fFlatWorldLayers = *flatWorldLayers;
      ret.fGenerator = 2;
    }

    ret.fRainLevel = data->boolean("raining", false) ? 1 : 0;
    ret.fRainTime = data->int32("rainTime", ret.fRainTime);
    ret.fLightningLevel = data->boolean("thundering", false) ? 1 : 0;
    ret.fLightningTime = data->int32("thunderTime", ret.fLightningTime);
    if (auto type = GameModeFromJava(data->int32("GameType", 0)); type) {
      ret.fGameType = *type;
    }

    if (auto enabledFeatures = data->listTag("enabled_features"); enabledFeatures) {
      bool experiments = false;
      for (auto it : *enabledFeatures) {
        auto s = it->asString();
        if (!s) {
          continue;
        }
        if (s->fValue != "minecraft:vanilla") {
          experiments = true;
          break;
        }
      }
      if (experiments) {
        ret.fExperiments["next_major_update"] = true;
      }
    }

    return ret;
  }

  static std::optional<std::string> TheEndData(CompoundTag const &tag, size_t numAutonomousEntities, std::unordered_set<Pos3i, Pos3iHasher> const &endPortalsInEndDimension) {
    auto data = tag.query("Data")->asCompound();
    if (!data) {
      return std::nullopt;
    }

    CompoundTag const *dragonFight = data->query("DragonFight")->asCompound();
    if (!dragonFight) {
      dragonFight = data->query("DimensionData/1/DragonFight")->asCompound();
    }
    if (!dragonFight) {
      return std::nullopt;
    }

    auto fight = Compound();

    fight->set("DragonFightVersion", Byte(kDragonFightVersion));

    auto killed = dragonFight->boolean("DragonKilled", false);
    fight->set("DragonKilled", Bool(numAutonomousEntities == 0 && killed));

    auto previouslyKilled = dragonFight->byteTag("PreviouslyKilled");
    if (previouslyKilled) {
      fight->set("PreviouslyKilled", Bool(previouslyKilled->fValue != 0));
    }

    auto uuid = props::GetUuid(*dragonFight, {.fIntArray = "Dragon"});
    if (uuid) {
      fight->set("DragonUUID", Long(UuidRegistrar::ToId(*uuid)));
      fight->set("DragonSpawned", Bool(true));
    } else {
      fight->set("DragonUUID", Long(-1));
      fight->set("DragonSpawned", Bool(true));
      fight->set("IsRespawning", Bool(false));
    }

    auto gateways = dragonFight->listTag("Gateways");
    if (gateways) {
      auto v = List<Tag::Type::Int>();
      for (auto const &it : *gateways) {
        auto p = it->asInt();
        if (!p) {
          v = nullptr;
          break;
        }
        v->push_back(Int(p->fValue));
      }
      if (v) {
        fight->set("Gateways", v);
      }
    }

    auto exitPortalLocation = dragonFight->compoundTag("ExitPortalLocation");
    std::optional<Pos3i> exitLocation;
    if (exitPortalLocation) {
      auto x = exitPortalLocation->int32("X");
      auto y = exitPortalLocation->int32("Y");
      auto z = exitPortalLocation->int32("Z");
      if (x && y && z) {
        exitLocation = Pos3i(*x, *y, *z);
      }
    }
    if (!exitLocation) {
      auto pos = ExitPortalAltitude(endPortalsInEndDimension);
      if (pos) {
        exitLocation = *pos;
      }
    }
    if (exitLocation) {
      auto v = List<Tag::Type::Int>();
      v->push_back(Int(exitLocation->fX));
      v->push_back(Int(exitLocation->fY));
      v->push_back(Int(exitLocation->fZ));
      fight->set("ExitPortalLocation", v);
    }

    auto root = Compound();
    auto d = Compound();
    d->set("DragonFight", fight);
    root->set("data", d);

    return CompoundTag::Write(*root, mcfile::Endian::Little);
  }

  static std::optional<std::string> MobEvents(CompoundTag const &tag) {
    using namespace std;

    auto gameRulesTag = tag.query("Data/GameRules");
    if (!gameRulesTag) {
      return nullopt;
    }
    auto gameRules = gameRulesTag->asCompound();
    if (!gameRules) {
      return nullopt;
    }
    auto ret = Compound();
    ret->set("events_enabled", Bool(true));
    ret->set("minecraft:ender_dragon_event", Bool(true));
    ret->set("minecraft:pillager_patrols_event", Bool(gameRules->string("doPatrolSpawning", "true") == "true"));
    ret->set("minecraft:wandering_trader_event", Bool(gameRules->string("doTraderSpawning", "true") == "true"));

    return CompoundTag::Write(*ret, mcfile::Endian::Little);
  }

  static CompoundTagPtr Read(std::filesystem::path const &javaEditionLevelDat) {
    using namespace std;
    namespace fs = std::filesystem;
    using namespace mcfile::stream;

    error_code ec;
    if (!fs::is_regular_file(javaEditionLevelDat, ec)) {
      return nullptr;
    }
    if (ec) {
      return nullptr;
    }

    auto stream = make_shared<GzFileInputStream>(javaEditionLevelDat);
    return CompoundTag::Read(stream, mcfile::Endian::Big);
  }

private:
  static std::optional<Pos3i> ExitPortalAltitude(std::unordered_set<Pos3i, Pos3iHasher> const &blocks) {
    std::vector<Pos3i> candidates;
    for (auto const &pos : blocks) {
      if (IsValidExitPortal(pos, blocks)) {
        candidates.push_back(Pos3i(pos.fX + 1, pos.fY, pos.fZ + 2));
      }
    }
    if (candidates.empty()) {
      return std::nullopt;
    }
    if (candidates.size() > 1) {
      Pos3i origin(0, 0, 0);
      std::sort(candidates.begin(), candidates.end(), [origin](Pos3i const &a, Pos3i const &b) {
        auto distanceA = Pos3i::DistanceSquare(a, origin);
        auto distanceB = Pos3i::DistanceSquare(b, origin);
        return distanceA < distanceB;
      });
    }
    return candidates[0];
  }

  static bool IsValidExitPortal(Pos3i p, std::unordered_set<Pos3i, Pos3iHasher> const &blocks) {
    static std::vector<Pos3i> const portalPlacement = {
        // line1
        Pos3i(0, 0, 0),
        Pos3i(1, 0, 0),
        Pos3i(2, 0, 0),
        // line2
        Pos3i(-1, 0, 1),
        Pos3i(0, 0, 1),
        Pos3i(1, 0, 1),
        Pos3i(2, 0, 1),
        Pos3i(3, 0, 1),
        // line3
        Pos3i(-1, 0, 2),
        Pos3i(0, 0, 2),
        Pos3i(2, 0, 2),
        Pos3i(3, 0, 2),
        // line4
        Pos3i(-1, 0, 3),
        Pos3i(0, 0, 3),
        Pos3i(1, 0, 3),
        Pos3i(2, 0, 3),
        Pos3i(3, 0, 3),
        // line5
        Pos3i(0, 0, 4),
        Pos3i(1, 0, 4),
        Pos3i(2, 0, 4),
    };
    if (blocks.size() < portalPlacement.size()) {
      return false;
    }
    for (auto const &it : portalPlacement) {
      Pos3i test(p.fX + it.fX, p.fY + it.fY, p.fZ + it.fZ);
      if (blocks.find(test) == blocks.end()) {
        return false;
      }
    }
    return true;
  }
};

} // namespace je2be::tobe
