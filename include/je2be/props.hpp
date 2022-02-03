#pragma once

namespace je2be::props {

inline std::shared_ptr<ByteTag> Bool(bool b) { return std::make_shared<ByteTag>(b ? 1 : 0); }

inline std::shared_ptr<ByteTag> Byte(int8_t v) {
  uint8_t t = *(uint8_t *)&v;
  return std::make_shared<ByteTag>(t);
}

inline std::shared_ptr<IntTag> Int(int32_t v) { return std::make_shared<IntTag>(v); }

inline std::shared_ptr<LongTag> Long(int64_t v) { return std::make_shared<LongTag>(v); }

inline std::shared_ptr<StringTag> String(std::string v) { return std::make_shared<StringTag>(v); }

inline std::shared_ptr<FloatTag> Float(float v) { return std::make_shared<FloatTag>(v); }

inline std::shared_ptr<ShortTag> Short(int16_t v) { return std::make_shared<ShortTag>(v); }

struct UUIDKeyName {
  std::optional<std::string> fLeastAndMostPrefix = std::nullopt;
  std::optional<std::string> fIntArray = std::nullopt;
  std::optional<std::string> fHexString = std::nullopt;
};

inline std::optional<Uuid> GetUuidWithFormatLeastAndMost(CompoundTag const &tag, std::string const &namePrefix) {
  using namespace std;
  auto least = tag.int64(namePrefix + "Least");
  auto most = tag.int64(namePrefix + "Most");

  if (!least || !most) {
    return nullopt;
  }

  int64_t l = *least;
  int64_t m = *most;

  Uuid uuid;
  uuid.f1 = *(uint32_t *)&m;
  uuid.f2 = *((uint32_t *)&m + 1);
  uuid.f3 = *(uint32_t *)&l;
  uuid.f4 = *((uint32_t *)&l + 1);
  return uuid;
}

inline std::optional<Uuid> GetUuidWithFormatIntArray(CompoundTag const &tag, std::string const &name) {
  using namespace std;

  auto found = tag.find(name);
  if (found == tag.end()) {
    return nullopt;
  }

  IntArrayTag const *list = found->second->asIntArray();
  if (!list) {
    return nullopt;
  }
  vector<int32_t> const &value = list->value();
  if (value.size() != 4) {
    return nullopt;
  }

  int32_t a = value[0];
  int32_t b = value[1];
  int32_t c = value[2];
  int32_t d = value[3];

  Uuid uuid;
  uuid.f1 = *(uint32_t *)&a;
  uuid.f2 = *(uint32_t *)&b;
  uuid.f3 = *(uint32_t *)&c;
  uuid.f4 = *(uint32_t *)&d;
  return uuid;
}

inline std::optional<Uuid> GetUuidWithFormatHexString(CompoundTag const &tag, std::string const &name) {
  using namespace std;

  auto found = tag.find(name);
  if (found == tag.end()) {
    return nullopt;
  }

  auto hex = found->second->asString();
  if (!hex) {
    return nullopt;
  }

  auto s = strings::Remove(hex->fValue, "-");
  if (s.size() != 32) {
    return nullopt;
  }
  auto a0 = strings::Tol(s.substr(0, 8), 16);
  auto b0 = strings::Tol(s.substr(8, 8), 16);
  auto c0 = strings::Tol(s.substr(16, 8), 16);
  auto d0 = strings::Tol(s.substr(24), 16);

  if (!a0 || !b0 || !c0 || !d0) {
    return nullopt;
  }

  Uuid uuid;
  uuid.f1 = (uint32_t)*a0;
  uuid.f2 = (uint32_t)*b0;
  uuid.f3 = (uint32_t)*c0;
  uuid.f4 = (uint32_t)*d0;
  return uuid;
}

inline std::optional<Uuid> GetUuid(CompoundTag const &tag, UUIDKeyName keyName) {
  if (keyName.fIntArray) {
    auto ret = GetUuidWithFormatIntArray(tag, *keyName.fIntArray);
    if (ret) {
      return ret;
    }
  }
  if (keyName.fLeastAndMostPrefix) {
    auto ret = GetUuidWithFormatLeastAndMost(tag, *keyName.fIntArray);
    if (ret) {
      return ret;
    }
  }
  if (keyName.fHexString) {
    auto ret = GetUuidWithFormatHexString(tag, *keyName.fHexString);
    if (ret) {
      return ret;
    }
  }
  return std::nullopt;
}

inline std::optional<Pos3d> GetPos3d(CompoundTag const &tag, std::string const &name) {
  using namespace std;
  auto found = tag.find(name);
  if (found == tag.end()) {
    return nullopt;
  }
  auto list = found->second->asList();
  if (!list) {
    return nullopt;
  }
  if (list->fType != Tag::Type::Double || list->size() != 3) {
    return nullopt;
  }
  double x = list->at(0)->asDouble()->fValue;
  double y = list->at(1)->asDouble()->fValue;
  double z = list->at(2)->asDouble()->fValue;
  return Pos3d(x, y, z);
}

inline std::optional<Rotation> GetRotation(CompoundTag const &tag, std::string const &name) {
  using namespace std;
  auto found = tag.find(name);
  if (found == tag.end()) {
    return nullopt;
  }
  auto list = found->second->asList();
  if (!list) {
    return nullopt;
  }
  if (list->fType != Tag::Type::Float || list->size() != 2) {
    return nullopt;
  }
  double yaw = list->at(0)->asFloat()->fValue;
  double pitch = list->at(1)->asFloat()->fValue;
  return Rotation(yaw, pitch);
}

template <char xKey, char yKey, char zKey>
inline std::optional<Pos3i> GetPos3i(CompoundTag const &tag) {
  auto x = tag.int32(std::string(1, xKey));
  auto y = tag.int32(std::string(1, yKey));
  auto z = tag.int32(std::string(1, zKey));
  if (!x || !y || !z) {
    return std::nullopt;
  }
  return Pos3i(*x, *y, *z);
}

inline std::optional<Pos3i> GetPos3i(CompoundTag const &tag, std::string const &name) {
  auto xyz = tag.compoundTag(name);
  if (!xyz) {
    return std::nullopt;
  }
  return GetPos3i<'X', 'Y', 'Z'>(*xyz);
}

inline std::optional<nlohmann::json> ParseAsJson(std::string const &s) {
  try {
    return nlohmann::json::parse(s);
  } catch (...) {
    return std::nullopt;
  }
}

inline std::optional<nlohmann::json> GetJson(CompoundTag const &tag, std::string const &name) {
  using namespace std;
  using nlohmann::json;
  auto found = tag.find(name);
  if (found == tag.end()) {
    return nullopt;
  }
  auto s = found->second->asString();
  if (!s) {
    return nullopt;
  }
  return ParseAsJson(s->fValue);
}

inline int32_t SquashI64ToI32(int64_t v) {
  if (v < (int64_t)std::numeric_limits<int32_t>::min() || (int64_t)std::numeric_limits<int32_t>::max() < v) {
    XXHash32 x(0);
    x.add(&v, sizeof(v));
    uint32_t u = x.hash();
    return *(int32_t *)&u;
  } else {
    return static_cast<int32_t>(v);
  }
}

} // namespace je2be::props
