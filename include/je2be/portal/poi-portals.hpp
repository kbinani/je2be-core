#pragma once

namespace je2be {

class PoiPortals {
public:
  void add(Pos3i const &pos) {
    int rx = mcfile::Coordinate::RegionFromBlock(pos.fX);
    int rz = mcfile::Coordinate::RegionFromBlock(pos.fZ);
    fRegions.insert(Pos2i(rx, rz));
    fPortals.insert(pos);
  }

  void mergeInto(PoiPortals &other) const {
    for (auto const &r : fRegions) {
      other.fRegions.insert(r);
    }
    for (auto const &p : fPortals) {
      other.fPortals.insert(p);
    }
  }

  bool write(std::filesystem::path const &path, int dataVersion) const {
    using namespace std;
    using namespace mcfile::je;
    using namespace mcfile::stream;
    using Region = mcfile::je::Region;
    namespace fs = std::filesystem;
    if (fRegions.empty()) {
      return true;
    }

    if (!Fs::CreateDirectories(path)) {
      return false;
    }

    for (Pos2i const &r : fRegions) {
      auto name = Region::GetDefaultRegionFileName(r.fX, r.fZ);
      auto stream = make_shared<FileOutputStream>(path / name);
      bool ok = Region::SquashChunksAsMca(*stream, [r, dataVersion, this](int localChunkX, int localChunkZ, bool &) -> CompoundTagPtr {
        int cx = r.fX * 32 + localChunkX;
        int cz = r.fZ * 32 + localChunkZ;
        unordered_map<int32_t, vector<CompoundTagPtr>> sections;
        for (auto const &pos : fPortals) {
          int x = mcfile::Coordinate::ChunkFromBlock(pos.fX);
          int z = mcfile::Coordinate::ChunkFromBlock(pos.fZ);
          if (x == cx && z == cz) {
            auto record = Compound();
            record->set("free_tickets", Int(0));
            record->set("type", String("minecraft:nether_portal"));
            record->set("pos", IntArrayFromPos3i(pos));
            int32_t y = mcfile::Coordinate::ChunkFromBlock(pos.fY);
            sections[y].push_back(record);
          }
        }
        if (sections.empty()) {
          return nullptr;
        }
        auto sectionsTag = Compound();
        for (auto const &it : sections) {
          int32_t y = it.first;
          vector<CompoundTagPtr> const &records = it.second;
          auto recordsTag = List<Tag::Type::Compound>();
          for (auto const &record : records) {
            recordsTag->push_back(record);
          }
          auto sectionTag = Compound();
          sectionTag->set("Records", recordsTag);
          sectionTag->set("Valid", Bool(true));
          sectionsTag->set(to_string(y), sectionTag);
        }
        auto chunk = Compound();
        chunk->set("Sections", sectionsTag);
        chunk->set("DataVersion", Int(dataVersion));
        return chunk;
      });
      if (!ok) {
        return false;
      }
    }

    return true;
  }

private:
  std::unordered_set<Pos2i, Pos2iHasher> fRegions;
  std::unordered_set<Pos3i, Pos3iHasher> fPortals;
};

} // namespace je2be
