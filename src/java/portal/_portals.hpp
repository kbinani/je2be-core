#pragma once

#include "db/_db-interface.hpp"
#include "java/portal/_portal-blocks.hpp"
#include "java/portal/_portal.hpp"

namespace je2be::java {

class Portals {
public:
  void add(PortalBlocks &pb, mcfile::Dimension dim) {
    if (dim == mcfile::Dimension::Overworld) {
      pb.drain(fOverworld);
    } else if (dim == mcfile::Dimension::Nether) {
      pb.drain(fNether);
    }
  }

  [[nodiscard]] Status putInto(DbInterface &db) {
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
    data->set(u8"PortalRecords", portalRecords);
    root->set(u8"data", data);

    auto buffer = CompoundTag::Write(*root, Endian::Little);
    if (!buffer) {
      return JE2BE_ERROR;
    }

    auto key = mcfile::be::DbKey::Portals();
    leveldb::Slice value(*buffer);

    return db.put(key, value);
  }

private:
  PortalBlocks fOverworld;
  PortalBlocks fNether;
};

} // namespace je2be::java
