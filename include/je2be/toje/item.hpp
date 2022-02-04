#pragma once

namespace je2be::toje {

class Item {
  Item() = delete;
  using Converter = std::function<std::string(std::string const &, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx)>;

public:
  static std::shared_ptr<CompoundTag> From(CompoundTag const &tagB, Context &ctx) {
    using namespace std;
    auto name = tagB.string("Name");
    if (!name) {
      return nullptr;
    }
    static unique_ptr<unordered_map<string, Converter> const> const sTable(CreateTable());
    auto ret = make_shared<CompoundTag>();
    if (name == "") {
      return ret;
    }
    auto found = sTable->find(*name);
    if (found == sTable->end()) {
      Default(*name, tagB, *ret, ctx);
    } else {
      auto renamed = found->second(*name, tagB, *ret, ctx);
      Default(renamed, tagB, *ret, ctx);
    }
    return ret;
  }

  static void Default(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    using namespace std;
    auto count = itemB.byte("Count");
    itemJ.set("id", props::String(name));
    if (count) {
      itemJ.set("Count", props::Byte(*count));
    }
    auto tagB = itemB.compoundTag("tag");
    if (!tagB) {
      return;
    }
    shared_ptr<CompoundTag> tagJ = itemJ.compoundTag("tag");
    if (!tagJ) {
      tagJ = make_shared<CompoundTag>();
    }

    shared_ptr<CompoundTag> displayJ = make_shared<CompoundTag>();

    CopyIntValues(*tagB, *tagJ, {{"Damage"}, {"RepairCost"}});

    auto customColor = tagB->int32("customColor");
    if (customColor) {
      int32_t c = *customColor;
      uint32_t rgb = 0xffffff & *(uint32_t *)&c;
      if (!displayJ) {
        displayJ = make_shared<CompoundTag>();
      }
      displayJ->set("color", props::Int(*(int32_t *)&rgb));
    }

    auto displayB = tagB->compoundTag("display");
    if (displayB) {
      if (!displayJ) {
        displayJ = make_shared<CompoundTag>();
      }

      auto displayName = displayB->string("Name");
      if (displayName) {
        nlohmann::json json;
        json["text"] = *displayName;
        displayJ->set("Name", props::String(nlohmann::to_string(json)));
      }
    }

    auto enchB = tagB->listTag("ench");
    if (enchB) {
      auto enchJ = make_shared<ListTag>(Tag::Type::Compound);
      for (auto const &it : *enchB) {
        auto cB = it->asCompound();
        if (!cB) {
          continue;
        }
        auto idB = cB->int16("id");
        auto lvlB = cB->int16("lvl");
        if (idB && lvlB) {
          auto cJ = make_shared<CompoundTag>();
          auto idJ = Enchantments::JavaEnchantmentIdFromBedrock(*idB);
          cJ->set("id", props::String(idJ));
          cJ->set("lvl", props::Short(*lvlB));
          enchJ->push_back(cJ);
        }
      }
      tagJ->set("Enchantments", enchJ);
    }

    if (displayJ && !displayJ->empty()) {
      tagJ->set("display", displayJ);
    }

    itemJ.set("tag", tagJ);
  }

#pragma region Converters
  static std::string Book(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    using namespace std;
    auto tagB = itemB.compoundTag("tag");
    if (!tagB) {
      return name;
    }
    auto tagJ = make_shared<CompoundTag>();
    auto pagesB = tagB->listTag("pages");
    if (pagesB) {
      auto pagesJ = make_shared<ListTag>(Tag::Type::String);
      for (auto const &pageB : *pagesB) {
        auto const *c = pageB->asCompound();
        if (!c) {
          continue;
        }
        auto text = c->string("text");
        if (!text) {
          continue;
        }
        if (name == "minecraft:written_book") {
          nlohmann::json json;
          json["text"] = *text;
          string jsonString = nlohmann::to_string(json);
          pagesJ->push_back(props::String(jsonString));
        } else {
          pagesJ->push_back(props::String(*text));
        }
      }
      tagJ->set("pages", pagesJ);
    }
    auto author = tagB->string("author");
    if (author) {
      tagJ->set("author", props::String(*author));
    }
    auto title = tagB->string("title");
    if (title) {
      tagJ->set("title", props::String(*title));
    }
    itemJ.set("tag", tagJ);
    return name;
  }

