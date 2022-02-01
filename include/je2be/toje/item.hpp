#pragma once

namespace je2be::toje {

class Item {
  Item() = delete;
  using Converter = std::function<std::string(std::string const &, mcfile::nbt::CompoundTag const &itemB, mcfile::nbt::CompoundTag &itemJ, Context &ctx)>;

public:
  static std::shared_ptr<mcfile::nbt::CompoundTag> From(mcfile::nbt::CompoundTag const &tagB, Context &ctx) {
    using namespace std;
    auto name = tagB.string("Name");
    if (!name) {
      return nullptr;
    }
    static unique_ptr<unordered_map<string, Converter> const> const sTable(CreateTable());
    auto ret = make_shared<mcfile::nbt::CompoundTag>();
    auto found = sTable->find(*name);
    if (found == sTable->end()) {
      Default(*name, tagB, *ret, ctx);
    } else {
      auto renamed = found->second(*name, tagB, *ret, ctx);
      Default(renamed, tagB, *ret, ctx);
    }
    return ret;
  }

  static void Default(std::string const &name, mcfile::nbt::CompoundTag const &itemB, mcfile::nbt::CompoundTag &itemJ, Context &ctx) {
    using namespace std;
    using namespace mcfile::nbt;
    auto count = itemB.byte("Count");
    itemJ.set("id", props::String(name));
    if (count) {
      itemJ.set("Count", props::Byte(*count));
    }
    auto tagB = itemB.compoundTag("tag");
    if (tagB) {
      shared_ptr<CompoundTag> tagJ = itemJ.compoundTag("tag");
      if (!tagJ) {
        tagJ = make_shared<CompoundTag>();
      }

      shared_ptr<CompoundTag> displayJ = make_shared<CompoundTag>();

      CopyIntValues(*tagB, *tagJ, {{"Damage", "Damage"}, {"RepairCost", "RepairCost"}});

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
      }

      if (displayJ && !displayJ->empty()) {
        tagJ->set("display", displayJ);
      }

      itemJ.set("tag", tagJ);
    }
  }

#pragma region Converters
  static std::string Book(std::string const &name, mcfile::nbt::CompoundTag const &itemB, mcfile::nbt::CompoundTag &itemJ, Context &ctx) {
    using namespace std;
    using namespace mcfile::nbt;
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
      tagJ->set("resolved", props::Bool(true));
    }
    auto title = tagB->string("title");
    if (title) {
      tagJ->set("title", props::String(*title));
      tagJ->set("filtered_title", props::String(*title));
    }
    itemJ.set("tag", tagJ);
    return name;
  }

  static std::string Map(std::string const &name, mcfile::nbt::CompoundTag const &itemB, mcfile::nbt::CompoundTag &itemJ, Context &ctx) {
    auto tagB = itemB.compoundTag("tag");
    if (tagB) {
      auto tagJ = std::make_shared<mcfile::nbt::CompoundTag>();
      auto index = tagB->int32("map_name_index");
      auto uuid = tagB->int64("map_uuid");
      if (index && uuid) {
        tagJ->set("map", props::Int(*index));
        ctx.addMap(*index, *uuid);
      }
      itemJ.set("tag", tagJ);
    }
    return name;
  }

  static std::string Potion(std::string const &name, mcfile::nbt::CompoundTag const &itemB, mcfile::nbt::CompoundTag &itemJ, Context &ctx) {
    auto damage = itemB.int16("Damage", 0);
    auto potionName = je2be::Potion::JavaPotionTypeFromBedrock(damage);
    auto tagJ = std::make_shared<mcfile::nbt::CompoundTag>();
    tagJ->set("Potion", props::String(potionName));
    itemJ.set("tag", tagJ);
    return name;
  }
#pragma endregion

#pragma region Converter generators
  static Converter Rename(std::string name) {
    return [name](std::string const &, mcfile::nbt::CompoundTag const &itemB, mcfile::nbt::CompoundTag &itemJ, Context &ctx) {
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

#undef E
    return ret;
  }
};

} // namespace je2be::toje
