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
      std::atomic<u64> &done,
      uint64_t total) {
    using namespace std;
    using namespace std::placeholders;
    namespace fs = std::filesystem;
    if (total == 0) {
      return Status::Ok();
    }
    vector<Pos2i> works;
    for (auto const &it : entityStore->fChunks) {
      works.push_back(it);
    }
    EntityStore *es = entityStore.get();
    return Parallel::Process<Pos2i>(
        works,
        concurrency,
        bind(PutChunkEntities, d, _1, &db, es, progress, &done, total));
  }

private:
  static Status PutChunkEntities(
      mcfile::Dimension d,
      Pos2i chunk,
      DbInterface *db,
      EntityStore *store,
      Progress *progress,
      std::atomic<u64> *done,
      u64 total) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::be;
    string digp;
    Status st = store->entityUuids(chunk, [db, &digp](i64 uuid) {
      string prefix;
      prefix.assign((char const *)&uuid, sizeof(uuid));
      digp += prefix;
      return Status::Ok();
    });
    if (!st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    auto key = mcfile::be::DbKey::Digp(chunk.fX, chunk.fZ, d);
    if (digp.empty()) {
      if (st = db->del(key); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
    } else {
      if (st = db->put(key, leveldb::Slice(digp)); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
    }
    u64 p = done->fetch_add(1) + 1;
    if (progress) {
      if (!progress->reportEntityPostProcess({p, total})) {
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
