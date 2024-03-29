#pragma once

#include <je2be/fs.hpp>
#include <je2be/pos2.hpp>

#include "_pos2i-set.hpp"
#include "_pos3.hpp"

#include <minecraft-file.hpp>

namespace je2be {

class PoiBlocks {
  struct Data {
    i32 fFreeTickets;
    std::u8string fType;

    Data() = default;
    Data(i32 freeTickets, std::u8string const &type) : fFreeTickets(freeTickets), fType(type) {}
  };

  static std::unordered_map<mcfile::blocks::BlockId, Data> const &GetTable() {
    static std::unique_ptr<std::unordered_map<mcfile::blocks::BlockId, Data> const> const sTable(CreateTable());
    return *sTable;
  }

  static std::unordered_map<mcfile::blocks::BlockId, Data> *CreateTable() {
    using namespace std;
    using namespace mcfile::blocks::minecraft;
    auto ret = new unordered_map<mcfile::blocks::BlockId, Data>();
    auto &t = *ret;
    t[nether_portal] = {0, u8"minecraft:nether_portal"};
    t[bell] = {32, u8"minecraft:meeting"};
    t[cartography_table] = {1, u8"minecraft:cartographer"};
    t[stonecutter] = {1, u8"minecraft:mason"};
    t[blast_furnace] = {1, u8"minecraft:armorer"};
    t[white_bed] = {1, u8"minecraft:home"};
    t[orange_bed] = {1, u8"minecraft:home"};
    t[magenta_bed] = {1, u8"minecraft:home"};
    t[light_blue_bed] = {1, u8"minecraft:home"};
    t[yellow_bed] = {1, u8"minecraft:home"};
    t[lime_bed] = {1, u8"minecraft:home"};
    t[pink_bed] = {1, u8"minecraft:home"};
    t[gray_bed] = {1, u8"minecraft:home"};
    t[light_gray_bed] = {1, u8"minecraft:home"};
    t[cyan_bed] = {1, u8"minecraft:home"};
    t[purple_bed] = {1, u8"minecraft:home"};
    t[blue_bed] = {1, u8"minecraft:home"};
    t[brown_bed] = {1, u8"minecraft:home"};
    t[green_bed] = {1, u8"minecraft:home"};
    t[red_bed] = {1, u8"minecraft:home"};
    t[black_bed] = {1, u8"minecraft:home"};
    t[lectern] = {1, u8"minecraft:librarian"};
    t[fletching_table] = {1, u8"minecraft:fletcher"};
    t[composter] = {1, u8"minecraft:farmer"};
    t[loom] = {1, u8"minecraft:shepherd"};
    t[grindstone] = {1, u8"minecraft:weaponsmith"};
    t[barrel] = {1, u8"minecraft:fisherman"};
    t[smithing_table] = {1, u8"minecraft:toolsmith"};
    t[smoker] = {1, u8"minecraft:butcher"};
    t[brewing_stand] = {1, u8"minecraft:cleric"};
    t[cauldron] = {1, u8"minecraft:leatherworker"};
    t[water_cauldron] = {1, u8"minecraft:leatherworker"};
    t[lava_cauldron] = {1, u8"minecraft:leatherworker"};
    t[powder_snow_cauldron] = {1, u8"minecraft:leatherworker"};
    t[beehive] = {0, u8"minecraft:beehive"};
    t[bee_nest] = {0, u8"minecraft:bee_nest"};
    t[lightning_rod] = {0, u8"minecraft:lightning_rod"};
    t[lodestone] = {0, u8"minecraft:lodestone"};
    return ret;
  }

public:
  static bool Interest(mcfile::je::Block const &block) {
    using namespace mcfile::blocks::minecraft;
    switch (block.fId) {
    case nether_portal:
    case loom:
    case composter:
    case barrel:
    case smoker:
    case blast_furnace:
    case cartography_table:
    case fletching_table:
    case grindstone:
    case smithing_table:
    case stonecutter:
    case bell:
    case lectern:
    case brewing_stand:
    case cauldron:
    case water_cauldron:
    case lava_cauldron:
    case powder_snow_cauldron:
    case beehive:
    case bee_nest:
    case lightning_rod:
    case lodestone:
      return true;
    case white_bed:
    case orange_bed:
    case magenta_bed:
    case light_blue_bed:
    case yellow_bed:
    case lime_bed:
    case pink_bed:
    case gray_bed:
    case light_gray_bed:
    case cyan_bed:
    case purple_bed:
    case blue_bed:
    case brown_bed:
    case green_bed:
    case red_bed:
    case black_bed:
      return block.property(u8"part") == u8"head";
    default:
      return false;
    }
  }

  void add(Pos3i const &pos, mcfile::blocks::BlockId id) {
    int rx = mcfile::Coordinate::RegionFromBlock(pos.fX);
    int rz = mcfile::Coordinate::RegionFromBlock(pos.fZ);
    fRegions.insert(Pos2i(rx, rz));
    fBlocks[pos] = id;
  }

  void mergeInto(PoiBlocks &other) const {
    for (auto const &r : fRegions) {
      other.fRegions.insert(r);
    }
    for (auto const &p : fBlocks) {
      other.fBlocks.insert(p);
    }
  }

  bool write(std::filesystem::path const &path, int dataVersion) const {
    using namespace std;
    using namespace mcfile::je;
    using namespace mcfile::stream;
    using namespace mcfile::blocks;
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
        unordered_map<i32, vector<CompoundTagPtr>> sections;
        auto const &table = GetTable();
        for (auto const &it : fBlocks) {
          Pos3i const &pos = it.first;
          BlockId id = it.second;
          int x = mcfile::Coordinate::ChunkFromBlock(pos.fX);
          int z = mcfile::Coordinate::ChunkFromBlock(pos.fZ);
          if (x != cx || z != cz) {
            continue;
          }
          if (auto found = table.find(id); found != table.end()) {
            Data const &data = found->second;
            auto record = Compound();
            record->set(u8"free_tickets", Int(data.fFreeTickets));
            record->set(u8"type", data.fType);
            record->set(u8"pos", IntArrayFromPos3i(pos));

            i32 y = mcfile::Coordinate::ChunkFromBlock(pos.fY);
            sections[y].push_back(record);
          }
        }
        if (sections.empty()) {
          return nullptr;
        }
        auto sectionsTag = Compound();
        for (auto const &it : sections) {
          i32 y = it.first;
          vector<CompoundTagPtr> const &records = it.second;
          auto recordsTag = List<Tag::Type::Compound>();
          for (auto const &record : records) {
            recordsTag->push_back(record);
          }
          auto sectionTag = Compound();
          sectionTag->set(u8"Records", recordsTag);
          sectionTag->set(u8"Valid", Bool(true));
          sectionsTag->set(mcfile::String::ToString(y), sectionTag);
        }
        auto chunk = Compound();
        chunk->set(u8"Sections", sectionsTag);
        chunk->set(u8"DataVersion", Int(dataVersion));
        return chunk;
      });
      if (!ok) {
        return false;
      }
    }

    return true;
  }

private:
  Pos2iSet fRegions;
  std::unordered_map<Pos3i, mcfile::blocks::BlockId, Pos3iHasher> fBlocks;
};

} // namespace je2be
