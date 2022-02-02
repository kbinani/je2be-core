#pragma once

namespace je2be {
static inline void CopyBoolValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<RenamePair<bool>> keys) {
  for (auto const &it : keys) {
    auto value = src.byte(it.fBefore);
    if (value) {
      dest.set(it.fAfter, props::Byte(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, props::Byte(*it.fDefault));
    }
  }
}

static inline void CopyFloatValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<RenamePair<float>> keys) {
  for (auto const &it : keys) {
    auto value = src.float32(it.fBefore);
    if (value) {
      dest.set(it.fAfter, props::Float(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, props::Float(*it.fDefault));
    }
  }
}

static inline void CopyIntValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<RenamePair<int32_t>> keys) {
  for (auto const &it : keys) {
    auto value = src.int32(it.fBefore);
    if (value) {
      dest.set(it.fAfter, props::Int(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, props::Int(*it.fDefault));
    }
  }
}

static inline void CopyLongValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<RenamePair<int64_t>> keys) {
  for (auto const &it : keys) {
    auto value = src.int64(it.fBefore);
    if (value) {
      dest.set(it.fAfter, props::Long(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, props::Long(*it.fDefault));
    }
  }
}

static inline void CopyShortValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<RenamePair<int16_t>> keys) {
  for (auto const &it : keys) {
    auto value = src.int16(it.fBefore);
    if (value) {
      dest.set(it.fAfter, props::Short(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, props::Short(*it.fDefault));
    }
  }
}

static inline void CopyStringValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<RenamePair<std::string>> keys) {
  for (auto const &it : keys) {
    auto value = src.string(it.fBefore);
    if (value) {
      dest.set(it.fAfter, props::String(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, props::String(*it.fDefault));
    }
  }
}
} // namespace je2be
