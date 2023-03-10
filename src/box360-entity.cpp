#include "box360/_entity.hpp"

#include "_namespace.hpp"
#include "_xxhash.hpp"
#include "box360/_context.hpp"
#include "box360/_item.hpp"
#include "entity/_painting.hpp"
#include "tile-entity/_loot-table.hpp"

namespace je2be::box360 {

class Entity::Impl {
  Impl() = delete;

  using Converter = std::function<bool(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx)>;

public:
  static std::optional<Result> Convert(CompoundTag const &in, Context const &ctx) {
    using namespace std;

    auto rawId = in.string("id");
    if (!rawId) {
      return nullopt;
    }
    string id = MigrateName(*rawId);
    assert(id.starts_with("minecraft:"));

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

  static std::string MigrateName(std::string const &rawName) {
    std::string name = Namespace::Remove(strings::SnakeFromUpperCamel(rawName));
    if (name == "zombie_pigman") {
      name = "zombified_piglin";
    } else if (name == "evocation_illager") {
      name = "evoker";
    } else if (name == "vindication_illager") {
      name = "vindicator";
    } else if (name == "fish") {
      name = "cod";
    } else if (name == "tropicalfish") {
      name = "tropical_fish";
    } else if (name == "ender_crystal") {
      name = "end_crystal";
    }
    return "minecraft:" + name;
  }

  static std::optional<Uuid> MigrateUuid(std::string const &uuid, Context const &ctx) {
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
    i64 seed = XXHash::Digest(uuid.c_str(), uuid.size());
    return Uuid::GenWithI64Seed(seed);
  }

  static void CopyItems(CompoundTag const &in, CompoundTag &out, Context const &ctx, std::string const &key) {
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

  static bool Item(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    if (auto item = in.compoundTag("Item"); item) {
      if (auto converted = Item::Convert(*item, ctx); converted) {
        out->set("Item", converted);
      }
    }
    if (auto throwerB = in.string("Thrower"); throwerB) {
      if (auto throwerJ = MigrateUuid(*throwerB, ctx); throwerJ) {
        out->set("Thrower", throwerJ->toIntArrayTag());
      }
    }
    return true;
  }

  static bool ItemFrame(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    i8 facingB = in.byte("Facing", 0);
    i8 facingJ = 3;
    switch (facingB) {
    case 1:
      facingJ = 4;
      break;
    case 2:
      facingJ = 2;
      break;
    case 3:
      facingJ = 5;
      break;
    case 0:
    default:
      facingJ = 3;
      break;
    }
    out->set("Facing", Byte(facingJ));

    if (auto item = in.compoundTag("Item"); item) {
      if (auto converted = Item::Convert(*item, ctx); converted) {
        out->set("Item", converted);
      }
    }

    return true;
  }

  static bool Painting(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    if (auto motiveX = in.string("Motive"); motiveX) {
      auto motive = Painting::MotiveFromBedrock(*motiveX);
      auto motiveJ = Painting::JavaFromMotive(motive);
      out->set("variant", String(motiveJ));
    }
    return true;
  }

  static bool Shulker(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    out->set("Color", Byte(16));
    return true;
  }
#pragma endregion

  static CompoundTagPtr Default(CompoundTag const &in, Context const &ctx) {
    auto uuidB = in.string("UUID");
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
    ret->erase("createdOnHost");
    ret->erase("namedByRestrictedPlayer");

    ret->set("UUID", uuidJ.toIntArrayTag());

    if (auto riding = in.listTag("Riding"); riding) {
      ret->erase("Riding");
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
      ret->set("Passengers", passengers);
    }

    if (auto customNameB = in.string("CustomName"); customNameB && !customNameB->empty()) {
      nlohmann::json obj;
      obj["text"] = *customNameB;
      ret->set("CustomName", String(nlohmann::to_string(obj)));
    }

    auto idB = in.string("id");
    if (!idB) {
      return nullptr;
    }
    auto idJ = MigrateName(*idB);
    ret->set("id", String(idJ));

    if (auto drownedConversionTime = in.int32("DrownedConversionTime"); drownedConversionTime) {
      if (*drownedConversionTime == 0) {
        ret->set("DrownedConversionTime", Int(-1));
      }
    }

    CopyItems(in, *ret, ctx, "HandItems");
    CopyItems(in, *ret, ctx, "ArmorItems");
    CopyItems(in, *ret, ctx, "Items");

    return ret;
  }

  static std::optional<Uuid> MigrateEntityUuid(std::string const &uuid) {
    using namespace std;
    if (!uuid.starts_with("ent") && uuid.size() != 35) {
      return nullopt;
    }

    return Uuid::FromString(uuid.substr(3));
  }

  static std::unordered_map<std::string, Converter> const &GetTable() {
    using namespace std;
    static unique_ptr<unordered_map<string, Converter> const> const sTable(CreateTable());
    return *sTable;
  }

  static std::unordered_map<std::string, Converter> const *CreateTable() {
    auto ret = new std::unordered_map<std::string, Converter>();
#define E(__name, __conv)                   \
  assert(ret->find(#__name) == ret->end()); \
  ret->insert(std::make_pair(#__name, __conv))

    E(item_frame, ItemFrame);
    E(painting, Painting);
    E(shulker, Shulker);
    E(item, Item);
    E(chest_minecart, ChestMinecart);

#undef E
    return ret;
  }
};

std::optional<Entity::Result> Entity::Convert(CompoundTag const &in, Context const &ctx) {
  return Impl::Convert(in, ctx);
}

std::string Entity::MigrateName(std::string const &rawName) {
  return Impl::MigrateName(rawName);
}

std::optional<Uuid> Entity::MigrateUuid(std::string const &uuid, Context const &ctx) {
  return Impl::MigrateUuid(uuid, ctx);
}

void Entity::CopyItems(CompoundTag const &in, CompoundTag &out, Context const &ctx, std::string const &key) {
  return Impl::CopyItems(in, out, ctx, key);
}

} // namespace je2be::box360
