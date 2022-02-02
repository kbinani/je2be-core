#pragma once

namespace je2be {
static inline void CopyBoolValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<RenamePair> keys) {
  for (auto const &it : keys) {
    auto value = src.byte(it.fBefore);
    if (value) {
      dest.set(it.fAfter, props::Byte(*value));
    }
  }
}

static inline void CopyFloatValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<RenamePair> keys) {
  for (auto const &it : keys) {
    auto value = src.float32(it.fBefore);
    if (value) {
      dest.set(it.fAfter, props::Float(*value));
    }
  }
}

static inline void CopyIntValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<RenamePair> keys) {
  for (auto const &it : keys) {
    auto value = src.int32(it.fBefore);
    if (value) {
      dest.set(it.fAfter, props::Int(*value));
    }
  }
}

static inline void CopyLongValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<RenamePair> keys) {
  for (auto const &it : keys) {
    auto value = src.int64(it.fBefore);
    if (value) {
      dest.set(it.fAfter, props::Long(*value));
    }
  }
}

static inline void CopyStringValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<RenamePair> keys) {
  for (auto const &it : keys) {
    auto value = src.string(it.fBefore);
    if (value) {
      dest.set(it.fAfter, props::String(*value));
    }
  }
}
} // namespace je2be