  static std::string FireworkRocket(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto tagB = itemB.compoundTag("tag");
    if (tagB) {
      auto tagJ = std::make_shared<CompoundTag>();
      auto fireworksB = tagB->compoundTag("Fireworks");
      if (fireworksB) {
        FireworksData data = FireworksData::FromBedrock(*fireworksB);
        auto fireworksJ = data.toJavaCompoundTag();
        tagJ->set("Fireworks", fireworksJ);
      }
      itemJ.set("tag", tagJ);
    }
    return "minecraft:firework_rocket";
  }

  static std::string FireworkStar(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto tagB = itemB.compoundTag("tag");
    if (tagB) {
      tagB->erase("customColor");
      auto tagJ = std::make_shared<CompoundTag>();
      auto fireworksB = tagB->compoundTag("FireworksItem");
      if (fireworksB) {
        FireworksExplosion explosion = FireworksExplosion::FromBedrock(*fireworksB);
        auto explosionJ = explosion.toJavaCompoundTag();
        tagJ->set("Explosion", explosionJ);
        itemJ["tag"] = tagJ;
      }
    }
    return "minecraft:firework_star";
  }

  static std::string Map(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto tagB = itemB.compoundTag("tag");
    if (!tagB) {
      return name;
    }
    auto tagJ = std::make_shared<CompoundTag>();
    auto uuid = tagB->int64("map_uuid");
    if (!uuid) {
      return name;
    }
    auto info = ctx.mapFromUuid(*uuid);
    if (info) {
      tagJ->set("map", props::Int(info->fNumber));
      ctx.markMapUuidAsUsed(*uuid);

      if (!info->fDecorations.empty()) {
        auto decorationsJ = std::make_shared<ListTag>(Tag::Type::Compound);
        for (MapInfo::Decoration const &decoration : info->fDecorations) {
          decorationsJ->push_back(decoration.toCompoundTag());
        }
        tagJ->set("Decorations", decorationsJ);
      }
    }
    auto damage = itemB.int16("Damage");
    std::string translate;
    if (damage == 3) {
      translate = "filled_map.monument";
    } else if (damage == 4) {
      translate = "filled_map.mansion";
    } else if (damage == 5) {
      translate = "filled_map.buried_treasure";
    }
    if (!translate.empty()) {
      auto displayJ = std::make_shared<CompoundTag>();
      nlohmann::json json;
      json["translate"] = translate;
      displayJ->set("Name", props::String(nlohmann::to_string(json)));
      tagJ->set("display", displayJ);
    }
    itemJ.set("tag", tagJ);
    return name;
  }

  static std::string Potion(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto damage = itemB.int16("Damage", 0);
    auto potionName = je2be::Potion::JavaPotionTypeFromBedrock(damage);
    auto tagJ = std::make_shared<CompoundTag>();
    tagJ->set("Potion", props::String(potionName));
    itemJ.set("tag", tagJ);
    return name;
  }
#pragma endregion

#pragma region Converter generators
  static Converter Rename(std::string name) {
    return [name](std::string const &, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
      return "minecraft:" + name;
    };
  }
#pragma endregion

  static std::unordered_map<std::string, Converter> *CreateTable() {
    using namespace std;
    auto *ret = new unordered_map<string, Converter>();

#define E(__name, __conv)                                \
  assert(ret->find("minecraft:" #__name) == ret->end()); \
  ret->insert(make_pair("minecraft:" #__name, __conv))

    E(writable_book, Book);
    E(written_book, Book);
    E(fish, Rename("cod"));
    E(potion, Potion);
    E(filled_map, Map);
    E(firework_rocket, FireworkRocket);
    E(fireworks, FireworkRocket); // legacy
    E(firework_star, FireworkStar);
    E(fireworkscharge, FireworkStar); // legacy

#undef E
    return ret;
  }
};

} // namespace je2be::toje
