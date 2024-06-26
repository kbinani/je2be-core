#include "lce/_entity.hpp"

#include "_namespace.hpp"
#include "_nbt-ext.hpp"
#include "entity/_painting.hpp"
#include "enums/_facing6.hpp"
#include "lce/_attribute.hpp"
#include "lce/_block-data.hpp"
#include "lce/_chunk.hpp"
#include "lce/_context.hpp"
#include "lce/_item.hpp"
#include "tile-entity/_loot-table.hpp"

namespace je2be::lce {

class Entity::Impl {
  Impl() = delete;

  using Converter = std::function<bool(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx)>;

public:
  static std::optional<Result> Convert(CompoundTag const &in, Context const &ctx) {
    using namespace std;

    auto rawId = in.string(u8"id");
    if (!rawId) {
      return nullopt;
    }
    u8string id = MigrateName(*rawId);
    assert(id.starts_with(u8"minecraft:"));

    auto out = Default(in, ctx);
    if (!out) {
      return nullopt;
    }

    auto const &table = GetTable();
    auto found = table.find(Namespace::Remove(id));
    if (found == table.end()) {
      Result r;
      r.fEntity = out;
      return r;
    }

    auto ok = found->second(in, out, ctx);
    if (!ok) {
      return nullopt;
    }
    if (!out) {
      return nullopt;
    }

    Result r;
    r.fEntity = out;
    return r;
  }

  static std::unordered_map<std::u8string, std::u8string> *CreateNameMigrationTable() {
    using namespace std;
    auto ret = make_unique<unordered_map<u8string, u8string>>();
    auto &t = *ret;
    t[u8"ender_crystal"] = u8"end_crystal";
    t[u8"entity_horse"] = u8"horse";       // tu19, tu31, tu43, tu46
    t[u8"evocation_illager"] = u8"evoker"; // tu54, tu69, tu75
    t[u8"falling_sand"] = u8"falling_block";
    t[u8"fish"] = u8"cod";              // tu69, tu75
    t[u8"lava_slime"] = u8"magma_cube"; // tu9, tu19, tu31, tu43, tu46
    t[u8"minecart_chest"] = u8"chest_minecart";
    t[u8"minecart_rideable"] = u8"minecart";  // tu31, tu46
    t[u8"mushroom_cow"] = u8"mooshroom";      // tu9, tu19, tu31, tu43, tu46
    t[u8"ozelot"] = u8"ocelot";               // tu12, tu19, tu31, tu43, tu46
    t[u8"pig_zombie"] = u8"zombified_piglin"; // tu9, tu19, tu31, tu43, tu46
    t[u8"primed_tnt"] = u8"tnt";
    t[u8"snow_man"] = u8"snow_golem";            // tu31, tu43, tu46
    t[u8"snowman"] = u8"snow_golem";             // tu54, tu69, tu75
    t[u8"tropicalfish"] = u8"tropical_fish";     // tu69, tu75
    t[u8"villager_golem"] = u8"iron_golem";      // tu12, tu19, tu31, tu43, tu46, tu54, tu69, tu75
    t[u8"vindication_illager"] = u8"vindicator"; // tu54, tu69, tu75
    t[u8"wither_boss"] = u8"wither";             // tu19, tu31, tu43, tu46
    t[u8"zombie_pigman"] = u8"zombified_piglin"; // tu54, tu69, tu75
    return ret.release();
  }

  static std::u8string MigrateName(std::u8string const &rawName) {
    using namespace std;
    u8string name = Namespace::Remove(strings::SnakeFromUpperCamel(rawName));
    static unique_ptr<unordered_map<u8string, u8string> const> const sTable(CreateNameMigrationTable());
    if (auto found = sTable->find(name); found != sTable->end()) {
      name = found->second;
    }
    return u8"minecraft:" + name;
  }

  static std::optional<Uuid> MigrateUuid(std::u8string const &uuid, Context const &ctx) {
    using namespace std;
    if (uuid.empty()) {
      return nullopt;
    }
    auto found = ctx.fPlayers.find(uuid);
    if (found != ctx.fPlayers.end()) {
      return found->second;
    }
    if (auto u = MigrateEntityUuid(uuid); u) {
      return *u;
    }
    i64 seed = mcfile::XXHash<i64>::Digest(uuid.c_str(), uuid.size());
    return Uuid::GenWithI64Seed(seed);
  }

