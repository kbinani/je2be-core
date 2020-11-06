#pragma once

namespace j2b::props {

inline std::shared_ptr<mcfile::nbt::ByteTag> Bool(bool b) {
  return std::make_shared<mcfile::nbt::ByteTag>(b ? 1 : 0);
}

inline std::shared_ptr<mcfile::nbt::ByteTag> Byte(int8_t v) {
  uint8_t t = *(uint8_t *)&v;
  return std::make_shared<mcfile::nbt::ByteTag>(t);
}

inline std::shared_ptr<mcfile::nbt::IntTag> Int(int32_t v) {
  return std::make_shared<mcfile::nbt::IntTag>(v);
}

inline std::shared_ptr<mcfile::nbt::LongTag> Long(int64_t v) {
  return std::make_shared<mcfile::nbt::LongTag>(v);
}

inline std::shared_ptr<mcfile::nbt::StringTag> String(std::string v) {
  return std::make_shared<mcfile::nbt::StringTag>(v);
}

inline std::shared_ptr<mcfile::nbt::FloatTag> Float(float v) {
  return std::make_shared<mcfile::nbt::FloatTag>(v);
}

inline std::shared_ptr<mcfile::nbt::ShortTag> Short(int16_t v) {
  return std::make_shared<mcfile::nbt::ShortTag>(v);
}

inline std::optional<int32_t> GetInt(mcfile::nbt::CompoundTag const &tag,
                                     std::string const &name) {
  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end()) {
    return std::nullopt;
  }
  auto itag = found->second->asInt();
  if (!itag) {
    return std::nullopt;
  }
  return itag->asInt()->fValue;
}

inline int32_t GetIntOrDefault(mcfile::nbt::CompoundTag const &tag,
                               std::string const &name, int32_t fallback) {
  auto v = GetInt(tag, name);
  if (v) {
    return *v;
  } else {
    return fallback;
  }
}

inline std::optional<bool> GetBool(mcfile::nbt::CompoundTag const &tag,
                                   std::string const &name) {
  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end()) {
    return std::nullopt;
  }
  auto itag = found->second->asByte();
  if (!itag) {
    return std::nullopt;
  }
  return itag->asByte()->fValue != 0;
}

inline bool GetBoolOrDefault(mcfile::nbt::CompoundTag const &tag,
                             std::string const &name, bool fallback) {
  auto v = GetBool(tag, name);
  if (v) {
    return *v;
  } else {
    return fallback;
  }
}

inline std::optional<int8_t> GetByte(mcfile::nbt::CompoundTag const &tag,
                                     std::string const &name) {
  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end()) {
    return std::nullopt;
  }
  auto itag = found->second->asByte();
  if (!itag) {
    return std::nullopt;
  }
  return itag->asByte()->fValue;
}

inline int8_t GetByteOrDefault(mcfile::nbt::CompoundTag const &tag,
                               std::string const &name, int8_t fallback) {
  auto v = GetByte(tag, name);
  if (v) {
    return *v;
  } else {
    return fallback;
  }
}

inline std::optional<std::string> GetString(mcfile::nbt::CompoundTag const &tag,
                                            std::string const &name) {
  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end()) {
    return std::nullopt;
  }
  auto itag = found->second->asString();
  if (!itag) {
    return std::nullopt;
  }
  return itag->asString()->fValue;
}

inline std::string GetStringOrDefault(mcfile::nbt::CompoundTag const &tag,
                                      std::string const &name,
                                      std::string const &fallback) {
  auto v = GetString(tag, name);
  if (v) {
    return *v;
  } else {
    return fallback;
  }
}

inline std::optional<int64_t> GetLong(mcfile::nbt::CompoundTag const &tag,
                                      std::string const &name) {
  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end()) {
    return std::nullopt;
  }
  auto itag = found->second->asLong();
  if (!itag) {
    return std::nullopt;
  }
  return itag->asLong()->fValue;
}

inline int64_t GetLongOrDefault(mcfile::nbt::CompoundTag const &tag,
                                std::string const &name, int64_t fallback) {
  auto v = GetLong(tag, name);
  if (v) {
    return *v;
  } else {
    return fallback;
  }
}

inline std::optional<int16_t> GetShort(mcfile::nbt::CompoundTag const &tag,
                                       std::string const &name) {
  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end()) {
    return std::nullopt;
  }
  auto itag = found->second->asShort();
  if (!itag) {
    return std::nullopt;
  }
  return itag->asShort()->fValue;
}

inline int16_t GetShortOrDefault(mcfile::nbt::CompoundTag const &tag,
                                 std::string const &name, int16_t fallback) {
  auto v = GetShort(tag, name);
  if (v) {
    return *v;
  } else {
    return fallback;
  }
}

