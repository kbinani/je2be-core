#pragma once

namespace je2be::toje {

class World {
public:
  static bool Convert(mcfile::Dimension d, leveldb::DB &db, std::filesystem::path root, unsigned concurrency) {
    using namespace std;
    using namespace mcfile;
    namespace fs = std::filesystem;

    fs::path dir;
    switch (d) {
    case Dimension::Overworld:
      dir = root / "region";
      break;
    case Dimension::Nether:
      dir = root / "DIM-1" / "region";
      break;
    case Dimension::End:
      dir = root / "DIM1" / "region";
      break;
    default:
      return false;
    }

    error_code ec;
    fs::create_directories(dir, ec);
    if (ec) {
      return false;
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
    deque<future<bool>> futures;

    bool ok = true;
    for (auto const &region : regions) {
      Pos2i r = region.first;
      vector<future<bool>> drain;
      FutureSupport::Drain(concurrency + 1, futures, drain);
      for (auto &d : drain) {
        if (!d.get()) {
          ok = false;
        }
      }
      if (!ok) {
        break;
      }
      futures.emplace_back(queue.enqueue(Region::Convert, d, region.second.fChunks, r.fX, r.fZ, &db, dir));
    }

    for (auto &f : futures) {
      if (!f.get()) {
        ok = false;
      }
    }

    return ok;
  }

private:
  World() = delete;
};

} // namespace je2be::toje
