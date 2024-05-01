#include "bedrock/_block-entity.hpp"

#include "_namespace.hpp"
#include "_optional.hpp"
#include "bedrock/_context.hpp"
#include "bedrock/_entity.hpp"
#include "bedrock/_item.hpp"
#include "color/_sign-color.hpp"
#include "command/_command.hpp"
#include "enums/_banner-color-code-bedrock.hpp"
#include "enums/_color-code-java.hpp"
#include "enums/_facing6.hpp"
#include "enums/_red-flower.hpp"
#include "enums/_skull-type.hpp"
#include "item/_banner.hpp"
#include "tile-entity/_beacon.hpp"
#include "tile-entity/_loot-table.hpp"

#include <minecraft-file.hpp>

namespace je2be::bedrock {

class BlockEntity::Impl {
private:
  Impl() = delete;

public:
  static std::optional<Result> FromBlockAndBlockEntity(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;
    static unique_ptr<unordered_map<u8string_view, Converter> const> const sTable(CreateTable());
    u8string_view key(block.fName);
    auto found = sTable->find(Namespace::Remove(key));
    if (found == sTable->end()) {
      return nullopt;
    }
    auto result = found->second(pos, block, tag, blockJ, ctx, dataVersion);
    if (result && result->fTileEntity) {
      if (!result->fTileEntity->string(u8"CustomName")) {
        auto customName = tag.string(u8"CustomName");
        if (customName) {
          props::Json json;
          props::SetJsonString(json, u8"text", *customName);
          result->fTileEntity->set(u8"CustomName", props::StringFromJson(json));
        }
      }
    }
    return result;
  }

#pragma region Dedicated converters
  static std::optional<Result> Banner(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;
    auto base = tag.int32(u8"Base", 0);
    BannerColorCodeBedrock bccb = static_cast<BannerColorCodeBedrock>(base);
    auto color = ColorCodeJavaFromBannerColorCodeBedrock(bccb);
    mcfile::blocks::BlockId id;
    if (block.fName == u8"minecraft:standing_banner") {
      id = BannerBlockIdFromColorCodeJava(color);
    } else {
      id = WallBannerBlockIdFromColorCodeJava(color);
    }
    auto te = EmptyShortName(u8"banner", pos);
    auto type = tag.int32(u8"Type", 0);
    ListTagPtr patternsJ;
    if (type == 1) {
      // Illager Banner
      te->set(u8"CustomName", u8R"({"color":"gold","translate":"block.minecraft.ominous_banner"})");
      patternsJ = Banner::OminousBannerPatterns();
    } else {
      auto patternsB = tag.listTag(u8"Patterns");
      patternsJ = List<Tag::Type::Compound>();
      if (patternsB) {
        for (auto const &pB : *patternsB) {
          CompoundTag const *c = pB->asCompound();
          if (!c) {
            continue;
          }
          auto pColorB = c->int32(u8"Color");
          auto pPatternB = c->string(u8"Pattern");
          if (!pColorB || !pPatternB) {
            continue;
          }
          auto pJ = Compound();
          pJ->set(u8"Color", Int(static_cast<i32>(ColorCodeJavaFromBannerColorCodeBedrock(static_cast<BannerColorCodeBedrock>(*pColorB)))));
          pJ->set(u8"Pattern", *pPatternB);
          patternsJ->push_back(pJ);
        }
      }
    }
    if (patternsJ) {
      AppendComponent(te, u8"banner_patterns", patternsJ);
    }
    Result r;
    r.fBlock = blockJ.withId(id);
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Beacon(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    auto t = EmptyShortName(u8"beacon", pos);
    if (auto primaryB = tagB.int32(u8"primary"); primaryB) {
      if (auto primaryJ = Beacon::JavaEffectFromLegacyJavaAndBedrock(*primaryB); primaryJ) {
        t->set(u8"primary_effect", *primaryJ);
      }
    }
    if (auto secondaryB = tagB.int32(u8"secondary"); secondaryB) {
      if (auto secondaryJ = Beacon::JavaEffectFromLegacyJavaAndBedrock(*secondaryB); secondaryJ) {
        t->set(u8"secondary_effect", *secondaryJ);
      }
    }
    // NOTE: "Levels" need to be defined by terrain around the beacon.
    // See also beacon.hpp
    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> Bed(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    auto color = tagB.byte(u8"color", 0);
    ColorCodeJava ccj = static_cast<ColorCodeJava>(color);
    Result r;
    r.fBlock = blockJ.withId(BedBlockIdFromColorCodeJava(ccj));
    r.fTileEntity = EmptyShortName(u8"bed", pos);
    return r;
  }

  static std::optional<Result> Beehive(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    auto te = EmptyShortName(u8"beehive", pos);
    auto occupants = tagB.listTag(u8"Occupants");
    if (occupants) {
      auto bees = List<Tag::Type::Compound>();
      for (auto const &it : *occupants) {
        auto occupant = it->asCompound();
        if (!occupant) {
          continue;
        }
        CompoundTag const *beeB = occupant; // legacy(?)

        if (auto saveData = occupant->compoundTag(u8"SaveData"); saveData) {
          // This format was seen at least v1.19.50
          beeB = saveData.get();
        }

        auto result = Entity::From(*beeB, ctx, dataVersion);
        if (result && result->fEntity) {
          auto data = Compound();
          for (auto const &key : {u8"Air", u8"ArmorDropChances", u8"ArmorItems", u8"Brain", u8"UUID",
                                  u8"Pos", u8"CanPickUpLoot", u8"CannotEnterHiveTicks", u8"CropsGrownSincePollination", u8"DeathTime",
                                  u8"FallDistance", u8"HandItems", u8"FallFlying", u8"HurtTime", u8"NoGravity",
                                  u8"OnGround", u8"PortalCooldown", u8"Rotation"}) {
            result->fEntity->erase(key);
          }
          data->set(u8"EntityData", result->fEntity);
          bees->push_back(data);
        }
      }
      te->set(u8"Bees", bees);
    }
    Result r;
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> BrewingStand(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;
    auto t = EmptyShortName(u8"brewing_stand", pos);
    auto itemsB = ContainerItems(tagB, u8"Items", ctx, dataVersion);
    if (itemsB) {
      u8 const mapping[5] = {3, 0, 1, 2, 4};
      map<int, shared_ptr<CompoundTag>> items;
      for (int i = 0; i < 5 && i < itemsB->size(); i++) {
        auto itemTag = itemsB->at(i);
        CompoundTag const *item = itemTag->asCompound();
        if (!item) {
          continue;
        }
        auto slot = item->byte(u8"Slot");
        if (!slot) {
          continue;
        }
        if (5 <= *slot) {
          continue;
        }
        auto newSlot = mapping[*slot];
        shared_ptr<CompoundTag> copy = item->copy();
        copy->set(u8"Slot", Byte(newSlot));
        items[newSlot] = copy;
      }
      auto itemsJ = List<Tag::Type::Compound>();
      for (auto it : items) {
        itemsJ->push_back(it.second);
      }
      t->set(u8"Items", itemsJ);
    }
    CopyShortValues(tagB, *t, {{u8"CookTime", u8"BrewTime", 0}});
    auto fuelAmount = tagB.int16(u8"FuelAmount", 0);
    t->set(u8"Fuel", Byte(fuelAmount));
    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> Campfire(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;
    auto items = List<Tag::Type::Compound>();
    vector<int> times;
    vector<int> totalTimes;
    for (int i = 0; i < 4; i++) {
      auto itemTag = tagB.compoundTag(u8"Item" + mcfile::String::ToString(i + 1));
      bool itemAdded = false;
      if (itemTag) {
        auto item = Item::From(*itemTag, ctx, dataVersion, {});
        if (item) {
          item->set(u8"Slot", Byte(i));
          items->push_back(item);
          itemAdded = true;
        }
      }
      auto time = tagB.int32(u8"ItemTime" + mcfile::String::ToString(i + 1), 0);
      times.push_back(time);
      totalTimes.push_back((itemAdded || time > 0) ? 600 : 0);
    }
    auto timesTag = make_shared<IntArrayTag>(times);
    auto totalTimesTag = make_shared<IntArrayTag>(totalTimes);
    auto t = EmptyShortName(u8"campfire", pos);
    t->set(u8"Items", items);
    t->set(u8"CookingTimes", timesTag);
    t->set(u8"CookingTotalTimes", totalTimesTag);

    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> Chest(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;
    auto px = tagB.int32(u8"pairx");
    auto pz = tagB.int32(u8"pairz");
    bool pairlead = tagB.boolean(u8"pairlead", false);
    u8string type = u8"single";

    Result r;

    if (px && pz) {
      Facing6 f6 = Facing6FromBedrockCardinalDirectionMigratingFacingDirectionA(block);
      Pos3i vec = Pos3iFromFacing6(f6);
      Pos2i d2(vec.fX, vec.fZ);
      Pos2i pos2d(pos.fX, pos.fZ);
      Pos2i pair(*px, *pz);

      if (pair + Right90(d2) == pos2d) {
        type = u8"right";
      } else if (pair + Left90(d2) == pos2d) {
        type = u8"left";
      }
      if ((type == u8"right" && !pairlead) || (type == u8"left" && pairlead)) {
        // pairlead = true side of chest occupies upper half of the large chest in BE.
        // On the other hand, in JE, type = right side of chest always occupies upper half of the large chest.
        // Therefore, it is needed to swap "Items" between two chests in this situation.
        r.fTakeItemsFrom = Pos3i(*px, pos.fY, *pz);
      }
    }
    r.fBlock = blockJ.applying({{u8"type", type}});
    auto te = EmptyFullName(block.fName, pos);
    if (auto st = LootTable::BedrockToJava(tagB, *te); st == LootTable::State::NoLootTable && !r.fTakeItemsFrom) {
      auto items = ContainerItems(tagB, u8"Items", ctx, dataVersion);
      if (items) {
        te->set(u8"Items", items);
      }
    }
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> ChiseledBookshelf(Pos3i const &pos, mcfile::be::Block const &blockB, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;

    auto t = EmptyShortName(u8"chiseled_bookshelf", pos);

    auto itemsJ = ContainerItemsWithoutSlot(tagB, u8"Items", ctx, true, dataVersion);
    t->set(u8"Items", itemsJ);

    CopyIntValues(tagB, *t, {{u8"LastInteractedSlot", u8"last_interacted_slot"}});

    map<u8string, optional<u8string>> props;
    if (auto items = tagB.listTag(u8"Items"); items) {
      for (int i = 0; i < 6; i++) {
        bool occupied = false;
        if (i < items->size()) {
          if (auto item = items->at(i); item && item->asCompound()) {
            auto count = item->asCompound()->byte(u8"Count", 0);
            occupied = count > 0;
          }
        }
        u8string name = u8"slot_" + mcfile::String::ToString(i) + u8"_occupied";

        props[name] = occupied ? u8"true" : u8"false";
      }
    }

    Result r;
    r.fTileEntity = t;
    r.fBlock = blockJ.applying(props);
    return r;
  }

  static std::optional<Result> CommandBlock(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    auto t = EmptyShortName(u8"command_block", pos);

    if (auto commandB = tagB.string(u8"Command"); commandB) {
      auto commandJ = je2be::command::Command::TranspileBedrockToJava(*commandB);
      t->set(u8"Command", commandJ);
    }
    CopyIntValues(tagB, *t, {{u8"SuccessCount"}});
    CopyBoolValues(tagB, *t, {{u8"auto"}, {u8"powered"}, {u8"conditionMet"}, {u8"TrackOutput"}});
    CopyLongValues(tagB, *t, {{u8"LastExecution"}});

    t->set(u8"UpdateLastExecution", Bool(true));

    auto customName = tagB.string(u8"CustomName", u8"");
    if (customName.empty()) {
      customName = u8"\"@\"";
    }
    props::Json json;
    props::SetJsonString(json, u8"text", customName);
    t->set(u8"CustomName", props::StringFromJson(json));

    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> Comparator(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    auto t = EmptyShortName(u8"comparator", pos);
    CopyIntValues(tagB, *t, {{u8"OutputSignal", u8"OutputSignal", 0}});
    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> Crafter(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    auto t = EmptyShortName(u8"crafter", pos);
    Result r;
    auto disabledSlotsB = tagB.int16(u8"disabled_slots", 0);
    uint32_t bits = *(u16 *)&disabledSlotsB;
    auto disabledSlotsJ = std::make_shared<IntArrayTag>();
    for (int i = 0; i < 9; i++) {
      u32 test = u32(1) << i;
      if ((bits & test) == test) {
        disabledSlotsJ->fValue.push_back(i);
      }
    }
    t->set(u8"disabled_slots", disabledSlotsJ);
    t->set(u8"triggered", Int(0));
    t->set(u8"crafting_ticks_remaining", Int(0));
    if (auto itemsJ = ContainerItems(tagB, u8"Items", ctx, dataVersion); itemsJ) {
      t->set(u8"Items", itemsJ);
    }
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> DecoratedPot(Pos3i const &pos, mcfile::be::Block const &blockB, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    auto t = EmptyShortName(u8"decorated_pot", pos);
    auto sherdsJ = List<Tag::Type::String>();
    bool some = false;
    for (int i = 0; i < 4; i++) {
      sherdsJ->push_back(String(u8"minecraft:brick"));
    }
    auto sherdsB = tagB.listTag(u8"sherds");
    if (!sherdsB) {
      // < 1.20
      sherdsB = tagB.listTag(u8"shards");
    }
    if (sherdsB) {
      for (int i = 0; i < 4 && i < sherdsB->size(); i++) {
        auto const &sherdB = sherdsB->at(i);
        if (auto sherdName = sherdB->asString(); sherdName && !sherdName->fValue.empty()) {
          some = true;
          sherdsJ->fValue[i] = String(sherdName->fValue);
        }
      }
    }
    if (some) {
      t->set(u8"sherds", sherdsJ);
    }
    if (auto itemB = tagB.compoundTag(u8"item"); itemB) {
      if (auto itemJ = Item::From(*itemB, ctx, dataVersion, {}); itemJ) {
        t->set(u8"item", itemJ);
      }
    }
    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> EndGateway(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    auto t = EmptyShortName(u8"end_gateway", pos);
    if (auto exitPortalB = props::GetPos3iFromListTag(tagB, u8"ExitPortal"); exitPortalB) {
      auto exitPortalJ = Compound();
      exitPortalJ->set(u8"X", Int(exitPortalB->fX));
      exitPortalJ->set(u8"Y", Int(exitPortalB->fY));
      exitPortalJ->set(u8"Z", Int(exitPortalB->fZ));
      t->set(u8"ExitPortal", exitPortalJ);
    }
    if (auto age = tagB.int32(u8"Age"); age) {
      t->set(u8"Age", Long(*age));
    }
    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> FlowerPot(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;
    auto plantBlock = tagB.compoundTag(u8"PlantBlock");
    if (!plantBlock) {
      return nullopt;
    }
    auto s = plantBlock->compoundTag(u8"states");
    if (!s) {
      return nullopt;
    }
    auto fullName = plantBlock->string(u8"name");
    if (!fullName) {
      return nullopt;
    }
    if (!fullName->starts_with(u8"minecraft:")) {
      return nullopt;
    }
    u8string name = fullName->substr(10);
    u8string suffix;
    if (name == u8"sapling") {
      auto type = s->string(u8"sapling_type");
      if (!type) {
        return nullopt;
      }
      suffix = *type + u8"_sapling";
    } else if (name == u8"red_flower") {
      auto type = s->string(u8"flower_type");
      if (!type) {
        return nullopt;
      }
      auto rf = RedFlowerFromBedrockName(*type);
      if (!rf) {
        return nullopt;
      }
      suffix = JavaNameFromRedFlower(*rf);
    } else if (name == u8"yellow_flower") {
      suffix = u8"dandelion";
    } else if (name == u8"deadbush") {
      suffix = u8"dead_bush";
    } else if (name == u8"tallgrass") {
      auto type = s->string(u8"tall_grass_type");
      if (!type) {
        return nullopt;
      }
      suffix = *type;
    } else if (name == u8"azalea") {
      suffix = u8"azalea_bush";
    } else if (name == u8"flowering_azalea") {
      suffix = u8"flowering_azalea_bush";
    } else {
      suffix = name;
    }
    Result r;
    r.fBlock = BlockShortName(u8"potted_" + suffix, dataVersion);
    return r;
  }

  static std::optional<Result> Furnace(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;
    u8string name = strings::RemovePrefix(block.fName.substr(10), u8"lit_");
    auto te = EmptyShortName(name, pos);
    CopyShortValues(tagB, *te, {{u8"BurnDuration", u8"BurnTime"}, {u8"CookTime"}, {u8"BurnTime", u8"CookTimeTotal"}});
    te->set(u8"RecipesUsed", Compound());
    auto items = ContainerItems(tagB, u8"Items", ctx, dataVersion);
    if (items) {
      te->set(u8"Items", items);
    }
    Result r;
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Hopper(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    auto t = EmptyShortName(u8"hopper", pos);
    auto items = ContainerItems(tagB, u8"Items", ctx, dataVersion);
    if (items) {
      t->set(u8"Items", items);
    }
    CopyIntValues(tagB, *t, {{u8"TransferCooldown", u8"TransferCooldown", 0}});
    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> Jigsaw(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    auto t = EmptyShortName(u8"jigsaw", pos);
    CopyStringValues(tag, *t, {{u8"final_state"}, {u8"joint"}, {u8"target"}, {u8"target_pool", u8"pool"}, {u8"name"}});
    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> Jukebox(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;
    auto record = tag.compoundTag(u8"RecordItem");
    auto te = EmptyShortName(u8"jukebox", pos);
    if (record) {
      auto itemJ = Item::From(*record, ctx, dataVersion, {});
      if (itemJ) {
        te->set(u8"RecordItem", itemJ);
      }
    }
    Result r;
    r.fBlock = blockJ.withId(mcfile::blocks::minecraft::jukebox)->applying({{u8"has_record", ToString(record != nullptr)}});
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Lectern(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;
    auto te = EmptyShortName(u8"lectern", pos);
    auto page = tag.int32(u8"page");
    auto bookB = tag.compoundTag(u8"book");
    shared_ptr<CompoundTag> bookJ;
    if (bookB) {
      bookJ = Item::From(*bookB, ctx, dataVersion, {});
    }

    Result r;
    r.fBlock = blockJ.applying({{u8"has_book", ToString(bookJ != nullptr)}});
    if (page) {
      te->set(u8"Page", Int(*page));
    }
    if (bookJ) {
      te->set(u8"Book", bookJ);
    }
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> MobSpawner(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;
    auto t = EmptyShortName(u8"mob_spawner", pos);

    CopyShortValues(tag, *t, {{u8"Delay"}, {u8"MaxNearbyEntities"}, {u8"MaxSpawnDelay"}, {u8"MinSpawnDelay"}, {u8"RequiredPlayerRange"}, {u8"SpawnCount"}, {u8"SpawnRange"}});

    auto entity = tag.string(u8"EntityIdentifier");
    if (entity) {
      auto entityTag = Compound();
      if (!entity->empty()) {
        entityTag->set(u8"id", *entity);
      }
      auto spawnData = Compound();
      spawnData->set(u8"entity", entityTag);
      t->set(u8"SpawnData", spawnData);

      auto potentials = List<Tag::Type::Compound>();
      auto potential = Compound();
      potential->set(u8"data", spawnData->clone());
      potential->set(u8"weight", Int(1));
      potentials->push_back(potential);
      t->set(u8"SpawnPotentials", potentials);
    } else {
      auto spawnData = Compound();
      spawnData->set(u8"entity", Compound());
      t->set(u8"SpawnData", spawnData);
    }

    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> Noteblock(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;
    auto note = tag.byte(u8"note");
    map<u8string, optional<u8string>> p;
    if (note) {
      p[u8"note"] = mcfile::String::ToString(*note);
    }
    Result r;
    r.fBlock = blockJ.applying(p);
    return r;
  }

  static std::optional<Result> SameNameEmpty(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    Result r;
    r.fTileEntity = EmptyFullName(block.fName, pos);
    return r;
  }

  static std::optional<Result> ShulkerBox(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;
    auto facing = tag.byte(u8"facing", 0);
    auto f6 = Facing6FromBedrockFacingDirectionA(facing);
    auto te = EmptyShortName(u8"shulker_box", pos);
    auto items = ContainerItems(tag, u8"Items", ctx, dataVersion);
    if (items) {
      te->set(u8"Items", items);
    }
    Result r;
    r.fBlock = blockJ.applying({{u8"facing", JavaNameFromFacing6(f6)}});
    r.fTileEntity = te;
    return r;
  }

  static CompoundTagPtr SignText(CompoundTag const &input) {
    using namespace std;
    auto ret = Compound();

    CopyBoolValues(input, *ret, {{u8"IgnoreLighting", u8"has_glowing_text", false}});

    auto color = input.int32(u8"SignTextColor");
    if (color) {
      Rgba rgba = Rgba::FromRGB(*color);
      ColorCodeJava jcc = SignColor::MostSimilarColor(rgba);
      ret->set(u8"color", JavaNameFromColorCodeJava(jcc));
    } else {
      ret->set(u8"color", u8"black");
    }

    auto text = input.string(u8"Text", u8"");
    vector<u8string> linesB = mcfile::String::Split(text, u8'\n');
    auto messagesJ = List<Tag::Type::String>();
    for (size_t i = 0; i < 4; i++) {
      u8string line = i < linesB.size() ? linesB[i] : u8"";
      if (line.empty()) {
        messagesJ->push_back(String(u8R"({"text":""})"));
      } else {
        props::Json json;
        props::SetJsonString(json, u8"text", line);
        messagesJ->push_back(String(props::StringFromJson(json)));
      }
    }
    ret->set(u8"messages", messagesJ);

    return ret;
  }

  static CompoundTagPtr SignTextEmpty() {
    auto ret = Compound();
    ret->set(u8"color", u8"black");
    ret->set(u8"has_glowing_text", Bool(false));
    auto messages = List<Tag::Type::String>();
    messages->push_back(String(u8R"({"text":""})"));
    messages->push_back(String(u8R"({"text":""})"));
    messages->push_back(String(u8R"({"text":""})"));
    messages->push_back(String(u8R"({"text":""})"));
    ret->set(u8"messages", messages);
    return ret;
  }

  static Converter Sign(std::u8string id) {
    return [id](Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) -> std::optional<Result> {
      using namespace std;
      auto te = EmptyShortName(id, pos);

      auto frontTextB = tag.compoundTag(u8"FrontText");
      auto backTextB = tag.compoundTag(u8"BackText");
      if (frontTextB || backTextB) {
        if (frontTextB) {
          auto frontTextJ = SignText(*frontTextB);
          te->set(u8"front_text", frontTextJ);
        } else {
          te->set(u8"front_text", SignTextEmpty());
        }
        if (backTextB) {
          auto backTextJ = SignText(*backTextB);
          te->set(u8"back_text", backTextJ);
        } else {
          te->set(u8"back_text", SignTextEmpty());
        }
      } else {
        auto frontTextJ = Compound();
        CopyBoolValues(tag, *frontTextJ, {{u8"IgnoreLighting", u8"GlowingText", false}});
        auto color = tag.int32(u8"SignTextColor");
        if (color) {
          Rgba rgba = Rgba::FromRGB(*color);
          ColorCodeJava jcc = SignColor::MostSimilarColor(rgba);
          frontTextJ->set(u8"Color", JavaNameFromColorCodeJava(jcc));
        }
        auto text = tag.string(u8"Text", u8"");
        vector<u8string> lines = mcfile::String::Split(text, u8'\n');
        auto messagesJ = List<Tag::Type::String>();
        for (int i = 0; i < 4; i++) {
          u8string key = u8"Text" + mcfile::String::ToString(i + 1);
          u8string line = i < lines.size() ? lines[i] : u8"";
          props::Json json;
          props::SetJsonString(json, u8"text", line);
          messagesJ->push_back(String(props::StringFromJson(json)));
        }
        frontTextJ->set(u8"messages", messagesJ);

        te->set(u8"front_text", frontTextJ);
        te->set(u8"back_text", SignTextEmpty());
      }

      CopyBoolValues(tag, *te, {{u8"IsWaxed", u8"is_waxed"}});

      Result r;
      r.fTileEntity = te;
      return r;
    };
  }

  static std::optional<Result> Skull(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;

    SkullType type = static_cast<SkullType>(tag.byte(u8"SkullType", 0));
    u8string skullName = JavaNameFromSkullType(type);

    map<u8string, optional<u8string>> p;

    int facingDirection = block.fStates->int32(u8"facing_direction", 1);
    Facing6 f = Facing6FromBedrockFacingDirectionA(facingDirection);
    bool floorPlaced = f == Facing6::Up || f == Facing6::Down;
    if (floorPlaced) {
      float r = Rotation::ClampDegreesBetween0And360(tag.float32(u8"Rotation", 0));
      int rotation = (int)std::round(r + 0.5f);
      int rot = rotation * 16 / 360;
      p[u8"rotation"] = mcfile::String::ToString(rot);
    } else {
      skullName = strings::Replace(skullName, u8"_head", u8"_wall_head");
      skullName = strings::Replace(skullName, u8"_skull", u8"_wall_skull");
      p[u8"facing"] = JavaNameFromFacing6(f);
    }
    Result r;
    r.fBlock = BlockShortName(skullName, dataVersion, p);
    auto te = EmptyShortName(u8"skull", pos);
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> StructureBlock(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    auto t = EmptyShortName(u8"structure_block", pos);

    t->set(u8"showair", Bool(false));

    CopyBoolValues(tag, *t, {{u8"ignoreEntities"}, {u8"isPowered", u8"powered"}, {u8"showBoundingBox", u8"showboundingbox"}});
    CopyFloatValues(tag, *t, {{u8"integrity"}});
    CopyStringValues(tag, *t, {{u8"structureName", u8"name"}, {u8"dataField", u8"metadata"}});
    CopyIntValues(tag, *t, {
                               {u8"xStructureOffset", u8"posX"},
                               {u8"yStructureOffset", u8"posY"},
                               {u8"zStructureOffset", u8"posZ"},
                               {u8"xStructureSize", u8"sizeX"},
                               {u8"yStructureSize", u8"sizeY"},
                               {u8"zStructureSize", u8"sizeZ"},
                           });
    CopyLongValues(tag, *t, {{u8"seed"}});

    // "NONE", "LEFT_RIGHT" (displayed as "<- ->"), "FRONT_BACK" (displayed as "^v")
    auto mirrorB = tag.byte(u8"mirror", 0);
    std::u8string mirrorJ = u8"NONE";
    switch (mirrorB) {
    case 1:
      mirrorJ = u8"LEFT_RIGHT";
      break;
    case 2:
      mirrorJ = u8"FRONT_BACK";
      break;
    case 0:
    default:
      mirrorJ = u8"NONE";
      break;
    }
    t->set(u8"mirror", mirrorJ);

    // "LOAD", "SAVE", "CORNER"
    auto dataB = tag.int32(u8"data", 1);
    std::u8string mode = u8"LOAD";
    switch (dataB) {
    case 1:
      mode = u8"SAVE";
      break;
    case 3:
      mode = u8"CORNER";
      break;
    case 2:
    default:
      mode = u8"LOAD";
      break;
    }
    t->set(u8"mode", mode);

    // "NONE" (displayed as "0"), "CLOCKWISE_90" (displayed as "90"), "CLOCKWISE_180" (displayed as "180"), "COUNTERCLOCKWISE_90" (displayed as "270")
    auto rotationB = tag.byte(u8"rotation", 0);
    std::u8string rotationJ = u8"NONE";
    switch (rotationB) {
    case 1:
      rotationJ = u8"CLOCKWISE_90";
      break;
    case 2:
      rotationJ = u8"CLOCKWISE_180";
      break;
    case 3:
      rotationJ = u8"COUNTERCLOCKWISE_90";
      break;
    case 0:
    default:
      rotationJ = u8"NONE";
      break;
    }
    t->set(u8"rotation", rotationJ);

    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> BrushableBlock(Pos3i const &pos, mcfile::be::Block const &blockB, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
    using namespace std;

    auto tagJ = EmptyShortName(u8"brushable_block", pos);
    if (LootTable::BedrockToJava(tagB, *tagJ) == LootTable::State::NoLootTable) {
      if (auto itemB = tagB.compoundTag(u8"item"); itemB) {
        if (auto itemJ = Item::From(*itemB, ctx, dataVersion, {}); itemJ) {
          tagJ->set(u8"item", itemJ);
        }
      }
    }

    map<u8string, optional<u8string>> p;
    auto brushCount = tagB.int32(u8"brush_count", 0);
    p[u8"dusted"] = mcfile::String::ToString(brushCount);

    Result r;
    r.fTileEntity = tagJ;
    r.fBlock = blockJ.applying(p);
    return r;
  }
#pragma endregion

#pragma region Converter generators
  static Converter AnyStorage(std::u8string const &id) {
    return [id](Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
      auto te = EmptyFullName(u8"minecraft:" + id, pos);
      if (LootTable::BedrockToJava(tag, *te) == LootTable::State::NoLootTable) {
        auto items = ContainerItems(tag, u8"Items", ctx, dataVersion);
        if (items) {
          te->set(u8"Items", items);
        }
      }
      Result r;
      r.fTileEntity = te;
      return r;
    };
  }

  static Converter NamedEmpty(std::u8string id) {
    return [id](Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
      Result r;
      r.fTileEntity = EmptyShortName(id, pos);
      return r;
    };
  }
#pragma endregion

#pragma region Utilities
  static std::shared_ptr<mcfile::je::Block const> BlockShortName(std::u8string const &name, int dataVersion, std::map<std::u8string, std::optional<std::u8string>> props = {}) {
    return mcfile::je::Block::FromName(u8"minecraft:" + name, dataVersion)->applying(props);
  }

  static std::shared_ptr<mcfile::je::Block const> BlockFullName(std::u8string const &name, int dataVersion, std::map<std::u8string, std::optional<std::u8string>> props = {}) {
    return mcfile::je::Block::FromName(name, dataVersion)->applying(props);
  }

  static ListTagPtr ContainerItems(CompoundTag const &parent, std::u8string const &key, Context &ctx, int dataVersion) {
    using namespace std;
    auto tag = parent.listTag(key);
    if (!tag) {
      return nullptr;
    }
    auto ret = List<Tag::Type::Compound>();
    for (auto const &it : *tag) {
      CompoundTag const *c = it->asCompound();
      if (!c) {
        continue;
      }
      auto converted = Item::From(*c, ctx, dataVersion, {});
      if (!converted) {
        continue;
      }
      auto slot = c->byte(u8"Slot");
      if (!slot) {
        continue;
      }
      converted->set(u8"Slot", Byte(*slot));
      ret->push_back(converted);
    }
    return ret;
  }

  static ListTagPtr ContainerItemsWithoutSlot(CompoundTag const &parent, std::u8string const &key, Context &ctx, bool removeCount0, int dataVersion) {
    using namespace std;
    auto tag = parent.listTag(key);
    auto ret = List<Tag::Type::Compound>();
    if (!tag) {
      return ret;
    }
    for (int i = 0; i < tag->size(); i++) {
      CompoundTag const *c = tag->at(i)->asCompound();
      if (!c) {
        continue;
      }
      if (removeCount0) {
        auto count = c->byteTag(u8"Count");
        if (!count) {
          continue;
        }
        if (count->fValue < 1) {
          continue;
        }
      }
      auto converted = Item::From(*c, ctx, dataVersion, {});
      if (!converted) {
        continue;
      }
      converted->set(u8"Slot", Byte(i));
      ret->push_back(converted);
    }
    return ret;
  }

  static CompoundTagPtr EmptyFullName(std::u8string const &id, Pos3i const &pos) {
    auto tag = Compound();
    tag->set(u8"id", id);
    tag->set(u8"x", Int(pos.fX));
    tag->set(u8"y", Int(pos.fY));
    tag->set(u8"z", Int(pos.fZ));
    tag->set(u8"keepPacked", Bool(false));
    return tag;
  }

  static CompoundTagPtr EmptyShortName(std::u8string const &id, Pos3i const &pos) {
    return EmptyFullName(u8"minecraft:" + id, pos);
  }

  static std::u8string ToString(bool b) {
    return b ? u8"true" : u8"false";
  }

  static Facing6 Facing6FromBedrockCardinalDirectionMigratingFacingDirectionA(mcfile::be::Block const &block) {
    auto cardinalDirection = block.fStates->string(u8"minecraft:cardinal_direction");
    if (cardinalDirection) {
      return Facing6FromBedrockCardinalDirection(*cardinalDirection);
    } else {
      auto facingDirectionA = block.fStates->int32(u8"facing_direction", 0);
      return Facing6FromBedrockFacingDirectionA(facingDirectionA);
    }
  }

  static void AppendComponent(CompoundTagPtr &tileEntityJ, std::u8string const &nameWithoutNamespace, std::shared_ptr<Tag> const &component) {
    if (!component) {
      return;
    }
    auto components = tileEntityJ->compoundTag(u8"components");
    if (!components) {
      components = Compound();
      tileEntityJ->set(u8"components", components);
    }
    components->set(u8"minecraft:" + nameWithoutNamespace, component);
  }

#pragma endregion

  static std::unordered_map<std::u8string_view, Converter> *CreateTable() {
    using namespace std;
    auto *t = new unordered_map<u8string_view, Converter>();
#define E(__name, __conv)                    \
  assert(t->find(u8"" #__name) == t->end()); \
  t->try_emplace(u8"" #__name, __conv)

    E(flower_pot, FlowerPot);
    E(skull, Skull);
    E(bed, Bed);
    E(standing_banner, Banner);
    E(wall_banner, Banner);
    E(jukebox, Jukebox);
    E(shulker_box, ShulkerBox); // legacy, < ?
    E(white_shulker_box, ShulkerBox);
    E(orange_shulker_box, ShulkerBox);
    E(magenta_shulker_box, ShulkerBox);
    E(light_blue_shulker_box, ShulkerBox);
    E(yellow_shulker_box, ShulkerBox);
    E(lime_shulker_box, ShulkerBox);
    E(pink_shulker_box, ShulkerBox);
    E(gray_shulker_box, ShulkerBox);
    E(light_gray_shulker_box, ShulkerBox);
    E(cyan_shulker_box, ShulkerBox);
    E(purple_shulker_box, ShulkerBox);
    E(blue_shulker_box, ShulkerBox);
    E(brown_shulker_box, ShulkerBox);
    E(green_shulker_box, ShulkerBox);
    E(red_shulker_box, ShulkerBox);
    E(black_shulker_box, ShulkerBox);
    E(undyed_shulker_box, ShulkerBox);
    E(noteblock, Noteblock);
    E(chest, Chest);
    E(trapped_chest, Chest);
    E(lectern, Lectern);
    static Converter const sign = Sign(u8"sign");
    E(wall_sign, sign);
    E(standing_sign, sign);
    E(acacia_wall_sign, sign);
    E(birch_wall_sign, sign);
    E(crimson_wall_sign, sign);
    E(jungle_wall_sign, sign);
    E(spruce_wall_sign, sign);
    E(warped_wall_sign, sign);
    E(darkoak_wall_sign, sign);
    E(mangrove_wall_sign, sign);
    E(bamboo_wall_sign, sign);
    E(darkoak_standing_sign, sign);
    E(acacia_standing_sign, sign);
    E(birch_standing_sign, sign);
    E(crimson_standing_sign, sign);
    E(jungle_standing_sign, sign);
    E(spruce_standing_sign, sign);
    E(warped_standing_sign, sign);
    E(mangrove_standing_sign, sign);
    E(bamboo_standing_sign, sign);
    E(ender_chest, SameNameEmpty);
    E(enchanting_table, SameNameEmpty);
    E(barrel, AnyStorage(u8"barrel"));
    E(bell, SameNameEmpty);
    E(conduit, SameNameEmpty);
    E(campfire, Campfire);
    E(soul_campfire, Campfire);

    E(furnace, Furnace);
    E(lit_furnace, Furnace);
    E(blast_furnace, Furnace);
    E(lit_blast_furnace, Furnace);
    E(smoker, Furnace);
    E(lit_smoker, Furnace);

    E(dispenser, AnyStorage(u8"dispenser"));
    E(powered_comparator, Comparator);
    E(unpowered_comparator, Comparator);
    E(dropper, AnyStorage(u8"dropper"));
    E(brewing_stand, BrewingStand);
    E(hopper, Hopper);
    E(daylight_detector, NamedEmpty(u8"daylight_detector"));
    E(daylight_detector_inverted, NamedEmpty(u8"daylight_detector"));
    E(end_portal, SameNameEmpty);
    E(beacon, Beacon);
    E(mob_spawner, MobSpawner);
    E(command_block, CommandBlock);
    E(chain_command_block, CommandBlock);
    E(repeating_command_block, CommandBlock);
    E(structure_block, StructureBlock);
    E(beehive, Beehive);
    E(bee_nest, Beehive);

    E(sculk_sensor, SameNameEmpty);
    E(sculk_shrieker, SameNameEmpty);
    E(sculk_catalyst, SameNameEmpty);

    E(chiseled_bookshelf, ChiseledBookshelf);

    static Converter const hangingSign = Sign(u8"hanging_sign");
    E(acacia_hanging_sign, hangingSign);
    E(bamboo_hanging_sign, hangingSign);
    E(birch_hanging_sign, hangingSign);
    E(crimson_hanging_sign, hangingSign);
    E(dark_oak_hanging_sign, hangingSign);
    E(jungle_hanging_sign, hangingSign);
    E(mangrove_hanging_sign, hangingSign);
    E(oak_hanging_sign, hangingSign);
    E(spruce_hanging_sign, hangingSign);
    E(warped_hanging_sign, hangingSign);

    E(end_gateway, EndGateway);
    E(decorated_pot, DecoratedPot);
    E(suspicious_sand, BrushableBlock);
    E(suspicious_gravel, BrushableBlock);
    E(jigsaw, Jigsaw);
    E(cherry_hanging_sign, hangingSign);
    E(cherry_standing_sign, sign);
    E(cherry_wall_sign, sign);
    E(calibrated_sculk_sensor, SameNameEmpty);

    E(crafter, Crafter);
#undef E
    return t;
  }
};

std::optional<BlockEntity::Result> BlockEntity::FromBlockAndBlockEntity(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion) {
  return Impl::FromBlockAndBlockEntity(pos, block, tag, blockJ, ctx, dataVersion);
}

ListTagPtr BlockEntity::ContainerItems(CompoundTag const &parent, std::u8string const &key, Context &ctx, int dataVersion) {
  return Impl::ContainerItems(parent, key, ctx, dataVersion);
}

} // namespace je2be::bedrock
