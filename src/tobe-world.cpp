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
  static bool PutWorldEntities(
      mcfile::Dimension d,
      DbInterface &db,
      std::shared_ptr<EntityStore> const &entityStore,
      unsigned int concurrency) {
    using namespace std;
    using namespace std::placeholders;
    namespace fs = std::filesystem;

    vector<Pos2i> works;
    for (auto const &it : entityStore->fChunks) {
      works.push_back(it);
    }
    EntityStore *es = entityStore.get();
    bool ok = Parallel::Reduce<Pos2i, bool>(
        works,
        concurrency,
        true,
        bind(PutChunkEntities, d, _1, &db, es),
        Parallel::MergeBool);
    return ok;
  }

private:
  static bool PutChunkEntities(mcfile::Dimension d, Pos2i chunk, DbInterface *db, EntityStore *store) {
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
    return true;
  }
};

bool World::PutWorldEntities(
    mcfile::Dimension d,
    DbInterface &db,
    std::shared_ptr<EntityStore> const &entityStore,
    unsigned int concurrency) {
  return Impl::PutWorldEntities(d, db, entityStore, concurrency);
}

} // namespace je2be::tobe
