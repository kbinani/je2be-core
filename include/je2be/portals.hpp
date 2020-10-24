#pragma once

namespace j2b {

class Portals {
public:
    Portals()
        : fOverworldX(Dimension::Overworld, true)
        , fOverworldZ(Dimension::Overworld, false)
        , fNetherX(Dimension::Nether, true)
        , fNetherZ(Dimension::Nether, false)
    {}

    void add(int32_t x, int32_t y, int32_t z, mcfile::Block const& block, Dimension dim) {
        auto axis = block.property("axis", "x");
        if (dim == Dimension::Overworld) {
            if (axis == "x") {
                fOverworldX.add(x, y, z);
            } else {
                fOverworldZ.add(x, y, z);
            }
        } else if (dim == Dimension::Nether) {
            if (axis == "x") {
                fNetherX.add(x, y, z);
            } else {
                fNetherZ.add(x, y, z);
            }
        }
    }

    void putInto(Db& db) {
        using namespace std;
        using namespace mcfile::stream;
        using namespace mcfile::nbt;
        vector<Portal> portals;
        fOverworldX.drain(portals);
        fOverworldZ.drain(portals);
        fNetherX.drain(portals);
        fNetherZ.drain(portals);

        auto root = make_shared<CompoundTag>();
        auto data = make_shared<CompoundTag>();
        auto portalRecords = make_shared<ListTag>();
        portalRecords->fType = Tag::TAG_Compound;
        for (auto portal : portals) {
            portalRecords->fValue.push_back(portal.toCompoundTag());
        }
        data->fValue.emplace("PortalRecords", portalRecords);
        root->fValue.emplace("data", data);

        auto s = make_shared<ByteStream>();
        OutputStreamWriter w(s, {.fLittleEndian = true});
        w.write((uint8_t)Tag::TAG_Compound);
        w.write(string());
        root->write(w);
        w.write((uint8_t)Tag::TAG_End);

        auto key = Key::Portals();
        vector<uint8_t> buffer;
        s->drain(buffer);

        leveldb::Slice v((char*)buffer.data(), buffer.size());
        db.put(key, v);
    }

private:
    PortalBlocks fOverworldX;
    PortalBlocks fOverworldZ;
    PortalBlocks fNetherX;
    PortalBlocks fNetherZ;
};

}
