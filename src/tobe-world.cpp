#include <je2be/tobe/_world.hpp>

#include <je2be/tobe/options.hpp>
#include <je2be/tobe/progress.hpp>

#include <je2be/_parallel.hpp>
#include <je2be/tobe/_chunk.hpp>
#include <je2be/tobe/_java-edition-map.hpp>
#include <je2be/tobe/_level-data.hpp>
#include <je2be/tobe/_world-data.hpp>

namespace je2be::tobe {

class World::Impl {
  Impl() = delete;

public:
  static bool PutWorldEntities(mcfile::Dimension d, DbInterface &db, std::filesystem::path temp, unsigned int concurrency) {
    using namespace std;
    using namespace std::placeholders;
    namespace fs = std::filesystem;
    error_code ec;
    unordered_map<Pos2i, vector<fs::path>, Pos2iHasher> files;
    for (auto i : fs::directory_iterator(temp, ec)) {
      auto path = i.path();
      if (!fs::is_directory(path)) {
        continue;
      }
      error_code ec1;
      for (auto j : fs::directory_iterator(path, ec1)) {
        auto p = j.path();
        if (!fs::is_regular_file(p)) {
          continue;
        }
        auto name = p.filename().string();
        auto tokens = mcfile::String::Split(name, '.');
        if (tokens.size() != 4) {
          continue;
        }
        if (tokens[0] != "c" || tokens[3] != "nbt") {
          continue;
        }
        auto cx = strings::Toi(tokens[1]);
        auto cz = strings::Toi(tokens[2]);
        if (!cx || !cz) {
          continue;
        }
        files[{*cx, *cz}].push_back(p);
      }
    }

    vector<pair<Pos2i, vector<fs::path>>> works;
    for (auto const &it : files) {
      works.push_back(it);
    }
    bool ok = Parallel::Reduce<pair<Pos2i, vector<fs::path>>, bool>(
        works,
        true,
        bind(PutChunkEntities, d, _1, &db),
        Parallel::Merge);
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

bool World::PutWorldEntities(mcfile::Dimension d, DbInterface &db, std::filesystem::path temp, unsigned int concurrency) {
  return Impl::PutWorldEntities(d, db, temp, concurrency);
}

} // namespace je2be::tobe
