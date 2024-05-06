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

template <class NbtTag>
inline std::shared_ptr<NbtTag> FallbackPtr(CompoundTag const &c, std::initializer_list<std::u8string> names) {
  for (auto const &name : names) {
    if (auto v = c.get<NbtTag>(name); v) {
      return v;
    }
  }
  return nullptr;
}

template <class NbtTag>
inline std::shared_ptr<NbtTag> FallbackPtr(CompoundTagPtr const &c, std::initializer_list<std::u8string> names) {
  if (!c) {
    return nullptr;
  }
  return FallbackPtr<NbtTag>(*c, names);
}

inline Tag const *FallbackQuery(CompoundTag const &c, std::initializer_list<std::u8string> queries) {
  for (auto const &query : queries) {
    if (auto v = c.query(query); v && v->type() != Tag::Type::End) {
      return v;
    }
  }
  return mcfile::nbt::EndTag::Instance();
}

template <class T>
inline std::optional<T> FallbackValue(CompoundTag const &c, std::initializer_list<std::u8string> names) {
  for (auto const &name : names) {
    if (auto v = c.value<T>(name); v) {
      return v;
    }
  }
  return std::nullopt;
}

} // namespace je2be
