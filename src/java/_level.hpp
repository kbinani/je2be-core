#pragma once

#include "_props.hpp"
#include "_version.hpp"
#include "enums/_game-mode.hpp"
#include "java/_flat-world-layers.hpp"
#include "java/_player-abilities.hpp"
#include "java/_uuid-registrar.hpp"
#include "java/_versions.hpp"

namespace je2be::java {

class Level {
public:
  PlayerAbilities fAbilities;
  Version fMinimumCompatibleClientVersion = kMinimumCompatibleClientVersion;
  std::u8string fBaseGameVersion = u8"*";
  std::u8string fBiomeOverride = u8"";
  bool fBonusChestEnabled = false;
  bool fBonusChestSpawned = false;
  i8 fCenterMapsToOrigin = 0;
  bool fCommandBlockOutput = true;
  bool fCommandblocksEnabled = true;
  bool fCommandsEnabled = false;
  bool fConfirmedPlatformLockedContent = false;
  i64 fCurrentTick = 0;
  i32 fDifficulty = 2;
  std::u8string fFlatWorldLayers = u8"null\x0a";

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
  bool fDoLimitedCrafting = false;
  i32 fPlayersSleepingPercentage = 100;
  bool fRecipeUnlock = true;
  bool fRespawnBlocksExplode = true;
  bool fTntExplosionDropDecay = false;

  bool fEducationFeaturesEnabled = false;
  i32 fEduOffer = 0;
  std::unordered_map<std::u8string, bool> fExperiments;
  bool fExperimentsEverUsed = false;
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
  std::u8string fInventoryVersion = kInventoryVersion;
  bool fLANBroadcast = true;
  bool fLANBroadcastIntent = true;
  i64 fLastPlayed = 0;
  std::u8string fLevelName = u8"";
  i32 fLimitedWorldOriginX = 0;
  i32 fLimitedWorldOriginY = 32767;
  i32 fLimitedWorldOriginZ = 0;
  bool fMultiplayerGame = true;
  bool fMultiplayerGameIntent = true;
  bool fNaturalRegeneration = true;
  i32 fNetherScale = 8;
  i32 fNetworkVersion = kNetworkVersion;
  i32 fPlatform = 2;
  i32 fPlatformBroadcastIntent = 2;
  std::u8string fPrid = u8"";
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
  i32 fXBLBroadcastIntent = 2;
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
  i32 fWorldVersion = 1;
  bool fCheatsEnabled = false;
  i32 fEditorWorldType = 0;
  bool fIsCreatedInEditor = false;
  bool fIsExportedFromEditor = false;
  bool fIsRandomSeedAllowed = false;
  int fPermissionsLevel = 0;
  int fPlayerPermissionsLevel = 1;
  bool fShowborderEffect = true;
  i32 fDataVersion = mcfile::je::Chunk::kDataVersion;
  bool fProjectilescanbreakblocks = true;
  bool fShowrecipemessages = true;
  bool fIsHardcore = false;
  bool fPlayerHasDied = false;
  bool fLocatorBar = true;

