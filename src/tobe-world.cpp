#include "tobe/_world.hpp"

#include <je2be/tobe/options.hpp>
#include <je2be/tobe/progress.hpp>

#include "_parallel.hpp"
#include "tobe/_chunk.hpp"
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
      std::unordered_map<Pos2i, std::vector<std::filesystem::path>, Pos2iHasher> const &files,
      unsigned int concurrency) {
    using namespace std;
    using namespace std::placeholders;
    namespace fs = std::filesystem;

    vector<pair<Pos2i, vector<fs::path>>> works;
    for (auto const &it : files) {
      works.push_back(it);
    }
    bool ok = Parallel::Reduce<pair<Pos2i, vector<fs::path>>, bool>(
        works,
        concurrency,
        true,
        bind(PutChunkEntities, d, _1, &db),
        Parallel::MergeBool);
    return ok;
  }

private:
  static bool PutChunkEntities(mcfile::Dimension d, std::pair<Pos2i, std::vector<std::filesystem::path>> item, DbInterface *db) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;
    using namespace mcfile::be;
    Pos2i chunk = item.first;
    string digp;
    for (auto const &file : item.second) {
      auto s = make_shared<FileInputStream>(file);
      if (!s->valid()) {
        return false;
      }
      InputStreamReader r(s, mcfile::Endian::Little);
      CompoundTag::ReadUntilEos(r, [db, &digp](auto const &c) {
        auto id = c->int64("UniqueID");
        if (!id) {
          return;
        }
        int64_t v = *id;
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
    }
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
    std::unordered_map<Pos2i, std::vector<std::filesystem::path>, Pos2iHasher> const &files,
    unsigned int concurrency) {
  return Impl::PutWorldEntities(d, db, files, concurrency);
}

} // namespace je2be::tobe
