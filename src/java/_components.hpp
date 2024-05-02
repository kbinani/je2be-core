#pragma once

namespace je2be {
namespace java {

enum class Depth {
  Root,           // /
  Tag,            // /tag
  BlockEntityTag, // /tag/BlockEntityTag
  Display,        // /tag/display
};

template <class NbtTag>
inline std::shared_ptr<NbtTag> Migrate(CompoundTag const &root, std::u8string const &nameWithoutNamespace, Depth depth, std::u8string const &legacyName) {
  if (auto components = root.compoundTag(u8"components"); components) {
    if (auto v = components->get<NbtTag>(u8"minecraft:" + nameWithoutNamespace); v) {
      return v;
    }
  }
  switch (depth) {
  case Depth::Root:
    if (auto v = root.get<NbtTag>(legacyName); v) {
      return v;
    }
    break;
  case Depth::Tag:
    if (auto legacyTag = root.compoundTag(u8"tag"); legacyTag) {
      if (auto v = legacyTag->get<NbtTag>(legacyName); v) {
        return v;
      }
    }
    break;
  case Depth::BlockEntityTag:
    if (auto legacyTag = root.compoundTag(u8"tag"); legacyTag) {
      if (auto legacyBlockEntityTag = legacyTag->compoundTag(u8"BlockEntityTag"); legacyBlockEntityTag) {
        if (auto v = legacyBlockEntityTag->get<NbtTag>(legacyName); v) {
          return v;
        }
      }
    }
    break;
  case Depth::Display:
    if (auto legacyTag = root.compoundTag(u8"tag"); legacyTag) {
      if (auto legacyDisplayTag = legacyTag->compoundTag(u8"display"); legacyDisplayTag) {
        if (auto v = legacyDisplayTag->get<NbtTag>(legacyName); v) {
          return v;
        }
      }
    }
    break;
  }
  return nullptr;
}

template <class NbtTag>
inline std::shared_ptr<NbtTag> Migrate(CompoundTagPtr const &root, std::u8string const &nameWithoutNamespace, Depth depth, std::u8string const &legacyName) {
  if (!root) {
    return nullptr;
  }
  return Migrate<NbtTag>(*root, nameWithoutNamespace, depth, legacyName);
}

} // namespace java
} // namespace je2be
