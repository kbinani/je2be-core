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

template <class NbtTag>
inline void AppendComponent(CompoundTag &root, std::u8string const &nameWithoutNamespace, std::shared_ptr<NbtTag> const &component) {
  if (!component) {
    return;
  }
  auto components = root.compoundTag(u8"components");
  if (!components) {
    components = Compound();
    root[u8"components"] = components;
  }
  components->set(u8"minecraft:" + nameWithoutNamespace, component);
}

template <class NbtTag>
inline void AppendComponent(CompoundTagPtr &root, std::u8string const &nameWithoutNamespace, std::shared_ptr<NbtTag> const &component) {
  if (!root) {
    return;
  }
  AppendComponent<NbtTag>(*root, nameWithoutNamespace, component);
}

template <class NbtTag>
inline void AppendMemory(CompoundTag &root, std::u8string const &nameWithoutNamespace, std::shared_ptr<NbtTag> const &memory) {
  if (!memory) {
    return;
  }
  auto brain = root.compoundTag(u8"Brain");
  if (!brain) {
    brain = Compound();
    root[u8"Brain"] = brain;
  }
  auto memories = brain->compoundTag(u8"memories");
  if (!memories) {
    memories = Compound();
    brain->set(u8"memories", memories);
  }
  memories->set(Namespace::Add(nameWithoutNamespace), memory);
}

template <class NbtTag>
inline void AppendMemory(CompoundTagPtr &root, std::u8string const &nameWithoutNamespace, std::shared_ptr<NbtTag> const &memory) {
  if (!root) {
    return;
  }
  AppendMemory<NbtTag>(*root, nameWithoutNamespace, memory);
}

template <class NbtTag>
inline std::shared_ptr<NbtTag> GetComponent(CompoundTag const &root, std::u8string const &nameWithoutNamespace) {
  auto components = root.compoundTag(u8"components");
  if (!components) {
    return nullptr;
  }
  return components->get<NbtTag>(Namespace::Add(nameWithoutNamespace));
}

template <class NbtTag>
inline std::shared_ptr<NbtTag> GetComponent(CompoundTagPtr const &root, std::u8string const &nameWithoutNamespace) {
  if (!root) {
    return nullptr;
  }
  return GetComponent<NbtTag>(*root, nameWithoutNamespace);
}

} // namespace java
} // namespace je2be
