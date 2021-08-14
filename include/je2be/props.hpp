#pragma once

namespace j2b::props {

inline std::shared_ptr<mcfile::nbt::ByteTag> Bool(bool b) { return std::make_shared<mcfile::nbt::ByteTag>(b ? 1 : 0); }

inline std::shared_ptr<mcfile::nbt::ByteTag> Byte(int8_t v) {
  uint8_t t = *(uint8_t *)&v;
  return std::make_shared<mcfile::nbt::ByteTag>(t);
}

inline std::shared_ptr<mcfile::nbt::IntTag> Int(int32_t v) { return std::make_shared<mcfile::nbt::IntTag>(v); }

inline std::shared_ptr<mcfile::nbt::LongTag> Long(int64_t v) { return std::make_shared<mcfile::nbt::LongTag>(v); }

inline std::shared_ptr<mcfile::nbt::StringTag> String(std::string v) { return std::make_shared<mcfile::nbt::StringTag>(v); }

inline std::shared_ptr<mcfile::nbt::FloatTag> Float(float v) { return std::make_shared<mcfile::nbt::FloatTag>(v); }

inline std::shared_ptr<mcfile::nbt::ShortTag> Short(int16_t v) { return std::make_shared<mcfile::nbt::ShortTag>(v); }

struct UUIDKeyName {
  std::optional<std::string> fLeastAndMostPrefix = std::nullopt;
  std::optional<std::string> fIntArray = std::nullopt;
  std::optional<std::string> fHexString = std::nullopt;
};

inline std::optional<int64_t> GetUUIDWithFormatLeastAndMost(mcfile::nbt::CompoundTag const &tag, std::string const &namePrefix) {
  using namespace std;
  using namespace mcfile::nbt;
  auto least = tag.int64(namePrefix + "Least");
  auto most = tag.int64(namePrefix + "Most");

  if (!least || !most)
    return nullopt;

  int64_t l = *least;
  int64_t m = *most;

  XXHash h;
  h.update((int32_t *)&m + 1, sizeof(int32_t));
  h.update((int32_t *)&m, sizeof(int32_t));
  h.update((int32_t *)&l + 1, sizeof(int32_t));
  h.update((int32_t *)&l, sizeof(int32_t));
  return h.digest();
}

inline std::optional<int64_t> GetUUIDWithFormatIntArray(mcfile::nbt::CompoundTag const &tag, std::string const &name) {
  using namespace std;
  using namespace mcfile::nbt;

  auto found = tag.find(name);
  if (found == tag.end())
    return nullopt;

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

  XXHash h;
  h.update(&a, sizeof(a));
  h.update(&b, sizeof(b));
  h.update(&c, sizeof(c));
  h.update(&d, sizeof(d));
  return h.digest();
}

inline std::optional<int64_t> GetUUIDWithFormatHexString(mcfile::nbt::CompoundTag const &tag, std::string const &name) {
  using namespace std;
  using namespace mcfile::nbt;

  auto found = tag.find(name);
  if (found == tag.end())
    return nullopt;

  auto hex = found->second->asString();
  if (!hex)
    return nullopt;

  auto s = strings::Replace(hex->fValue, "-");
  if (s.size() != 32)
    return nullopt;
  auto a0 = strings::Tol(s.substr(0, 8), 16);
  auto b0 = strings::Tol(s.substr(8, 8), 16);
  auto c0 = strings::Tol(s.substr(16, 8), 16);
  auto d0 = strings::Tol(s.substr(24), 16);

  if (!a0 || !b0 || !c0 || !d0)
    return nullopt;

  uint32_t a = (uint32_t)*a0;
  uint32_t b = (uint32_t)*b0;
  uint32_t c = (uint32_t)*c0;
  uint32_t d = (uint32_t)*d0;

  XXHash h;
  h.update(&a, sizeof(a));
  h.update(&b, sizeof(b));
  h.update(&c, sizeof(c));
  h.update(&d, sizeof(d));
  return h.digest();
}

inline std::optional<int64_t> GetUUID(mcfile::nbt::CompoundTag const &tag, UUIDKeyName keyName) {
  if (keyName.fIntArray) {
    auto ret = GetUUIDWithFormatIntArray(tag, *keyName.fIntArray);
    if (ret)
      return ret;
  }
  if (keyName.fLeastAndMostPrefix) {
    auto ret = GetUUIDWithFormatLeastAndMost(tag, *keyName.fIntArray);
    if (ret)
      return ret;
  }
  if (keyName.fHexString) {
    auto ret = GetUUIDWithFormatHexString(tag, *keyName.fHexString);
    if (ret)
      return ret;
  }
  return std::nullopt;
}

inline std::optional<Vec> GetVec(mcfile::nbt::CompoundTag const &tag, std::string const &name) {
  using namespace std;
  using namespace mcfile::nbt;
  auto found = tag.find(name);
  if (found == tag.end()) {
    return nullopt;
  }
  auto list = found->second->asList();
  if (!list) {
    return nullopt;
  }
  if (list->fType != Tag::TAG_Double || list->size() != 3) {
    return nullopt;
  }
  double x = list->at(0)->asDouble()->fValue;
  double y = list->at(1)->asDouble()->fValue;
  double z = list->at(2)->asDouble()->fValue;
  return Vec((float)x, (float)y, (float)z);
}

inline std::optional<Rotation> GetRotation(mcfile::nbt::CompoundTag const &tag, std::string const &name) {
  using namespace std;
  using namespace mcfile::nbt;
  auto found = tag.find(name);
  if (found == tag.end()) {
    return nullopt;
  }
  auto list = found->second->asList();
  if (!list) {
    return nullopt;
  }
  if (list->fType != Tag::TAG_Float || list->size() != 2) {
    return nullopt;
  }
  double yaw = list->at(0)->asFloat()->fValue;
  double pitch = list->at(1)->asFloat()->fValue;
  return Rotation(yaw, pitch);
}

inline std::optional<Pos3> GetPos3(mcfile::nbt::CompoundTag const &tag, std::string const &name) {
  auto xyz = tag.compoundTag(name);
  if (!xyz) {
    return std::nullopt;
  }
  auto x = xyz->int32("X");
  auto y = xyz->int32("Y");
  auto z = xyz->int32("Z");
  if (!x || !y || !z) {
    return std::nullopt;
  }
  return Pos3(*x, *y, *z);
}

inline std::optional<nlohmann::json> ParseAsJson(std::string const &s) {
  try {
    return nlohmann::json::parse(s);
  } catch (...) {
    return std::nullopt;
  }
}

inline std::optional<nlohmann::json> GetJson(mcfile::nbt::CompoundTag const &tag, std::string const &name) {
  using namespace std;
  using namespace mcfile::nbt;
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

} // namespace j2b::props
