#pragma once

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

    auto s = make_shared<ByteStream>();
    OutputStreamWriter w(s, Endian::Little);
    if (!root->writeAsRoot(w)) {
      return false;
    }

    auto key = mcfile::be::DbKey::Portals();
    vector<uint8_t> buffer;
    s->drain(buffer);

    leveldb::Slice v((char *)buffer.data(), buffer.size());
    db.put(key, v);
    return true;
  }

private:
  PortalBlocks fOverworld;
  PortalBlocks fNether;
};

} // namespace je2be::tobe
