#pragma once

namespace j2b {

class ChunkDataPackage {
public:
    void serialize(ChunkData& cd) {
        serializeData2D(cd);
        serializeBlockEntity(cd);
        serializeEntity(cd);
    }

private:
    void serializeData2D(ChunkData& cd) {
        using namespace std;
        using namespace mcfile::stream;
        auto s = make_shared<ByteStream>();
        OutputStreamWriter w(s, {.fLittleEndian = true});
        fHeightMap.write(w);
        fBiomeMap.write(w);
        s->drain(cd.fData2D);
    }

    void serializeBlockEntity(ChunkData &cd) {
        using namespace std;
        using namespace mcfile::stream;
        using namespace mcfile::nbt;

        if (fContainerBlocks.empty()) {
            return;
        }
        auto s = make_shared<ByteStream>();
        OutputStreamWriter w(s, {.fLittleEndian = true});
        for (auto const& it : fContainerBlocks) {
            auto tag = it->toCompoundTag();
            w.write((uint8_t)Tag::TAG_Compound);
            w.write(string());
            tag->write(w);
            w.write((uint8_t)Tag::TAG_End);
        }
        s->drain(cd.fBlockEntity);
    }

    void serializeEntity(ChunkData& cd) {
        using namespace std;
        using namespace mcfile::stream;
        using namespace mcfile::nbt;

        if (fEntities.empty()) {
            return;
        }
        auto s = make_shared<ByteStream>();
        OutputStreamWriter w(s, {.fLittleEndian = true});
        for (auto const& tag : fEntities) {
            w.write((uint8_t)Tag::TAG_Compound);
            w.write(string());
            tag->write(w);
            w.write((uint8_t)Tag::TAG_End);
        }
        s->drain(cd.fEntity);
    }

public:
    HeightMap fHeightMap;
    BiomeMap fBiomeMap;
    std::vector<std::shared_ptr<entities::Chest>> fContainerBlocks; //TODO(kbinani): unordered_map<Pos, Container>
    std::vector<std::shared_ptr<mcfile::nbt::CompoundTag>> fEntities;
};

}
