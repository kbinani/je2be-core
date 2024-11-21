#pragma once

#include <je2be/nbt.hpp>
#include <je2be/uuid.hpp>

#include "_pos3.hpp"

namespace je2be::bedrock {

class Context;

class Entity {
  class Impl;

public:
  static CompoundTagPtr ItemFrameFromBedrock(mcfile::Dimension d, Pos3i pos, mcfile::be::Block const &blockJ, CompoundTag const &blockEntityB, Context &ctx, int dataVersion);

  struct Result {
    Uuid fUuid;
    CompoundTagPtr fEntity;

    std::map<size_t, Uuid> fPassengers;
    std::optional<i64> fLeasherId;
  };

  static std::optional<Result> From(CompoundTag const &entityB, Context &ctx, int dataVersion);

  struct LocalPlayerData {
    CompoundTagPtr fEntity;
    i64 fEntityIdBedrock;
    Uuid fEntityIdJava;
    std::optional<i64> fShoulderEntityLeft;
    std::optional<i64> fShoulderEntityRight;
  };

  static std::optional<LocalPlayerData> LocalPlayer(CompoundTag const &b, Context &ctx, Uuid const *uuid, int dataVersion);

  static bool HasDefinition(CompoundTag const &entityB, std::u8string const &definitionToSearch) {
    auto definitions = entityB.listTag(u8"definitions");
    if (!definitions) {
      return false;
    }
    for (auto const &it : *definitions) {
      auto definition = it->asString();
      if (!definition) {
        continue;
      }
      if (definition->fValue == definitionToSearch) {
        return true;
      }
    }
    return false;
  }
};

} // namespace je2be::bedrock