inline std::optional<float> GetFloat(mcfile::nbt::CompoundTag const &tag,
                                     std::string const &name) {
  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end()) {
    return std::nullopt;
  }
  auto itag = found->second->asFloat();
  if (!itag) {
    return std::nullopt;
  }
  return itag->asFloat()->fValue;
}

inline float GetFloatOrDefault(mcfile::nbt::CompoundTag const &tag,
                               std::string const &name, float fallback) {
  auto v = GetFloat(tag, name);
  if (v) {
    return *v;
  } else {
    return fallback;
  }
}

inline std::optional<double> GetDouble(mcfile::nbt::CompoundTag const &tag,
                                       std::string const &name) {
  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end()) {
    return std::nullopt;
  }
  auto itag = found->second->asDouble();
  if (!itag) {
    return std::nullopt;
  }
  return itag->asDouble()->fValue;
}

inline float GetDoubleOrDefault(mcfile::nbt::CompoundTag const &tag,
                                std::string const &name, double fallback) {
  auto v = GetDouble(tag, name);
  if (v) {
    return *v;
  } else {
    return fallback;
  }
}

struct UUIDKeyName {
  std::optional<std::string> fLeastAndMostPrefix = std::nullopt;
  std::optional<std::string> fIntArray = std::nullopt;
  std::optional<std::string> fHexString = std::nullopt;
};

inline std::optional<int64_t>
GetUUIDWithFormatLeastAndMost(mcfile::nbt::CompoundTag const &tag,
                              std::string const &namePrefix) {
  using namespace std;
  using namespace mcfile::nbt;
  auto least = GetLong(tag, namePrefix + "Least");
  auto most = GetLong(tag, namePrefix + "Most");

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

inline std::optional<int64_t>
GetUUIDWithFormatIntArray(mcfile::nbt::CompoundTag const &tag,
                          std::string const &name) {
  using namespace std;
  using namespace mcfile::nbt;

  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end())
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

inline std::optional<int64_t>
GetUUIDWithFormatHexString(mcfile::nbt::CompoundTag const &tag,
                           std::string const &name) {
  using namespace std;
  using namespace mcfile::nbt;

  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end())
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

inline std::optional<int64_t> GetUUID(mcfile::nbt::CompoundTag const &tag,
                                      UUIDKeyName keyName) {
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

inline std::optional<Vec> GetVec(mcfile::nbt::CompoundTag const &tag,
                                 std::string const &name) {
  using namespace std;
  using namespace mcfile::nbt;
  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end()) {
    return nullopt;
  }
  auto list = found->second->asList();
  if (!list) {
    return nullopt;
  }
  if (list->fType != Tag::TAG_Double || list->fValue.size() != 3) {
    return nullopt;
  }
  double x = list->fValue[0]->asDouble()->fValue;
  double y = list->fValue[1]->asDouble()->fValue;
  double z = list->fValue[2]->asDouble()->fValue;
  return Vec((float)x, (float)y, (float)z);
}

inline std::optional<Rotation> GetRotation(mcfile::nbt::CompoundTag const &tag,
                                           std::string const &name) {
  using namespace std;
  using namespace mcfile::nbt;
  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end()) {
    return nullopt;
  }
  auto list = found->second->asList();
  if (!list) {
    return nullopt;
  }
  if (list->fType != Tag::TAG_Float || list->fValue.size() != 2) {
    return nullopt;
  }
  double yaw = list->fValue[0]->asFloat()->fValue;
  double pitch = list->fValue[1]->asFloat()->fValue;
  return Rotation(yaw, pitch);
}

inline std::optional<nlohmann::json>
GetJson(mcfile::nbt::CompoundTag const &tag, std::string const &name) {
  using namespace std;
  using namespace mcfile::nbt;
  using nlohmann::json;
  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end()) {
    return nullopt;
  }
  auto s = found->second->asString();
  if (!s) {
    return nullopt;
  }
  json obj = json::parse(s->fValue);
  return obj;
}

inline std::shared_ptr<mcfile::nbt::CompoundTag>
GetCompound(mcfile::nbt::CompoundTag const &tag, std::string const &name) {
  using namespace mcfile::nbt;
  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end()) {
    return nullptr;
  }
  if (found->second->id() != Tag::TAG_Compound) {
    return nullptr;
  }
  return std::dynamic_pointer_cast<CompoundTag>(found->second);
}

inline std::shared_ptr<mcfile::nbt::ListTag>
GetList(mcfile::nbt::CompoundTag const &tag, std::string const &name) {
  using namespace mcfile::nbt;
  auto found = tag.fValue.find(name);
  if (found == tag.fValue.end()) {
    return nullptr;
  }
  if (found->second->id() != Tag::TAG_List) {
    return nullptr;
  }
  return std::dynamic_pointer_cast<ListTag>(found->second);
}

} // namespace j2b::props
