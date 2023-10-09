#include "box360/_tile-entity.hpp"

#include "_namespace.hpp"
#include "_nbt-ext.hpp"
#include "box360/_context.hpp"
#include "box360/_item.hpp"
#include "enums/_banner-color-code-bedrock.hpp"
#include "enums/_color-code-java.hpp"
#include "enums/_skull-type.hpp"
#include "tile-entity/_loot-table.hpp"

namespace je2be::box360 {

class TileEntity::Impl {
  Impl() = delete;

  using Converter = std::function<std::optional<Result>(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx)>;

public:
  static std::optional<Result> Convert(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, Pos3i const &pos, Context const &ctx) {
    using namespace std;

    auto rawId = in.string(u8"id");
    if (!rawId) {
      return nullopt;
    }
    auto id = MigrateId(*rawId);
    assert(id.starts_with(u8"minecraft:"));

    auto const &table = GetTable();
    auto found = table.find(Namespace::Remove(id));
    if (found == table.end()) {
      return nullopt;
    }

    auto out = in.copy();
    out->set(u8"x", Int(pos.fX));
    out->set(u8"y", Int(pos.fY));
    out->set(u8"z", Int(pos.fZ));
    out->set(u8"keepPacked", Bool(false));
    if (!out->string(u8"id")) {
      out->set(u8"id", String(id));
    }

    auto ret = found->second(in, block, out, ctx);
    if (!ret) {
      return nullopt;
    }
    return ret;
  }

#pragma region Dedicated_Converters
  static std::optional<Result> Banner(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &) {
    using namespace std;
    auto base = static_cast<BannerColorCodeBedrock>(in.int32(u8"Base", 0));
    u8string color = JavaNameFromColorCodeJava(ColorCodeJavaFromBannerColorCodeBedrock(base));
    u8string name;
    if (block && block->fName.find(u8"wall") == std::string::npos) {
      name = color + u8"_banner";
    } else {
      name = color + u8"_wall_banner";
    }
    if (auto patternsB = in.listTag(u8"Patterns"); patternsB && !patternsB->empty()) {
      auto patternsJ = List<Tag::Type::Compound>();
      for (auto const &it : *patternsB) {
        auto patternB = it->asCompound();
        if (!patternB) {
          continue;
        }
        auto patternJ = Compound();
        auto p = patternB->string(u8"Pattern");
        if (!p) {
          continue;
        }
        patternJ->set(u8"Pattern", String(*p));
        auto patternColorB = patternB->int32(u8"Color");
        if (!patternColorB) {
          continue;
        }
        auto patternColor = static_cast<BannerColorCodeBedrock>(*patternColorB);
        auto patternColorJ = ColorCodeJavaFromBannerColorCodeBedrock(patternColor);
        patternJ->set(u8"Color", Int(static_cast<i32>(patternColorJ)));
        patternsJ->push_back(patternJ);
      }
      out->set(u8"Patterns", patternsJ);
    }
    Result r;
    r.fTileEntity = out;
    if (block) {
      r.fBlock = block->renamed(u8"minecraft:" + name);
    }
    return r;
  }

  static std::optional<Result> Bed(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &) {
    auto color = static_cast<ColorCodeJava>(in.int32(u8"color", 0));
    std::u8string name = JavaNameFromColorCodeJava(color) + u8"_bed";
    Result r;
    r.fTileEntity = out;
    if (block) {
      r.fBlock = block->renamed(u8"minecraft:" + name);
    }
    return r;
  }