  CompoundTagPtr toBedrockCompoundTag() const {
    auto root = Compound();
    root->insert({
        {u8"abilities", fAbilities.toCompoundTag()},
        {u8"MinimumCompatibleClientVersion", fMinimumCompatibleClientVersion.toListTag()},
        {u8"baseGameVersion", String(fBaseGameVersion)},
        {u8"BiomeOverride", String(fBiomeOverride)},
        {u8"bonusChestEnabled", Bool(fBonusChestEnabled)},
        {u8"bonusChestSpawned", Bool(fBonusChestSpawned)},
        {u8"CenterMapsToOrigin", Byte(fCenterMapsToOrigin)},
        {u8"commandblockoutput", Bool(fCommandBlockOutput)},
        {u8"commandblocksenabled", Bool(fCommandblocksEnabled)},
        {u8"commandsEnabled", Bool(fCommandsEnabled)},
        {u8"ConfirmedPlatformLockedContent", Bool(fConfirmedPlatformLockedContent)},
        {u8"currentTick", Long(fCurrentTick)},
        {u8"Difficulty", Int(fDifficulty)},
        {u8"FlatWorldLayers", String(fFlatWorldLayers)},
        {u8"dodaylightcycle", Bool(fDoDaylightCycle)},
        {u8"doentitydrops", Bool(fDoEntityDrops)},
        {u8"dofiretick", Bool(fDoFireTick)},
        {u8"doimmediaterespawn", Bool(fDoImmediateRespawn)},
        {u8"doinsomnia", Bool(fDoInsomnia)},
        {u8"domobloot", Bool(fDoMobLoot)},
        {u8"domobspawning", Bool(fDoMobSpawning)},
        {u8"dotiledrops", Bool(fDoTileDrops)},
        {u8"doweathercycle", Bool(fDoWeatherCycle)},
        {u8"drowningdamage", Bool(fDrowningDamage)},
        {u8"educationFeaturesEnabled", Bool(fEducationFeaturesEnabled)},
        {u8"eduOffer", Int(fEduOffer)},
        {u8"falldamage", Bool(fFallDamage)},
        {u8"firedamage", Bool(fFireDamage)},
        {u8"freezedamage", Bool(fFreezeDamage)},
        {u8"ForceGameType", Bool(fForceGameType)},
        {u8"functioncommandlimit", Int(fFunctionCommandLimit)},
        {u8"GameType", Int(BedrockFromGameMode(fGameType))},
        {u8"Generator", Int(fGenerator)},
        {u8"hasBeenLoadedInCreative", Bool(fHasBeenLoadedInCreative)},
        {u8"hasLockedBehaviorPack", Bool(fHasLockedBehaviorPack)},
        {u8"hasLockedResourcePack", Bool(fHasLockedResourcePack)},
        {u8"immutableWorld", Bool(fImmutableWorld)},
        {u8"InventoryVersion", String(fInventoryVersion)},
        {u8"LANBroadcast", Bool(fLANBroadcast)},
        {u8"LANBroadcastIntent", Bool(fLANBroadcastIntent)},
        {u8"LastPlayed", Long(fLastPlayed)},
        {u8"LevelName", String(fLevelName)},
        {u8"LimitedWorldOriginX", Int(fLimitedWorldOriginX)},
        {u8"LimitedWorldOriginY", Int(fLimitedWorldOriginY)},
        {u8"LimitedWorldOriginZ", Int(fLimitedWorldOriginZ)},
        {u8"MultiplayerGame", Bool(fMultiplayerGame)},
        {u8"MultiplayerGameIntent", Bool(fMultiplayerGameIntent)},
        {u8"naturalregeneration", Bool(fNaturalRegeneration)},
        {u8"NetherScale", Int(fNetherScale)},
        {u8"NetworkVersion", Int(fNetworkVersion)},
        {u8"Platform", Int(fPlatform)},
        {u8"PlatformBroadcastIntent", Int(fPlatformBroadcastIntent)},
        {u8"prid", String(fPrid)},
        {u8"pvp", Bool(fPvp)},
        {u8"rainLevel", Float(fRainLevel)},
        {u8"rainTime", Int(fRainTime)},
        {u8"RandomSeed", Long(fRandomSeed)},
        {u8"randomtickspeed", Int(fRandomTickSpeed)},
        {u8"requiresCopiedPackRemovalCheck", Bool(fRequiresCopiedPackRemovalCheck)},
        {u8"sendcommandfeedback", Bool(fSendCommandFeedback)},
        {u8"serverChunkTickRange", Int(fServerChunkTickRange)},
        {u8"showcoordinates", Bool(fShowCoordinates)},
        {u8"showdeathmessages", Bool(fShowDeathMessages)},
        {u8"showtags", Bool(fShowTags)},
        {u8"spawnMobs", Bool(fSpawnMobs)},
        {u8"spawnradius", Int(fSpawnRadius)},
        {u8"SpawnV1Villagers", Bool(fSpawnV1Villagers)},
        {u8"SpawnX", Int(fSpawnX)},
        {u8"SpawnY", Int(fSpawnY)},
        {u8"SpawnZ", Int(fSpawnZ)},
        {u8"startWithMapEnabled", Bool(fStartWithMapEnabled)},
        {u8"StorageVersion", Int(fStorageVersion)},
        {u8"texturePacksRequired", Bool(fTexturePacksRequired)},
        {u8"Time", Long(fTime)},
        {u8"tntexplodes", Bool(fTntExplodes)},
        {u8"useMsaGamertagsOnly", Bool(fUseMsaGamertagsOnly)},
        {u8"worldStartCount", Long(fWorldStartCount)},
        {u8"XBLBroadcastIntent", Int(fXBLBroadcastIntent)},
        {u8"isFromLockedTemplate", Bool(fIsFromLockedTemplate)},
        {u8"isFromWorldTemplate", Bool(fIsFromWorldTemplate)},
        {u8"isSingleUseWorld", Bool(fIsSingleUseWorld)},
        {u8"isWorldTemplateOptionLocked", Bool(fIsWorldTemplateOptionLocked)},
        {u8"keepinventory", Bool(fKeepInventory)},
        {u8"mobgriefing", Bool(fMobGriefing)},
        {u8"lastOpenedWithVersion", fLastOpenedWithVersion.toListTag()},
        {u8"lightningLevel", Float(fLightningLevel)},
        {u8"lightningTime", Int(fLightningTime)},
        {u8"limitedWorldDepth", Int(fLimitedWorldDepth)},
        {u8"limitedWorldWidth", Int(fLimitedWorldWidth)},
        {u8"locatorbar", Bool(fLocatorBar)},
        {u8"maxcommandchainlength", Int(fMaxCommandChainLength)},
        {u8"WorldVersion", Int(fWorldVersion)},
        {u8"cheatsEnabled", Bool(fCheatsEnabled)},
        {u8"daylightCycle", Int(fDoDaylightCycle ? 0 : 1)},
        {u8"dolimitedcrafting", Bool(fDoLimitedCrafting)},
        {u8"editorWorldType", Int(fEditorWorldType)},
        {u8"isCreatedInEditor", Bool(fIsCreatedInEditor)},
        {u8"isExportedFromEditor", Bool(fIsExportedFromEditor)},
        {u8"isRandomSeedAllowed", Bool(fIsRandomSeedAllowed)},
        {u8"permissionsLevel", Int(fPermissionsLevel)},
        {u8"playerPermissionsLevel", Int(fPlayerPermissionsLevel)},
        {u8"playerssleepingpercentage", Int(fPlayersSleepingPercentage)},
        {u8"recipesunlock", Bool(fRecipeUnlock)},
        {u8"respawnblocksexplode", Bool(fRespawnBlocksExplode)},
        {u8"showbordereffect", Bool(fShowborderEffect)},
        {u8"world_policies", Compound()},
        {u8"projectilescanbreakblocks", Bool(fProjectilescanbreakblocks)},
        {u8"showrecipemessages", Bool(fShowrecipemessages)},
        {u8"IsHardcore", Bool(fIsHardcore)},
        {u8"tntexplosiondropdecay", Bool(fTntExplosionDropDecay)},
        {u8"PlayerHasDied", Bool(fPlayerHasDied)},

        {u8"HasUncompleteWorldFileOnDisk", Bool(false)},
    });
    auto experiments = Compound();
    experiments->set(u8"experiments_ever_used", Bool(fExperimentsEverUsed || !fExperiments.empty()));
    experiments->set(u8"saved_with_toggled_experiments", Bool(fExperimentsEverUsed || !fExperiments.empty()));
    if (!fExperiments.empty()) {
      for (auto const &it : fExperiments) {
        experiments->set(it.first, Bool(it.second));
      }
    }
    root->set(u8"experiments", experiments);
    return root;
  }

