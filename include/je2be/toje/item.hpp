#pragma once

namespace je2be::toje {

class Item {
  Item() = delete;
  using Converter = std::function<std::string(std::string const &, mcfile::nbt::CompoundTag const &itemB, mcfile::nbt::CompoundTag &itemJ)>;

public:
  static std::shared_ptr<mcfile::nbt::CompoundTag> From(mcfile::nbt::CompoundTag const &tagB) {
    using namespace std;
    auto name = tagB.string("Name");
    if (!name) {
      return nullptr;
    }
    static unique_ptr<unordered_map<string, Converter> const> const sTable(CreateTable());
    auto ret = make_shared<mcfile::nbt::CompoundTag>();
    auto found = sTable->find(*name);
    if (found == sTable->end()) {
      Default(*name, tagB, *ret);
    } else {
      auto renamed = found->second(*name, tagB, *ret);
      Default(renamed, tagB, *ret);
    }
    return ret;
  }

  static void Default(std::string const &name, mcfile::nbt::CompoundTag const &itemB, mcfile::nbt::CompoundTag &itemJ) {
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
      auto damage = tagB->int32("Damage");
      if (damage) {
        tagJ->set("Damage", props::Int(*damage));
      }
      itemJ.set("tag", tagJ);
    }
  }

#pragma region Converters
  static std::string Book(std::string const &name, mcfile::nbt::CompoundTag const &in, mcfile::nbt::CompoundTag &out) {
    using namespace std;
    using namespace mcfile::nbt;
    auto tagB = in.compoundTag("tag");
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
    out.set("tag", tagJ);
    return name;
  }
#pragma endregion

#pragma region Converter generators
  static Converter Rename(std::string name) {
    return [name](std::string const &, mcfile::nbt::CompoundTag const &in, mcfile::nbt::CompoundTag &out) {
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

#undef E
    return ret;
  }
};

} // namespace je2be::toje
