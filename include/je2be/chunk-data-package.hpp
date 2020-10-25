#pragma once

namespace j2b {

class ChunkDataPackage {
public:
    void build(mcfile::Chunk const& chunk) {
        buildEntities(chunk);
        buildBiomeMap(chunk);
        buildTileEntities(chunk);
    }

    void serialize(ChunkData& cd) {
        serializeData2D(cd);
        serializeBlockEntity(cd);
        serializeEntity(cd);
    }

    void updateAltitude(int x, int y, int z) {
        fHeightMap.update(x, y, z);
    }

    void addTileBlock(int x, int y, int z, std::shared_ptr<mcfile::Block const> const& block) {
        fTileBlocks.insert(make_pair(Pos(x, y, z), block));
    }

private:
    void buildTileEntities(mcfile::Chunk const& chunk) {
        using namespace std;
        using namespace mcfile;
        for (auto const& it : fTileBlocks) {
            Pos pos = it.first;
            shared_ptr<Block const> const& block = it.second;
            auto tag = TileEntity::From(pos, *block, nullptr/*TODO*/);
            if (!tag) {
                continue;
            }
            fTileEntities.push_back(tag);
        }
    }

    void buildEntities(mcfile::Chunk const& chunk) {
        using namespace std;
        using namespace props;
        for (auto e : chunk.fEntities) {
            auto c = e->asCompound();
            if (!c) {
                continue;
            }
            auto e = Entity::From(*c);
            if (!e) {
                continue;
            }
            fEntities.push_back(e);
        }
    }

    void buildBiomeMap(mcfile::Chunk const& chunk) {
        int const y = 0;
        int const x0 = chunk.minBlockX();
        int const z0 = chunk.minBlockZ();
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                auto biome = chunk.biomeAt(x + x0, z + z0);
                fBiomeMap.set(x, z, biome);
            }
        }
    }

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

        if (fTileEntities.empty()) {
            return;
        }
        auto s = make_shared<ByteStream>();
        OutputStreamWriter w(s, {.fLittleEndian = true});
        for (auto const& tag : fTileEntities) {
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

private:
    HeightMap fHeightMap;
    BiomeMap fBiomeMap;
    std::unordered_map<Pos, std::shared_ptr<mcfile::Block const>, PosHasher> fTileBlocks;
    std::vector<std::shared_ptr<mcfile::nbt::CompoundTag>> fTileEntities;
    std::vector<std::shared_ptr<mcfile::nbt::CompoundTag>> fEntities;
};

}