  [[nodiscard]] bool write(std::filesystem::path path) const {
    auto stream = std::make_shared<mcfile::stream::FileOutputStream>(path);
    mcfile::stream::OutputStreamWriter w(stream, mcfile::Encoding::LittleEndian);
    if (!w.write((u32)8)) {
      return false;
    }
    if (!w.write((u32)0)) {
      return false;
    }
    auto tag = this->toBedrockCompoundTag();
    if (!CompoundTag::Write(*tag, w)) {
      return false;
    }
    u64 pos = stream->pos();
    if (!stream->seek(4)) {
      return false;
    }
    return w.write((u32)pos - 8);
  }

  static Level ImportFromJava(CompoundTag const &tag) {
    Level ret;
    auto data = tag.compoundTag(u8"Data");
    if (!data) {
      return ret;
    }

#define I(__name, __type, __key) ret.__name = data->__type(__key, ret.__name)
    I(fCommandsEnabled, boolean, u8"allowCommands");
    I(fTime, int64, u8"DayTime");
    I(fDifficulty, byte, u8"Difficulty");
    I(fLevelName, string, u8"LevelName");
    I(fSpawnX, int32, u8"SpawnX");
    I(fSpawnY, int32, u8"SpawnY");
    I(fSpawnZ, int32, u8"SpawnZ");
    I(fCurrentTick, int64, u8"Time");
    I(fLimitedWorldOriginX, float64, u8"BorderCenterX");
    I(fLimitedWorldOriginZ, float64, u8"BorderCenterZ");
    I(fIsHardcore, boolean, u8"hardcore");
    ret.fLastPlayed = data->int64(u8"LastPlayed", ret.fLastPlayed * 1000) / 1000;
#undef I

    auto gameRules = data->compoundTag(u8"GameRules");
    if (gameRules) {
#define S(__name, __var)                            \
  auto __name = gameRules->stringTag(u8"" #__name); \
  if (__name) {                                     \
    __var = __name->fValue == u8"true";             \
  }
#define I(__name, __var)                            \
  auto __name = gameRules->stringTag(u8"" #__name); \
  if (__name) {                                     \
    auto v = strings::ToI32(__name->fValue);        \
    if (v) {                                        \
      __var = *v;                                   \
    }                                               \
  }
      // allowFireTicksAwayFromPlayer
      // announceAdvancements
      S(commandBlockOutput, ret.fCommandBlockOutput);
      // disableElytraMovementCheck
      // disableRaids
      S(doDaylightCycle, ret.fDoDaylightCycle);
      S(doEntityDrops, ret.fDoEntityDrops);
      S(doFireTick, ret.fDoFireTick);
      S(doImmediateRespawn, ret.fDoImmediateRespawn);
      S(doInsomnia, ret.fDoInsomnia);
      S(doLimitedCrafting, ret.fDoLimitedCrafting);
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
      S(locatorBar, ret.fLocatorBar);
      // logAdminCommands
      I(maxCommandChainLength, ret.fMaxCommandChainLength);
      // maxEntityCramming
      S(mobGriefing, ret.fMobGriefing);
      S(naturalRegeneration, ret.fNaturalRegeneration);
      I(randomTickSpeed, ret.fRandomTickSpeed);
      S(sendCommandFeedback, ret.fSendCommandFeedback);
      S(showDeathMessages, ret.fShowDeathMessages);
      I(spawnRadius, ret.fSpawnRadius);
      // spectatorsGenerateChunks
      // universalAnger
      I(playersSleepingPercentage, ret.fPlayersSleepingPercentage);
      // NOTE: showcoordinates and reducedDebugInfo are not identical, but converting here for player convenience
      ret.fShowCoordinates = gameRules->string(u8"reducedDebugInfo", u8"false") != u8"true";
      S(projectilesCanBreakBlocks, ret.fProjectilescanbreakblocks);
      S(tntExplodes, ret.fTntExplodes);
      S(tntexplosiondropdecay, ret.fTntExplosionDropDecay);
#undef S
#undef I
    }

    auto worldGenSettings = data->compoundTag(u8"WorldGenSettings");
    if (worldGenSettings) {
      ret.fRandomSeed = worldGenSettings->int64(u8"seed", ret.fRandomSeed);
    }

    auto flatWorldLayers = FlatWorldLayers::FromLevelData(*data);
    if (flatWorldLayers) {
      ret.fFlatWorldLayers = *flatWorldLayers;
      ret.fGenerator = 2;
    }

    ret.fRainLevel = data->boolean(u8"raining", false) ? 1 : 0;
    ret.fRainTime = data->int32(u8"rainTime", ret.fRainTime);
    ret.fLightningLevel = data->boolean(u8"thundering", false) ? 1 : 0;
    ret.fLightningTime = data->int32(u8"thunderTime", ret.fLightningTime);
    if (auto type = GameModeFromJava(data->int32(u8"GameType", 0)); type) {
      ret.fGameType = *type;
    }
    if (auto player = data->compoundTag(u8"Player"); player) {
      if (auto lastDeathLocation = player->compoundTag(u8"LastDeathLocation"); lastDeathLocation) {
        if (lastDeathLocation->string(u8"dimension") && lastDeathLocation->intArrayTag(u8"pos")) {
          ret.fPlayerHasDied = true;
        }
      }
    }
    ret.fDataVersion = data->int32(u8"DataVersion", mcfile::je::Chunk::kDataVersion);

    if (auto enabledFeatures = data->listTag(u8"enabled_features"); enabledFeatures) {
      for (auto it : *enabledFeatures) {
        auto s = it->asString();
        if (!s) {
          continue;
        }
        if (s->fValue == u8"minecraft:vanilla") {
          continue;
        }
        ret.fExperimentsEverUsed = true;
        if (s->fValue == u8"minecraft:trade_rebalance") {
          ret.fExperiments[u8"villager_trades_rebalance"] = true;
        }
      }
    }

    return ret;
  }

  static std::optional<std::string> TheEndData(CompoundTag const &tag, size_t numAutonomousEntities, std::unordered_set<Pos3i, Pos3iHasher> const &endPortalsInEndDimension, std::shared_ptr<UuidRegistrar> const &uuids) {
    auto data = tag.query(u8"Data")->asCompound();
    if (!data) {
      return std::nullopt;
    }

    CompoundTag const *dragonFight = data->query(u8"DragonFight")->asCompound();
    if (!dragonFight) {
      dragonFight = data->query(u8"DimensionData/1/DragonFight")->asCompound();
    }
    if (!dragonFight) {
      return std::nullopt;
    }

    auto fight = Compound();

    fight->set(u8"DragonFightVersion", Byte(kDragonFightVersion));

    auto killed = dragonFight->boolean(u8"DragonKilled", false);
    fight->set(u8"DragonKilled", Bool(numAutonomousEntities == 0 && killed));

    auto previouslyKilled = dragonFight->byteTag(u8"PreviouslyKilled");
    if (previouslyKilled) {
      fight->set(u8"PreviouslyKilled", Bool(previouslyKilled->fValue != 0));
    }

    auto uuid = props::GetUuid(*dragonFight, {.fIntArray = u8"Dragon"});
    if (uuid) {
      fight->set(u8"DragonUUID", Long(uuids->toId(*uuid)));
      fight->set(u8"DragonSpawned", Bool(true));
    } else {
      fight->set(u8"DragonUUID", Long(-1));
      fight->set(u8"DragonSpawned", Bool(true));
      fight->set(u8"IsRespawning", Bool(false));
    }

    if (auto gateways = dragonFight->intArrayTag(u8"Gateways"); gateways) {
      auto v = List<Tag::Type::Int>();
      for (i32 p : gateways->fValue) {
        v->push_back(Int(p));
      }
      fight->set(u8"Gateways", v);
    } else if (auto gatewaysList = dragonFight->listTag(u8"Gateways"); gatewaysList) {
      auto v = List<Tag::Type::Int>();
      for (auto const &it : *gatewaysList) {
        auto p = it->asInt();
        if (!p) {
          v = nullptr;
          break;
        }
        v->push_back(Int(p->fValue));
      }
      if (v) {
        fight->set(u8"Gateways", v);
      }
    }

    auto exitPortalLocation = dragonFight->compoundTag(u8"ExitPortalLocation");
    std::optional<Pos3i> exitLocation;
    if (exitPortalLocation) {
      auto x = exitPortalLocation->int32(u8"X");
      auto y = exitPortalLocation->int32(u8"Y");
      auto z = exitPortalLocation->int32(u8"Z");
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
      fight->set(u8"ExitPortalLocation", v);
    }

    auto root = Compound();
    auto d = Compound();
    d->set(u8"DragonFight", fight);
    root->set(u8"data", d);

    return CompoundTag::Write(*root, mcfile::Encoding::LittleEndian);
  }

  static std::optional<std::string> MobEvents(CompoundTag const &tag) {
    using namespace std;

    auto gameRulesTag = tag.query(u8"Data/GameRules");
    if (!gameRulesTag) {
      return nullopt;
    }
    auto gameRules = gameRulesTag->asCompound();
    if (!gameRules) {
      return nullopt;
    }
    auto ret = Compound();
    ret->set(u8"events_enabled", Bool(true));
    ret->set(u8"minecraft:ender_dragon_event", Bool(true));
    ret->set(u8"minecraft:pillager_patrols_event", Bool(gameRules->string(u8"doPatrolSpawning", u8"true") == u8"true"));
    ret->set(u8"minecraft:wandering_trader_event", Bool(gameRules->string(u8"doTraderSpawning", u8"true") == u8"true"));

    return CompoundTag::Write(*ret, mcfile::Encoding::LittleEndian);
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
    return CompoundTag::Read(stream, mcfile::Encoding::Java);
  }

private:
  static std::optional<Pos3i> ExitPortalAltitude(std::unordered_set<Pos3i, Pos3iHasher> const &blocks) {
    std::vector<Pos3i> candidates;
    for (auto const &pos : blocks) {
      if (IsValidExitPortal(pos, blocks)) {
        candidates.emplace_back(pos.fX + 1, pos.fY, pos.fZ + 2);
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
    static std::array<Pos3i, 20> const portalPlacement = {
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

} // namespace je2be::java
