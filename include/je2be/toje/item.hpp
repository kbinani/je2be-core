#pragma once

namespace je2be::toje {

class Item {
  Item() = delete;
  using Converter = std::function<std::string(mcfile::nbt::CompoundTag const &in, mcfile::nbt::CompoundTag &out)>;

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
      auto renamed = found->second(tagB, *ret);
      Default(renamed, tagB, *ret);
    }
    return ret;
  }

  static Converter Rename(std::string name) {
    return [name](mcfile::nbt::CompoundTag const &in, mcfile::nbt::CompoundTag &out) {
      return "minecraft:" + name;
    };
  }

  static void Default(std::string const &name, mcfile::nbt::CompoundTag const &in, mcfile::nbt::CompoundTag &out) {
    auto count = in.byte("Count");
    out.set("id", props::String(name));
    if (count) {
      out.set("Count", props::Byte(*count));
    }
  }

  static std::unordered_map<std::string, Converter> *CreateTable() {
    using namespace std;
    auto *ret = new unordered_map<string, Converter>();

#define E(__name, __conv)                                \
  assert(ret->find("minecraft:" #__name) == ret->end()); \
  ret->insert(make_pair("minecraft:" #__name, __conv))

#undef E
    return ret;
  }
};

} // namespace je2be::toje
