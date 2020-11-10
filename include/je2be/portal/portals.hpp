#pragma once

namespace j2b {

class Portals {
public:
  void add(PortalBlocks &pb, Dimension dim) {
    if (dim == Dimension::Overworld) {
      pb.drain(fOverworld);
    } else if (dim == Dimension::Nether) {
      pb.drain(fNether);
    }
  }

  void putInto(DbInterface &db) {
    using namespace std;
    using namespace mcfile::stream;
    using namespace mcfile::nbt;
    vector<Portal> portals;
    fOverworld.extract(portals, Dimension::Overworld);
    fNether.extract(portals, Dimension::Nether);

    auto root = make_shared<CompoundTag>();
    auto data = make_shared<CompoundTag>();
    auto portalRecords = make_shared<ListTag>();
    portalRecords->fType = Tag::TAG_Compound;
    for (auto const &portal : portals) {
      portalRecords->push_back(portal.toCompoundTag());
    }
    data->set("PortalRecords", portalRecords);
    root->set("data", data);

    auto s = make_shared<ByteStream>();
    OutputStreamWriter w(s, {.fLittleEndian = true});
    w.write((uint8_t)Tag::TAG_Compound);
    w.write(string());
    root->write(w);
    w.write((uint8_t)Tag::TAG_End);

    auto key = Key::Portals();
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
