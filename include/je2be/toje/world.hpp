#pragma once

namespace je2be::toje {

class World {
public:
  static std::shared_ptr<Context> Convert(mcfile::Dimension d, leveldb::DB &db, std::filesystem::path root, unsigned concurrency, Context const &parentContext) {
    using namespace std;
    using namespace mcfile;
    namespace fs = std::filesystem;

    fs::path dir;
    switch (d) {
    case Dimension::Overworld:
      dir = root;
      break;
    case Dimension::Nether:
      dir = root / "DIM-1";
      break;
    case Dimension::End:
      dir = root / "DIM1";
      break;
    default:
      return nullptr;
    }

    error_code ec;
    fs::create_directories(dir / "region", ec);
    if (ec) {
      return nullptr;
    }
    fs::create_directories(dir / "entities", ec);
    if (ec) {
      return nullptr;
    }

    unordered_map<Pos2i, je2be::toje::Region, Pos2iHasher> regions;
    mcfile::be::Chunk::ForAll(&db, d, [&regions](int cx, int cz) {
      int rx = Coordinate::RegionFromChunk(cx);
      int rz = Coordinate::RegionFromChunk(cz);
      Pos2i c(cx, cz);
      Pos2i r(rx, rz);
      regions[r].fChunks.insert(c);
    });

    hwm::task_queue queue(concurrency);
    deque<future<shared_ptr<Context>>> futures;
    auto ctx = parentContext.make();

    bool ok = true;
    for (auto const &region : regions) {
      Pos2i r = region.first;
      vector<future<shared_ptr<Context>>> drain;
      FutureSupport::Drain(concurrency + 1, futures, drain);
      for (auto &d : drain) {
        auto result = d.get();
        if (result) {
          result->mergeInto(*ctx);
        } else {
          ok = false;
        }
      }
      if (!ok) {
        break;
      }
      futures.emplace_back(queue.enqueue(Region::Convert, d, region.second.fChunks, r.fX, r.fZ, &db, dir, *ctx));
    }

    for (auto &f : futures) {
      auto result = f.get();
      if (result) {
        result->mergeInto(*ctx);
      } else {
        ok = false;
      }
    }

    if (ok) {
      return ctx;
    } else {
      return nullptr;
    }
  }

private:
  World() = delete;
};

} // namespace je2be::toje