  static std::optional<Result> BrewingStand(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx) {
    CopyShortValues(in, *out, {{u8"BrewTime"}});
    CopyByteValues(in, *out, {{u8"Fuel"}});
    Items(in, out, ctx);
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> Chest(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx) {
    if (block && block->fId == mcfile::blocks::minecraft::trapped_chest) {
      // TODO: When trapped_chest is an item
      out->set(u8"id", String(u8"minecraft:trapped_chest"));
    }
    Items(in, out, ctx);
    LootTable::Box360ToJava(in, *out);
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> Comparator(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &) {
    CopyIntValues(in, *out, {{u8"OutputSignal"}});
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> Dispenser(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx) {
    Items(in, out, ctx);
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> Dropper(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx) {
    Items(in, out, ctx);
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> EnderChest(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &) {
    out->set(u8"id", String(u8"minecraft:ender_chest"));
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> EndPortal(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx) {
    out->set(u8"id", String(u8"minecraft:end_portal"));
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> FlowerPot(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &) {
    using namespace std;
    Result r;
    u8string name;
    auto data = in.int32(u8"Data", 0);
    auto item = in.int32(u8"Item", 0);
    switch (item) {
    case 6:
      switch (data) {
      case 1:
        name = u8"spruce_sapling";
        break;
      case 2:
        name = u8"birch_sapling";
        break;
      case 3:
        name = u8"jungle_sapling";
        break;
      case 4:
        name = u8"acacia_sapling";
        break;
      case 5:
        name = u8"dark_oak_sapling";
        break;
      case 0:
      default:
        name = u8"oak_sapling";
        break;
      }
      break;
    case 31:
      name = u8"fern"; // data = 2
      break;
    case 32:
      name = u8"dead_bush";
      break;
    case 37:
      name = u8"dandelion";
      break;
    case 38:
      switch (data) {
      case 1:
        name = u8"blue_orchid";
        break;
      case 2:
        name = u8"allium";
        break;
      case 3:
        name = u8"azure_bluet";
        break;
      case 4:
        name = u8"red_tulip";
        break;
      case 5:
        name = u8"orange_tulip";
        break;
      case 6:
        name = u8"white_tulip";
        break;
      case 7:
        name = u8"pink_tulip";
        break;
      case 8:
        name = u8"oxeye_daisy";
        break;
      case 0:
      default:
        name = u8"poppy";
        break;
      }
      break;
    case 39:
      name = u8"brown_mushroom";
      break;
    case 40:
      name = u8"red_mushroom";
      break;
    case 81:
      name = u8"cactus";
      break;
    }
    if (name.empty()) {
      return nullopt;
    }
    r.fBlock = make_shared<mcfile::je::Block const>(u8"minecraft:potted_" + name);
    return r;
  }

  static std::optional<Result> Furnace(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx) {
    CopyShortValues(in, *out, {{u8"BurnTime"}, {u8"CookTime"}, {u8"CookTimeTotal"}});
    Items(in, out, ctx);
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> Identical(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx) {
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> Jukebox(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx) {
    if (auto recordItem = in.compoundTag(u8"RecordItem"); recordItem) {
      if (auto converted = Item::Convert(*recordItem, ctx); converted) {
        out->set(u8"RecordItem", converted);
      }
    }
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> MobSpawner(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx) {
    if (auto entityId = in.string(u8"EntityId"); entityId) {
      // TU0
      auto dataJ = Compound();
      auto idJ = ctx.fEntityNameMigrator(*entityId);
      auto entity = Compound();
      entity->set(u8"id", String(idJ));
      dataJ->set(u8"entity", entity);
      out->set(u8"SpawnData", dataJ);

      CopyShortValues(in, *out, {{u8"Delay"}});
    } else {
      if (auto dataB = in.compoundTag(u8"SpawnData"); dataB) {
        auto dataJ = Compound();
        if (auto idB = dataB->string(u8"id"); idB) {
          auto idJ = ctx.fEntityNameMigrator(*idB);
          auto entity = Compound();
          entity->set(u8"id", String(idJ));
          dataJ->set(u8"entity", entity);
        }
        out->set(u8"SpawnData", dataJ);
      }
      if (auto potentialsB = in.listTag(u8"SpawnPotentials"); potentialsB) {
        auto potentialsJ = List<Tag::Type::Compound>();
        for (auto const &it : *potentialsB) {
          auto potentialB = it->asCompound();
          if (!potentialB) {
            continue;
          }
          auto potentialJ = potentialB->copy();
          if (auto entity = potentialJ->compoundTag(u8"Entity"); entity) {
            if (auto idB = entity->string(u8"id"); idB) {
              entity->set(u8"id", String(ctx.fEntityNameMigrator(*idB)));
            }
          }
          potentialsJ->push_back(potentialJ);
        }
        out->set(u8"SpawnPotentials", potentialsJ);
      }
    }
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> NoteBlock(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &) {
    if (!block) {
      return std::nullopt;
    }
    auto note = in.byte(u8"note", 0);
    auto powered = in.boolean(u8"powered", false);
    Result r;
    r.fBlock = block->applying({{u8"note", mcfile::String::ToString(note)},
                                {u8"powered", powered ? u8"true" : u8"false"}});
    return r;
  }

  static std::optional<Result> ShulkerBox(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx) {
    Items(in, out, ctx);
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> Sign(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &) {
    using namespace std;
    Result r;
    for (int i = 1; i <= 4; i++) {
      u8string key = u8"Text" + mcfile::String::ToString(i);
      auto text = in.string(key);
      if (!text) {
        continue;
      }
      props::Json obj;
      props::SetJsonString(obj, u8"text", *text);
      out->set(key, String(props::StringFromJson(obj)));
    }
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> Skull(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &) {
    using namespace std;
    auto type = static_cast<SkullType>(in.byte(u8"SkullType", 0));
    auto skullName = JavaNameFromSkullType(type);
    auto rot = in.byte(u8"Rot");

    Result r;

    if (block) {
      map<u8string, optional<u8string>> props;
      u8string name;
      if (block->fName.find(u8"wall") == u8string::npos) {
        if (rot) {
          props[u8"rotation"] = mcfile::String::ToString(*rot);
        }
        name = skullName;
      } else {
        name = strings::Replace(strings::Replace(skullName, u8"_head", u8"_wall_head"), u8"_skull", u8"_wall_skull");
      }
      auto blockJ = block->renamed(u8"minecraft:" + name)->applying(props);
      r.fBlock = blockJ;
    }
    r.fTileEntity = out;
    return r;
  }
#pragma endregion

private:
#pragma region Helpers
  static std::u8string MigrateId(std::u8string const &id) {
    auto n = strings::SnakeFromUpperCamel(id);
    if (n.starts_with(u8"minecraft:")) {
      return n;
    } else {
      return u8"minecraft:" + n;
    }
  }

  static std::unordered_map<std::u8string, Converter> const &GetTable() {
    static std::unique_ptr<std::unordered_map<std::u8string, Converter> const> const sTable(CreateTable());
    return *sTable;
  }

  static void Items(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    auto items = in.listTag(u8"Items");
    if (!items) {
      return;
    }
    auto itemsJ = List<Tag::Type::Compound>();
    for (auto it : *items) {
      if (auto c = it->asCompound(); c) {
        if (auto converted = Item::Convert(*c, ctx); converted) {
          itemsJ->push_back(converted);
        }
      }
    }
    out->set(u8"Items", itemsJ);
  }

  static std::unordered_map<std::u8string, Converter> const *CreateTable() {
    auto ret = new std::unordered_map<std::u8string, Converter>();

#define E(__name, __conv)                        \
  assert(ret->find(u8"" #__name) == ret->end()); \
  ret->insert(std::make_pair(u8"" #__name, __conv))

    E(skull, Skull);
    E(banner, Banner);
    E(bed, Bed);
    E(chest, Chest);
    E(ender_Chest, EnderChest);
    E(shulker_box, ShulkerBox);
    E(flower_pot, FlowerPot);
    E(jukebox, Jukebox);
    E(sign, Sign);
    E(enchanting_table, Identical);
    E(conduit, Identical);
    E(furnace, Furnace);
    E(dropper, Dropper);
    E(dispenser, Dispenser);
    E(note_block, NoteBlock);
    E(comparator, Comparator);
    E(daylight_detector, Identical);
    E(brewing_stand, BrewingStand);
    E(end_gateway, Identical);
    E(end_portal, EndPortal);
    E(mob_spawner, MobSpawner);
    E(airportal, EndPortal);

#undef E
    return ret;
  }
#pragma endregion
};

std::optional<TileEntity::Result> TileEntity::Convert(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, Pos3i const &pos, Context const &ctx) {
  return Impl::Convert(in, block, pos, ctx);
}

} // namespace je2be::box360
