#include "tobe/_world.hpp"

#include <je2be/fs.hpp>
#include <je2be/tobe/options.hpp>
#include <je2be/tobe/progress.hpp>

#include "_parallel.hpp"
#include "tobe/_chunk.hpp"
#include "tobe/_entity-store.hpp"
#include "tobe/_java-edition-map.hpp"
#include "tobe/_level-data.hpp"
#include "tobe/_world-data.hpp"

namespace je2be::tobe {

class World::Impl {
  Impl() = delete;

public:
  static Status PutWorldEntities(
      mcfile::Dimension d,
      DbInterface &db,
      std::shared_ptr<EntityStore> const &entityStore,
      unsigned int concurrency,
      Progress *progress,
      std::atomic_uint64_t &done,
      uint64_t total) {
    using namespace std;
    using namespace std::placeholders;
    namespace fs = std::filesystem;

    vector<Pos2i> works;
    for (auto const &it : entityStore->fChunks) {
      works.push_back(it);
    }
    EntityStore *es = entityStore.get();
    return Parallel::Reduce<Pos2i, Status>(
        works,
        concurrency,
        Status::Ok(),
        bind(PutChunkEntities, d, _1, &db, es, progress, &done, total),
        Status::Merge);
  }

private:
  static Status PutChunkEntities(
      mcfile::Dimension d,
      Pos2i chunk,
      DbInterface *db,
      EntityStore *store,
      Progress *progress,
      std::atomic_uint64_t *done,
      u64 total) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::be;
    string digp;
    store->entities(chunk, [db, &digp](CompoundTagPtr const &c) {
      auto id = c->int64("UniqueID");
      if (!id) {
        return;
      }
      i64 v = *id;
      string prefix;
      prefix.assign((char const *)&v, sizeof(v));
      digp += prefix;

      auto key = DbKey::Actorprefix(prefix);
      auto value = CompoundTag::Write(*c, Endian::Little);
      if (!value) {
        return;
      }
      db->put(key, leveldb::Slice(*value));
    });
    auto key = mcfile::be::DbKey::Digp(chunk.fX, chunk.fZ, d);
    if (digp.empty()) {
      db->del(key);
    } else {
      db->put(key, leveldb::Slice(digp));
    }
    u64 p = done->fetch_add(1) + 1;
    if (progress) {
      if (!progress->reportEntityPostProcess(p / double(total))) {
        return JE2BE_ERROR;
      }
    }
    return Status::Ok();
  }
};

Status World::PutWorldEntities(
    mcfile::Dimension d,
    DbInterface &db,
    std::shared_ptr<EntityStore> const &entityStore,
    unsigned int concurrency,
    Progress *progress,
    std::atomic_uint64_t &done,
    u64 total) {
  return Impl::PutWorldEntities(d, db, entityStore, concurrency, progress, done, total);
}

} // namespace je2be::tobe