  static void CopyItems(CompoundTag const &in, CompoundTag &out, Context const &ctx, std::u8string const &key) {
    auto handItemsB = in.listTag(key);
    if (!handItemsB) {
      return;
    }
    auto handItemsJ = List<Tag::Type::Compound>();
    for (auto const &it : *handItemsB) {
      auto itemB = it->asCompound();
      if (!itemB) {
        continue;
      }
      if (itemB->empty()) {
        handItemsJ->push_back(Compound());
      } else if (auto converted = Item::Convert(*itemB, ctx); converted) {
        handItemsJ->push_back(converted);
      }
    }
    out[key] = handItemsJ;
  }

private:
#pragma region Converters
  static bool ChestMinecart(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    LootTable::Box360ToJava(in, *out);
    return true;
  }

  static bool FallingBlock(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    out->set(u8"id", u8"minecraft:falling_block");
    auto block = in.string(u8"Block");
    auto tile = in.byte(u8"Tile");
    auto data = in.byte(u8"Data", 0);
    if (tile) {
      if (auto b = BlockData(*tile, data).toBlock(); b) {
        out->set(u8"BlockState", b->toCompoundTag());
      }
    } else if (block) {
      auto b = mcfile::je::Block::FromName(*block, Chunk::kTargetDataVersion);
      out->set(u8"BlockState", b->toCompoundTag());
    }
    out->erase(u8"Block");
    out->erase(u8"Tile");
    out->erase(u8"Data");
    return true;
  }

  static bool Guardian(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    if (in.boolean(u8"Elder", false)) {
      out->set(u8"id", u8"minecraft:elder_guardian");
    } else {
      out->set(u8"id", u8"minecraft:guardian");
    }
    out->erase(u8"Elder");
    return true;
  }

  static bool Horse(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    auto type = in.int32(u8"Type", 0);
    switch (type) {
    case 1:
      out->set(u8"id", u8"minecraft:donkey");
      break;
    case 2:
      out->set(u8"id", u8"minecraft:mule");
      break;
    case 0:
    default:
      out->set(u8"id", u8"minecraft:horse");
      break;
    }
    out->erase(u8"Type");
    return true;
  }

  static bool Item(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    if (auto item = in.compoundTag(u8"Item"); item) {
      if (auto converted = Item::Convert(*item, ctx); converted) {
        out->set(u8"Item", converted);
      }
    }
    if (auto throwerB = in.string(u8"Thrower"); throwerB) {
      if (auto throwerJ = MigrateUuid(*throwerB, ctx); throwerJ) {
        out->set(u8"Thrower", throwerJ->toIntArrayTag());
      }
    }
    return true;
  }

  static bool ItemFrame(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    // (tu16)Direction  (tu16)Dir  (j)Facing  physical facing
    // 2                0          2          north
    // 3                3          5          east
    // 0                2          3          south
    // 1                1          4          west
    // N/A              N/A        1          up
    // N/A              N/A        0          down

    auto directionB = in.byte(u8"Direction");
    i8 facingB = in.byte(u8"Facing", 0);
    i8 f = directionB ? *directionB : facingB;

    Facing6 facing;
    i8 facingJ;
    Rotation rot(0, 0);
    switch (f) {
    case 1:
      facing = Facing6::West;
      facingJ = 4;
      rot = Rotation(90, 0);
      break;
    case 2:
      facingJ = 2;
      facing = Facing6::North;
      rot = Rotation(180, 0);
      break;
    case 3:
      facingJ = 5;
      facing = Facing6::East;
      rot = Rotation(270, 0);
      break;
    case 0:
    default:
      facingJ = 3;
      facing = Facing6::South;
      break;
    }
    out->set(u8"Facing", Byte(facingJ));
    out->erase(u8"Dir");
    out->erase(u8"Direction");
    out->set(u8"Rotation", rot.toListTag());

    auto xTileX = in.int32(u8"TileX");
    auto xTileY = in.int32(u8"TileY");
    auto xTileZ = in.int32(u8"TileZ");
    if (xTileX && xTileY && xTileZ) {
      Pos3i vec = Pos3iFromFacing6(facing);
      Pos3i jTile;
      if (directionB) {
        jTile = Pos3i(*xTileX, *xTileY, *xTileZ) + vec;
      } else {
        jTile = Pos3i(*xTileX, *xTileY, *xTileZ);
      }
      out->set(u8"TileX", Int(jTile.fX));
      out->set(u8"TileY", Int(jTile.fY));
      out->set(u8"TileZ", Int(jTile.fZ));
    }

    if (auto item = in.compoundTag(u8"Item"); item) {
      if (auto converted = Item::Convert(*item, ctx); converted) {
        out->set(u8"Item", converted);
      }
    }

    return true;
  }

