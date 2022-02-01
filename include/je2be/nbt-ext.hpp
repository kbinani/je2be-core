#pragma once

namespace je2be {
static inline void CopyBoolValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<std::pair<std::string, std::string>> keys) {
  for (auto const &it : keys) {
    auto value = src.byte(it.first);
    if (value) {
      dest.set(it.second, props::Byte(*value));
    }
  }
}

static inline void CopyFloatValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<std::pair<std::string, std::string>> keys) {
  for (auto const &it : keys) {
    auto value = src.float32(it.first);
    if (value) {
      dest.set(it.second, props::Float(*value));
    }
  }
}

static inline void CopyIntValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<std::pair<std::string, std::string>> keys) {
  for (auto const &it : keys) {
    auto value = src.int32(it.first);
    if (value) {
      dest.set(it.second, props::Int(*value));
    }
  }
}

static inline void CopyLongValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<std::pair<std::string, std::string>> keys) {
  for (auto const &it : keys) {
    auto value = src.int64(it.first);
    if (value) {
      dest.set(it.second, props::Long(*value));
    }
  }
}

static inline void CopyStringValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<std::pair<std::string, std::string>> keys) {
  for (auto const &it : keys) {
    auto value = src.string(it.first);
    if (value) {
      dest.set(it.second, props::String(*value));
    }
  }
}
} // namespace je2be
