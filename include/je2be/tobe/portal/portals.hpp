#pragma once

#include <je2be/db/db-interface.hpp>
#include <je2be/tobe/portal/portal-blocks.hpp>
#include <je2be/tobe/portal/portal.hpp>

namespace je2be::tobe {

class Portals {
public:
  void add(PortalBlocks &pb, mcfile::Dimension dim) {
    if (dim == mcfile::Dimension::Overworld) {
      pb.drain(fOverworld);
    } else if (dim == mcfile::Dimension::Nether) {
      pb.drain(fNether);
    }
  }

  [[nodiscard]] bool putInto(DbInterface &db) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;
    vector<Portal> portals;
    fOverworld.extract(portals, Dimension::Overworld);
    fNether.extract(portals, Dimension::Nether);

    auto root = Compound();
    auto data = Compound();
    auto portalRecords = List<Tag::Type::Compound>();
    for (auto const &portal : portals) {
      portalRecords->push_back(portal.toCompoundTag());
    }
    data->set("PortalRecords", portalRecords);
    root->set("data", data);

    auto buffer = CompoundTag::Write(*root, Endian::Little);
    if (!buffer) {
      return false;
    }

    auto key = mcfile::be::DbKey::Portals();
    leveldb::Slice value(*buffer);

    db.put(key, value);
    return true;
  }

private:
  PortalBlocks fOverworld;
  PortalBlocks fNether;
};

} // namespace je2be::tobe
