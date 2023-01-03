#include "box360/_tile-entity.hpp"

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
    auto rawId = in.string("id", "");
    if (!rawId.starts_with("minecraft:")) {
      return nullopt;
    }

    auto id = rawId.substr(10);
    auto const &table = GetTable();
    auto found = table.find(id);
    if (found == table.end()) {
      return nullopt;
    }
    auto out = in.copy();
    out->set("x", Int(pos.fX));
    out->set("y", Int(pos.fY));
    out->set("z", Int(pos.fZ));
    out->set("keepPacked", Bool(false));
    out->set("id", String(rawId));

    auto ret = found->second(in, block, out, ctx);
    if (!ret) {
      return nullopt;
    }
    return ret;
  }

#pragma region Dedicated_Converters
  static std::optional<Result> Banner(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &) {
    using namespace std;
    auto base = static_cast<BannerColorCodeBedrock>(in.int32("Base", 0));
    string color = JavaNameFromColorCodeJava(ColorCodeJavaFromBannerColorCodeBedrock(base));
    string name;
    if (block && block->fName.find("wall") == std::string::npos) {
      name = color + "_banner";
    } else {
      name = color + "_wall_banner";
    }
    if (auto patternsB = in.listTag("Patterns"); patternsB && !patternsB->empty()) {
      auto patternsJ = List<Tag::Type::Compound>();
      for (auto const &it : *patternsB) {
        auto patternB = it->asCompound();
        if (!patternsB) {
          continue;
        }
        auto patternJ = Compound();
        auto p = patternB->string("Pattern");
        if (!p) {
          continue;
        }
        patternJ->set("Pattern", String(*p));
        auto colorB = patternB->int32("Color");
        if (!colorB) {
          continue;
        }
        auto color = static_cast<BannerColorCodeBedrock>(*colorB);
        auto colorJ = ColorCodeJavaFromBannerColorCodeBedrock(color);
        patternJ->set("Color", Int(static_cast<int32_t>(colorJ)));
        patternsJ->push_back(patternJ);
      }
      out->set("Patterns", patternsJ);
    }
    Result r;
    r.fTileEntity = out;
    if (block) {
      r.fBlock = block->renamed("minecraft:" + name);
    }
    return r;
  }

  static std::optional<Result> Bed(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &) {
    auto color = static_cast<ColorCodeJava>(in.int32("color", 0));
    std::string name = JavaNameFromColorCodeJava(color) + "_bed";
    Result r;
    r.fTileEntity = out;
    if (block) {
      r.fBlock = block->renamed("minecraft:" + name);
    }
    return r;
  }

  static std::optional<Result> BrewingStand(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx) {
    CopyShortValues(in, *out, {{"BrewTime"}});
    CopyByteValues(in, *out, {{"Fuel"}});
    Items(in, out, ctx);
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> Chest(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx) {
    if (block && block->fId == mcfile::blocks::minecraft::trapped_chest) {
      // TODO: When trapped_chest is an item
      out->set("id", String("minecraft:trapped_chest"));
    }
    Items(in, out, ctx);
    LootTable::Box360ToJava(in, *out);
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> Comparator(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &) {
    CopyIntValues(in, *out, {{"OutputSignal"}});
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
    out->set("id", String("minecraft:ender_chest"));
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> FlowerPot(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &) {
    using namespace std;
    Result r;
    string name;
    auto data = in.int32("Data", 0);
    auto item = in.int32("Item", 0);
    switch (item) {
    case 6:
      switch (data) {
      case 1:
        name = "spruce_sapling";
        break;
      case 2:
        name = "birch_sapling";
        break;
      case 3:
        name = "jungle_sapling";
        break;
      case 4:
        name = "acacia_sapling";
        break;
      case 5:
        name = "dark_oak_sapling";
        break;
      case 0:
      default:
        name = "oak_sapling";
        break;
      }
      break;
    case 31:
      name = "fern"; // data = 2
      break;
    case 32:
      name = "dead_bush";
      break;
    case 37:
      name = "dandelion";
      break;
    case 38:
      switch (data) {
      case 1:
        name = "blue_orchid";
        break;
      case 2:
        name = "allium";
        break;
      case 3:
        name = "azure_bluet";
        break;
      case 4:
        name = "red_tulip";
        break;
      case 5:
        name = "orange_tulip";
        break;
      case 6:
        name = "white_tulip";
        break;
      case 7:
        name = "pink_tulip";
        break;
      case 8:
        name = "oxeye_daisy";
        break;
      case 0:
      default:
        name = "poppy";
        break;
      }
      break;
    case 39:
      name = "brown_mushroom";
      break;
    case 40:
      name = "red_mushroom";
      break;
    case 81:
      name = "cactus";
      break;
    }
    if (name.empty()) {
      return nullopt;
    }
    r.fBlock = make_shared<mcfile::je::Block const>("minecraft:potted_" + name);
    return r;
  }

  static std::optional<Result> Furnace(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx) {
    CopyShortValues(in, *out, {{"BurnTime"}, {"CookTime"}, {"CookTimeTotal"}});
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
    if (auto recordItem = in.compoundTag("RecordItem"); recordItem) {
      if (auto converted = Item::Convert(*recordItem, ctx); converted) {
        out->set("RecordItem", converted);
      }
    }
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> MobSpawner(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &ctx) {
    if (auto dataB = in.compoundTag("SpawnData"); dataB) {
      auto dataJ = Compound();
      if (auto idB = dataB->string("id"); idB) {
        auto idJ = ctx.fEntityNameMigrator(*idB);
        auto entity = Compound();
        entity->set("id", String(idJ));
        dataJ->set("entity", entity);
      }
      out->set("SpawnData", dataJ);
    }
    if (auto potentialsB = in.listTag("SpawnPotentials"); potentialsB) {
      auto potentialsJ = List<Tag::Type::Compound>();
      for (auto const &it : *potentialsB) {
        auto potentialB = it->asCompound();
        if (!potentialB) {
          continue;
        }
        auto potentialJ = potentialB->copy();
        if (auto entity = potentialJ->compoundTag("Entity"); entity) {
          if (auto idB = entity->string("id"); idB) {
            entity->set("id", String(ctx.fEntityNameMigrator(*idB)));
          }
        }
        potentialsJ->push_back(potentialJ);
      }
      out->set("SpawnPotentials", potentialsJ);
    }
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> NoteBlock(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &) {
    if (!block) {
      return std::nullopt;
    }
    auto note = in.byte("note", 0);
    auto powered = in.boolean("powered", false);
    Result r;
    r.fBlock = block->applying({{"note", std::to_string(note)},
                                {"powered", powered ? "true" : "false"}});
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
      string key = "Text" + to_string(i);
      auto text = in.string(key);
      if (!text) {
        continue;
      }
      nlohmann::json obj;
      obj["text"] = *text;
      out->set(key, String(nlohmann::to_string(obj)));
    }
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> Skull(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagPtr &out, Context const &) {
    using namespace std;
    auto type = static_cast<SkullType>(in.byte("SkullType", 0));
    auto skullName = JavaNameFromSkullType(type);
    auto rot = in.byte("Rot");

    Result r;

    if (block) {
      map<string, optional<string>> props;
      string name;
      if (block->fName.find("wall") == string::npos) {
        if (rot) {
          props["rotation"] = to_string(*rot);
        }
        name = skullName;
      } else {
        name = strings::Replace(strings::Replace(skullName, "_head", "_wall_head"), "_skull", "_wall_skull");
      }
      auto blockJ = block->renamed("minecraft:" + name)->applying(props);
      r.fBlock = blockJ;
    }
    r.fTileEntity = out;
    return r;
  }
#pragma endregion

private:
#pragma region Helpers
  static std::unordered_map<std::string, Converter> const &GetTable() {
    static std::unique_ptr<std::unordered_map<std::string, Converter> const> const sTable(CreateTable());
    return *sTable;
  }

  static void Items(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    auto items = in.listTag("Items");
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
    out->set("Items", itemsJ);
  }

  static std::unordered_map<std::string, Converter> const *CreateTable() {
    auto ret = new std::unordered_map<std::string, Converter>();

#define E(__name, __conv)                   \
  assert(ret->find(#__name) == ret->end()); \
  ret->insert(std::make_pair(#__name, __conv))

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
    E(end_portal, Identical);
    E(mob_spawner, MobSpawner);

#undef E
    return ret;
  }
#pragma endregion
};

std::optional<TileEntity::Result> TileEntity::Convert(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, Pos3i const &pos, Context const &ctx) {
  return Impl::Convert(in, block, pos, ctx);
}

} // namespace je2be::box360
