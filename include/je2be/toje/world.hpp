#pragma once

#include <je2be/nbt.hpp>
#include <je2be/status.hpp>
#include <je2be/toje/context.hpp>

namespace je2be::toje {

class World {
  class Impl;

public:
  static Status Convert(mcfile::Dimension d,
                        std::unordered_map<Pos2i, Context::ChunksInRegion, Pos2iHasher> const &regions,
                        leveldb::DB &db,
                        std::filesystem::path root,
                        unsigned concurrency,
                        Context const &parentContext,
                        std::shared_ptr<Context> &resultContext,
                        std::function<bool(void)> progress);

private:
  static Status SquashEntityChunks(int rx, int rz, std::filesystem::path dir);

  static Status AttachLeashAndPassengers(Pos2i chunk,
                                         std::filesystem::path dir,
                                         Context &ctx,
                                         std::unordered_map<Pos2i, std::unordered_map<Uuid, int64_t, UuidHasher, UuidPred>, Pos2iHasher> const &leashedEntities,
                                         std::unordered_map<Pos2i, std::unordered_map<Uuid, std::map<size_t, Uuid>, UuidHasher, UuidPred>, Pos2iHasher> const &vehicleEntities);

  static Status AttachPassengers(Pos2i chunk,
                                 Context &ctx,
                                 ListTagPtr const &entities,
                                 std::filesystem::path dir,
                                 std::unordered_map<Pos2i, std::unordered_map<Uuid, std::map<size_t, Uuid>, UuidHasher, UuidPred>, Pos2iHasher> const &vehicleEntities);

  static void AttachLeash(Pos2i chunk,
                          Context &ctx,
                          ListTag const &entities,
                          std::unordered_map<Pos2i, std::unordered_map<Uuid, int64_t, UuidHasher, UuidPred>, Pos2iHasher> const &leashedEntities);

  static CompoundTagPtr FindEntity(ListTag const &entities, Uuid entityId);

private:
  World() = delete;
};

} // namespace je2be::toje
