#pragma once

namespace je2be::toje {

class Poi {
  Poi() = delete;

  class Portals {
  public:
    void add(Portal const &p) {
      fPortals.push_back(p);
      Pos3i pos(p.fTpX, p.fTpY, p.fTpZ);
      Pos3i direction(p.fXa ? 1 : 0, 0, p.fZa ? 1 : 0);
      for (int i = 0; i < p.fSpan; i++) {
        int rx = mcfile::Coordinate::RegionFromBlock(pos.fX);
        int rz = mcfile::Coordinate::RegionFromBlock(pos.fZ);
        fRegions.insert(Pos2i(rx, rz));
        pos = pos + direction;
      }
    }

    bool write(std::filesystem::path const &path) const {
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
        bool ok = Region::SquashChunksAsMca(*stream, [r, this](int localChunkX, int localChunkZ, bool &) -> CompoundTagPtr {
          int cx = r.fX * 32 + localChunkX;
          int cz = r.fZ * 32 + localChunkZ;
          unordered_map<int32_t, vector<CompoundTagPtr>> sections;
          for (auto const &p : fPortals) {
            Pos3i pos(p.fTpX, p.fTpY, p.fTpZ);
            Pos3i direction(p.fXa ? 1 : 0, 0, p.fZa ? 1 : 0);
            for (int i = 0; i < p.fSpan; i++) {
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
              pos = pos + direction;
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
          chunk->set("DataVersion", Int(kDataVersion));
          return chunk;
        });
        if (!ok) {
          return false;
        }
      }

      return true;
    }

  public:
    std::unordered_set<Pos2i, Pos2iHasher> fRegions;

  private:
    std::vector<Portal> fPortals;
  };

public:
  static bool Export(leveldb::DB &db, std::filesystem::path const &rootDirectory) {
    using namespace std;
    string portalsValue;
    if (auto st = db.Get({}, mcfile::be::DbKey::Portals(), &portalsValue); !st.ok()) {
      return true;
    }
    auto portalsTag = CompoundTag::Read(portalsValue, mcfile::Endian::Little);
    if (!portalsTag) {
      return true;
    }
    auto data = portalsTag->compoundTag("data");
    if (!data) {
      return true;
    }
    auto records = data->listTag("PortalRecords");
    if (!records) {
      return true;
    }
    Portals overworldPortals;
    Portals netherPortals;
    for (auto const &record : *records) {
      auto compound = record->asCompound();
      if (!compound) {
        continue;
      }
      if (auto portal = Portal::FromCompound(*compound); portal) {
        if (portal->fDimId == static_cast<uint8_t>(mcfile::Dimension::Overworld)) {
          overworldPortals.add(*portal);
        } else if (portal->fDimId == static_cast<uint8_t>(mcfile::Dimension::Nether)) {
          netherPortals.add(*portal);
        }
      }
    }

    return overworldPortals.write(rootDirectory / "poi") && netherPortals.write(rootDirectory / "DIM-1" / "poi");
  }

private:
};

} // namespace je2be::toje
