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

  static std::u8string MigrateName(std::u8string const &rawName) {
    std::u8string name = Namespace::Remove(strings::SnakeFromUpperCamel(rawName));
    if (name == u8"zombie_pigman") {
      name = u8"zombified_piglin";
    } else if (name == u8"evocation_illager") {
      name = u8"evoker";
    } else if (name == u8"vindication_illager") {
      name = u8"vindicator";
    } else if (name == u8"fish") {
      name = u8"cod";
    } else if (name == u8"tropicalfish") {
      name = u8"tropical_fish";
    } else if (name == u8"ender_crystal") {
      name = u8"end_crystal";
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
    i64 seed = XXHash::Digest(uuid.c_str(), uuid.size());
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
    i8 facingB = in.byte(u8"Facing", 0);
    i8 facingJ;
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
    out->set(u8"Facing", Byte(facingJ));

    if (auto item = in.compoundTag(u8"Item"); item) {
      if (auto converted = Item::Convert(*item, ctx); converted) {
        out->set(u8"Item", converted);
      }
    }

    return true;
  }

  static bool Painting(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    auto direction = in.byte(u8"Direction", 0);
    auto f4 = Facing4FromNorth2East3South0West1(direction);

    if (auto motiveX = in.string(u8"Motive"); motiveX) {
      auto motive = Painting::MotiveFromBedrock(*motiveX);
      auto motiveJ = Painting::JavaFromMotive(motive);
      out->set(u8"Motive", String(motiveJ));

      if (auto pos = props::GetPos3d(in, u8"Pos"); pos) {
        if (auto tileOut = Painting::JavaTilePosFromBedrockPos(pos->toF(), f4, motive); tileOut) {
          out->set(u8"TileX", Int(tileOut->fX));
          out->set(u8"TileY", Int(tileOut->fY));
          out->set(u8"TileZ", Int(tileOut->fZ));
        }
      }
    }
    out->set(u8"Facing", Byte(direction));
    return true;
  }

  static bool Shulker(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    out->set(u8"Color", Byte(16));
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
      ret->set(u8"CustomName", String(props::StringFromJson(obj)));
    }

    auto idB = in.string(u8"id");
    if (!idB) {
      return nullptr;
    }
    auto idJ = MigrateName(*idB);
    ret->set(u8"id", String(idJ));

    if (auto drownedConversionTime = in.int32(u8"DrownedConversionTime"); drownedConversionTime) {
      if (*drownedConversionTime == 0) {
        ret->set(u8"DrownedConversionTime", Int(-1));
      }
    }

    CopyItems(in, *ret, ctx, u8"HandItems");
    CopyItems(in, *ret, ctx, u8"ArmorItems");
    CopyItems(in, *ret, ctx, u8"Items");

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
  ret->insert(std::make_pair(u8"" #__name, __conv))

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

std::u8string Entity::MigrateName(std::u8string const &rawName) {
  return Impl::MigrateName(rawName);
}

std::optional<Uuid> Entity::MigrateUuid(std::u8string const &uuid, Context const &ctx) {
  return Impl::MigrateUuid(uuid, ctx);
}

void Entity::CopyItems(CompoundTag const &in, CompoundTag &out, Context const &ctx, std::u8string const &key) {
  return Impl::CopyItems(in, out, ctx, key);
}

} // namespace je2be::box360
