#pragma once

#include <je2be/nbt.hpp>
#include <je2be/uuid.hpp>

namespace je2be::box360 {

class Context;

class Entity {
  class Impl;
  Entity() = delete;

public:
  struct Result {
    CompoundTagPtr fEntity;
  };

  static std::optional<Result> Convert(CompoundTag const &in, Context const &ctx);

  static std::string MigrateName(std::string const &rawName);

  static std::optional<Uuid> MigrateUuid(std::string const &uuid, Context const &ctx);

  static void CopyItems(CompoundTag const &in, CompoundTag &out, Context const &ctx, std::string const &key);
};

} // namespace je2be::box360
