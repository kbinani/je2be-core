#pragma once

#include <je2be/nbt.hpp>

#include "_rename-pair.hpp"

namespace je2be {

static inline void CopyBoolValues(CompoundTag const &src, CompoundTag &dest, std::initializer_list<RenamePair<bool>> keys) {
  for (auto const &it : keys) {
    auto value = src.boolean(it.fBefore);
    if (value) {
      dest.set(it.fAfter, Bool(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, Bool(*it.fDefault));
    }
  }
}

static inline void CopyByteValues(CompoundTag const &src, CompoundTag &dest, std::initializer_list<RenamePair<u8>> keys) {
  for (auto const &it : keys) {
    auto value = src.byte(it.fBefore);
    if (value) {
      dest.set(it.fAfter, Byte(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, Byte(*it.fDefault));
    }
  }
}

static inline void CopyFloatValues(CompoundTag const &src, CompoundTag &dest, std::initializer_list<RenamePair<float>> keys) {
  for (auto const &it : keys) {
    auto value = src.float32(it.fBefore);
    if (value) {
      dest.set(it.fAfter, Float(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, Float(*it.fDefault));
    }
  }
}

static inline void CopyIntValues(CompoundTag const &src, CompoundTag &dest, std::initializer_list<RenamePair<i32>> keys) {
  for (auto const &it : keys) {
    auto value = src.int32(it.fBefore);
    if (value) {
      dest.set(it.fAfter, Int(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, Int(*it.fDefault));
    }
  }
}

static inline void CopyLongValues(CompoundTag const &src, CompoundTag &dest, std::initializer_list<RenamePair<i64>> keys) {
  for (auto const &it : keys) {
    auto value = src.int64(it.fBefore);
    if (value) {
      dest.set(it.fAfter, Long(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, Long(*it.fDefault));
    }
  }
}

static inline void CopyShortValues(CompoundTag const &src, CompoundTag &dest, std::initializer_list<RenamePair<i16>> keys) {
  for (auto const &it : keys) {
    auto value = src.int16(it.fBefore);
    if (value) {
      dest.set(it.fAfter, Short(*value));
    } else if (it.fDefault) {
      dest.set(it.fAfter, Short(*it.fDefault));
    }
  }
}

static inline void CopyStringValues(CompoundTag const &src, CompoundTag &dest, std::initializer_list<RenamePair<std::u8string>> keys) {
  for (auto const &it : keys) {
    auto value = src.string(it.fBefore);
    if (value) {
      dest.set(it.fAfter, *value);
    } else if (it.fDefault) {
      dest.set(it.fAfter, *it.fDefault);
    }
  }
}
} // namespace je2be
