#pragma once

namespace je2be {

static inline void CopyBoolValues(CompoundTag const &src, CompoundTag &dest, std::initializer_list<RenamePair<bool>> keys) {
  for (auto const &it : keys) {
    auto value = src.boolean(it.fBefore);
    if (value) {
      dest.set(it.fAfter, nbt::Bool(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, nbt::Bool(*it.fDefault));
    }
  }
}

static inline void CopyByteValues(CompoundTag const &src, CompoundTag &dest, std::initializer_list<RenamePair<uint8_t>> keys) {
  for (auto const &it : keys) {
    auto value = src.byte(it.fBefore);
    if (value) {
      dest.set(it.fAfter, nbt::Byte(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, nbt::Byte(*it.fDefault));
    }
  }
}

static inline void CopyFloatValues(CompoundTag const &src, CompoundTag &dest, std::initializer_list<RenamePair<float>> keys) {
  for (auto const &it : keys) {
    auto value = src.float32(it.fBefore);
    if (value) {
      dest.set(it.fAfter, nbt::Float(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, nbt::Float(*it.fDefault));
    }
  }
}

static inline void CopyIntValues(CompoundTag const &src, CompoundTag &dest, std::initializer_list<RenamePair<int32_t>> keys) {
  for (auto const &it : keys) {
    auto value = src.int32(it.fBefore);
    if (value) {
      dest.set(it.fAfter, nbt::Int(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, nbt::Int(*it.fDefault));
    }
  }
}

static inline void CopyLongValues(CompoundTag const &src, CompoundTag &dest, std::initializer_list<RenamePair<int64_t>> keys) {
  for (auto const &it : keys) {
    auto value = src.int64(it.fBefore);
    if (value) {
      dest.set(it.fAfter, nbt::Long(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, nbt::Long(*it.fDefault));
    }
  }
}

static inline void CopyShortValues(CompoundTag const &src, CompoundTag &dest, std::initializer_list<RenamePair<int16_t>> keys) {
  for (auto const &it : keys) {
    auto value = src.int16(it.fBefore);
    if (value) {
      dest.set(it.fAfter, nbt::Short(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, nbt::Short(*it.fDefault));
    }
  }
}

static inline void CopyStringValues(CompoundTag const &src, CompoundTag &dest, std::initializer_list<RenamePair<std::string>> keys) {
  for (auto const &it : keys) {
    auto value = src.string(it.fBefore);
    if (value) {
      dest.set(it.fAfter, nbt::String(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, nbt::String(*it.fDefault));
    }
  }
}
} // namespace je2be