  static bool Ocelot(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    auto owner = in.string(u8"Owner", u8"");         // tu19
    auto ownerUUID = in.string(u8"OwnerUUID", u8""); // tu31
    auto catType = in.int32(u8"CatType");
    if ((!owner.empty() || !ownerUUID.empty()) && catType) {
      out->set(u8"id", u8"minecraft:cat");
    } else {
      out->set(u8"id", u8"minecraft:ocelot");
      out->erase(u8"CatType");
    }
    return true;
  }

  static bool Painting(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    auto direction = in.byte(u8"Direction", 0);
    auto f4 = Facing4FromNorth2East3South0West1(direction);

    if (auto motiveX = in.string(u8"Motive"); motiveX) {
      auto motive = Painting::MotiveFromBedrock(*motiveX);
      auto motiveJ = Painting::JavaMotiveFromBedrockMotif(motive);
      out->set(u8"Motive", motiveJ);

      if (auto pos = props::GetPos3d(in, u8"Pos"); pos) {
        if (auto tileOut = Painting::JavaTilePosFromLegacyConsolePos(pos->toF(), f4, motive); tileOut) {
          out->set(u8"TileX", Int(tileOut->fX));
          out->set(u8"TileY", Int(tileOut->fY));
          out->set(u8"TileZ", Int(tileOut->fZ));
        }
      }
    }
    out->set(u8"Facing", Byte(direction));
    out->erase(u8"Dir");
    out->erase(u8"Direction");
    return true;
  }

  static bool Shulker(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    out->set(u8"Color", Byte(16));
    return true;
  }

  static bool Skeleton(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    ListTagPtr equipment = in.listTag(u8"Equipment");
    if (!equipment) {
      // Equipment doesn't exist even when skeleton owns bow
      // tu9, tu12, tu14
      auto handItems = List<Tag::Type::Compound>();
      auto bow = Compound();
      bow->set(u8"id", u8"minecraft:bow");
      bow->set(u8"Count", Byte(1));
      auto tag = Compound();
      tag->set(u8"Damage", Short(0));
      bow->set(u8"tag", tag);
      handItems->push_back(bow);
      handItems->push_back(Compound());
      out->set(u8"HandItems", handItems);
    }
    return true;
  }

  static bool SnowGolem(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    auto glowing = in.boolean(u8"Glowing", false); // tu46
    out->set(u8"Pumpkin", Bool(!glowing));
    out->erase(u8"Glowing");
    return true;
  }

  static bool ZombifiedPiglin(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    ListTagPtr equipment = in.listTag(u8"Equipment");
    if (!equipment) {
      // Equipment doesn't exist even when skeleton owns golden_sword
      // tu9, tu12, tu14
      auto handItems = List<Tag::Type::Compound>();
      auto bow = Compound();
      bow->set(u8"id", u8"minecraft:golden_sword");
      bow->set(u8"Count", Byte(1));
      auto tag = Compound();
      tag->set(u8"Damage", Short(0));
      bow->set(u8"tag", tag);
      handItems->push_back(bow);
      handItems->push_back(Compound());
      out->set(u8"HandItems", handItems);
    }
    return true;
  }
#pragma endregion

