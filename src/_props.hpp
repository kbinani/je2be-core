#pragma once

#include <nlohmann/json.hpp>
#include <xxhash32.h>

#include <je2be/uuid.hpp>

#include "_pos3.hpp"
#include "_rotation.hpp"

namespace je2be::props {

struct UUIDKeyName {
  std::optional<std::string> fLeastAndMostPrefix = std::nullopt;
  std::optional<std::string> fIntArray = std::nullopt;
  std::optional<std::string> fHexString = std::nullopt;
};

static inline std::optional<Uuid> GetUuidWithFormatLeastAndMost(CompoundTag const &tag, std::string const &namePrefix) {
  using namespace std;
  auto least = tag.int64(namePrefix + "Least");
  auto most = tag.int64(namePrefix + "Most");

  if (!least || !most) {
    return nullopt;
  }

  i64 l = *least;
  i64 m = *most;

  Uuid uuid;
  *(u32 *)uuid.fData = *(u32 *)&m;
  *((u32 *)uuid.fData + 1) = *((u32 *)&m + 1);
  *((u32 *)uuid.fData + 2) = *(u32 *)&l;
  *((u32 *)uuid.fData + 3) = *((u32 *)&l + 1);
  return uuid;
}

static inline std::optional<Uuid> GetUuidWithFormatIntArray(CompoundTag const &tag, std::string const &name) {
  using namespace std;

  auto found = tag.find(name);
  if (found == tag.end()) {
    return nullopt;
  }

  IntArrayTag const *list = found->second->asIntArray();
  if (!list) {
    return nullopt;
  }
  vector<i32> const &value = list->value();
  if (value.size() != 4) {
    return nullopt;
  }

  i32 a = value[0];
  i32 b = value[1];
  i32 c = value[2];
  i32 d = value[3];

  return Uuid::FromInt32(a, b, c, d);
}

static inline std::optional<Uuid> GetUuidWithFormatHexString(CompoundTag const &tag, std::string const &name) {
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

  return Uuid::FromInt32((i32)*a0, (i32)*b0, (i32)*c0, (i32)*d0);
}

static inline std::optional<Uuid> GetUuid(CompoundTag const &tag, UUIDKeyName keyName) {
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

static inline std::optional<Pos3d> GetPos3d(CompoundTag const &tag, std::string const &name) {
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

static inline std::optional<Pos3f> GetPos3f(CompoundTag const &tag, std::string const &name) {
  using namespace std;
  auto found = tag.find(name);
  if (found == tag.end()) {
    return nullopt;
  }
  auto list = found->second->asList();
  if (!list) {
    return nullopt;
  }
  if (list->fType != Tag::Type::Float || list->size() != 3) {
    return nullopt;
  }
  float x = list->at(0)->asFloat()->fValue;
  float y = list->at(1)->asFloat()->fValue;
  float z = list->at(2)->asFloat()->fValue;
  return Pos3f(x, y, z);
}

static inline std::optional<Rotation> GetRotation(CompoundTag const &tag, std::string const &name) {
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
static inline std::optional<Pos3i> GetPos3i(CompoundTag const &tag) {
  auto x = tag.int32(std::string(1, xKey));
  auto y = tag.int32(std::string(1, yKey));
  auto z = tag.int32(std::string(1, zKey));
  if (!x || !y || !z) {
    return std::nullopt;
  }
  return Pos3i(*x, *y, *z);
}

static inline std::optional<Pos3i> GetPos3i(CompoundTag const &tag, std::string const &name) {
  auto xyz = tag.compoundTag(name);
  if (!xyz) {
    return std::nullopt;
  }
  return GetPos3i<'X', 'Y', 'Z'>(*xyz);
}

static inline std::optional<Pos3i> GetPos3iFromListTag(CompoundTag const &tag, std::string const &name) {
  auto arr = tag.listTag(name);
  if (!arr) {
    return std::nullopt;
  }
  if (arr->size() != 3) {
    return std::nullopt;
  }
  auto x = arr->at(0)->asInt();
  auto y = arr->at(1)->asInt();
  auto z = arr->at(2)->asInt();
  if (!x || !y || !z) {
    return std::nullopt;
  }
  return Pos3i(x->fValue, y->fValue, z->fValue);
}

static inline std::optional<Pos3i> GetPos3iFromIntArrayTag(CompoundTag const &tag, std::string const &name) {
  auto arr = tag.intArrayTag(name);
  if (!arr) {
    return std::nullopt;
  }
  if (arr->fValue.size() != 3) {
    return std::nullopt;
  }
  return Pos3i(arr->fValue[0], arr->fValue[1], arr->fValue[2]);
}

static inline std::optional<nlohmann::json> ParseAsJson(std::string const &s) {
  try {
    return nlohmann::json::parse(s);
  } catch (...) {
    return std::nullopt;
  }
}

static inline std::optional<nlohmann::json> GetJson(CompoundTag const &tag, std::string const &name) {
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

static inline i32 SquashI64ToI32(i64 v) {
  if (v < (i64)std::numeric_limits<i32>::min() || (i64)std::numeric_limits<i32>::max() < v) {
    XXHash32 x(0);
    x.add(&v, sizeof(v));
    u32 u = x.hash();
    return *(i32 *)&u;
  } else {
    return static_cast<i32>(v);
  }
}

} // namespace je2be::props
