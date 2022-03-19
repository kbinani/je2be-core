#pragma once

namespace je2be::box360 {

class Entity {
  Entity() = delete;

public:
  struct Result {
    CompoundTagPtr fEntity;
  };

  static std::optional<Result> Convert(CompoundTag const &in) {
    Result r;
    r.fEntity = in.copy();
    return r;
  }
};

} // namespace je2be::box360