  static CompoundTagPtr Default(CompoundTag const &in, Context const &ctx) {
    auto uuidB = in.string(u8"UUID");
    Uuid uuidJ;
    if (uuidB) {
      if (auto migrated = MigrateUuid(*uuidB, ctx); migrated) {
        uuidJ = *migrated;
      } else {
        return nullptr;
      }
    } else {
      uuidJ = Uuid::Gen();
    }

    auto ret = in.copy();
    ret->erase(u8"createdOnHost");
    ret->erase(u8"namedByRestrictedPlayer");

    ret->set(u8"UUID", uuidJ.toIntArrayTag());

    if (auto riding = in.listTag(u8"Riding"); riding) {
      ret->erase(u8"Riding");
      auto passengers = List<Tag::Type::Compound>();
      for (auto const &it : *riding) {
        auto rider = it->asCompound();
        if (!rider) {
          continue;
        }
        auto converted = Convert(*rider, ctx);
        if (!converted) {
          continue;
        }
        passengers->push_back(converted->fEntity);
      }
      ret->set(u8"Passengers", passengers);
    }

    if (auto customNameB = in.string(u8"CustomName"); customNameB && !customNameB->empty()) {
      props::Json obj;
      props::SetJsonString(obj, u8"text", *customNameB);
      ret->set(u8"CustomName", props::StringFromJson(obj));
    }

    auto idB = in.string(u8"id");
    if (!idB) {
      return nullptr;
    }
    auto idJ = MigrateName(*idB);
    ret->set(u8"id", idJ);

    if (auto drownedConversionTime = in.int32(u8"DrownedConversionTime"); drownedConversionTime) {
      if (*drownedConversionTime == 0) {
        ret->set(u8"DrownedConversionTime", Int(-1));
      }
    }

    if (auto equipment = in.listTag(u8"Equipment"); equipment && equipment->size() >= 5) {
      auto handItems = List<Tag::Type::Compound>();
      if (auto mainHandX = equipment->at(0)->asCompound(); mainHandX) {
        if (auto mainHandJ = Item::Convert(*mainHandX, ctx); mainHandJ) {
          handItems->push_back(mainHandJ);
        } else {
          handItems->push_back(Compound());
        }
      } else {
        handItems->push_back(Compound());
      }
      handItems->push_back(Compound());
      ret->set(u8"HandItems", handItems);

      auto armorItems = List<Tag::Type::Compound>();
      for (size_t i = 1; i <= 4; i++) {
        if (auto itemX = equipment->at(i)->asCompound(); itemX) {
          if (auto itemJ = Item::Convert(*itemX, ctx); itemJ) {
            armorItems->push_back(itemJ);
          } else {
            armorItems->push_back(Compound());
          }
        } else {
          armorItems->push_back(Compound());
        }
      }
      ret->set(u8"ArmorItems", armorItems);

      ret->erase(u8"Equipment");
    } else {
      // tu46
      CopyItems(in, *ret, ctx, u8"HandItems");
      CopyItems(in, *ret, ctx, u8"ArmorItems");
    }

    CopyItems(in, *ret, ctx, u8"Items");

    for (std::u8string key : {u8"Owner", u8"OwnerUUID"}) {
      if (auto ownerX = in.string(key); ownerX && !ownerX->empty()) {
        if (auto ownerJ = MigrateUuid(*ownerX, ctx); ownerJ) {
          ret->set(u8"Owner", ownerJ->toIntArrayTag());
          break;
        }
      }
    }

    for (std::u8string key : {u8"SaddleItem", u8"ArmorItem", u8"DecorItem"}) {
      if (auto itemX = in.compoundTag(key); itemX) {
        if (auto itemJ = Item::Convert(*itemX, ctx); itemJ) {
          ret->set(key, itemJ);
        }
      }
    }

    if (auto attributesX = in.listTag(u8"Attributes"); attributesX) {
      auto attributesJ = List<Tag::Type::Compound>();
      for (auto const &it : *attributesX) {
        if (auto c = it->asCompound(); c) {
          if (auto j = Attribute::Convert(*c, *idB); j) {
            attributesJ->push_back(j);
          }
        }
      }
      ret->set(u8"Attributes", attributesJ);
    }

    return ret;
  }

  static std::optional<Uuid> MigrateEntityUuid(std::u8string const &uuid) {
    using namespace std;
    if (!uuid.starts_with(u8"ent") && uuid.size() != 35) {
      return nullopt;
    }

    return Uuid::FromString(uuid.substr(3));
  }

  static std::unordered_map<std::u8string, Converter> const &GetTable() {
    using namespace std;
    static unique_ptr<unordered_map<u8string, Converter> const> const sTable(CreateTable());
    return *sTable;
  }

  static std::unordered_map<std::u8string, Converter> const *CreateTable() {
    auto ret = new std::unordered_map<std::u8string, Converter>();
#define E(__name, __conv)                        \
  assert(ret->find(u8"" #__name) == ret->end()); \
  ret->try_emplace(u8"" #__name, __conv)

    E(chest_minecart, ChestMinecart);
    E(falling_block, FallingBlock);
    E(guardian, Guardian);
    E(item, Item);
    E(item_frame, ItemFrame);
    E(horse, Horse);
    E(ocelot, Ocelot);
    E(painting, Painting);
    E(shulker, Shulker);
    E(skeleton, Skeleton);
    E(snow_golem, SnowGolem);
    E(zombified_piglin, ZombifiedPiglin);

#undef E
    return ret;
  }
};

std::optional<Entity::Result> Entity::Convert(CompoundTag const &in, Context const &ctx) {
  return Impl::Convert(in, ctx);
}

std::u8string Entity::MigrateName(std::u8string const &rawName) {
  return Impl::MigrateName(rawName);
}

std::optional<Uuid> Entity::MigrateUuid(std::u8string const &uuid, Context const &ctx) {
  return Impl::MigrateUuid(uuid, ctx);
}

void Entity::CopyItems(CompoundTag const &in, CompoundTag &out, Context const &ctx, std::u8string const &key) {
  return Impl::CopyItems(in, out, ctx, key);
}

} // namespace je2be::lce
