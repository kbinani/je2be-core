#pragma once

namespace j2b {

class Portals {
public:
  void add(PortalBlocks &pb, mcfile::Dimension dim) {
    if (dim == mcfile::Dimension::Overworld) {
      pb.drain(fOverworld);
    } else if (dim == mcfile::Dimension::Nether) {
      pb.drain(fNether);
    }
  }

  void putInto(DbInterface &db) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;
    using namespace mcfile::nbt;
    vector<Portal> portals;
    fOverworld.extract(portals, Dimension::Overworld);
    fNether.extract(portals, Dimension::Nether);

    auto root = make_shared<CompoundTag>();
    auto data = make_shared<CompoundTag>();
    auto portalRecords = make_shared<ListTag>(Tag::Type::Compound);
    for (auto const &portal : portals) {
      portalRecords->push_back(portal.toCompoundTag());
    }
    data->set("PortalRecords", portalRecords);
    root->set("data", data);

    auto s = make_shared<ByteStream>();
    OutputStreamWriter w(s, {.fLittleEndian = true});
    root->writeAsRoot(w);

    auto key = mcfile::be::DbKey::Portals();
    vector<uint8_t> buffer;
    s->drain(buffer);

    leveldb::Slice v((char *)buffer.data(), buffer.size());
    db.put(key, v);
  }

private:
  PortalBlocks fOverworld;
  PortalBlocks fNether;
};

} // namespace j2b
