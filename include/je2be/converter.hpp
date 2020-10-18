#pragma once

namespace j2b {

class Converter {
public:
    class InputOption {
    public:
        LevelDirectoryStructure fLevelDirectoryStructure = LevelDirectoryStructure::Vanilla;

        std::string getWorldDirectory(std::string const& root, Dimension dim) const {
            switch (fLevelDirectoryStructure) {
            case LevelDirectoryStructure::Vanilla: {
                switch (dim) {
                case Dimension::Overworld:
                    return root;
                case Dimension::Nether:
                    return root + "/DIM-1";
                case Dimension::End:
                    return root + "/DIM1";
                }
                break;
            }
            case LevelDirectoryStructure::Paper: {
                switch (dim) {
                case Dimension::Overworld:
                    return root + "/world";
                case Dimension::Nether:
                    return root + "/world_nether/DIM-1";
                case Dimension::End:
                    return root + "/world_the_end/DIM1";
                }
                break;
            }
            }
        }
    };

    class OutputOption {
    public:
    };

    Converter(std::string const& input, InputOption io, std::string const& output, OutputOption oo)
        : fInput(input)
        , fOutput(output)
        , fInputOption(io)
        , fOutputOption(oo)
    {}

    bool run(unsigned int concurrency) {
        using namespace std;
        namespace fs = mcfile::detail::filesystem;
        using namespace mcfile;

        auto rootPath = fs::path(fOutput);
        auto dbPath = rootPath / "db";

        fs::create_directory(rootPath);
        fs::create_directory(dbPath);
        LevelData levelData;
        if (true) {
            //TODO(kbinani): for debug
            levelData.fLevelName = "je2be-output";
            levelData.fShowCoordinates = true;
            levelData.fCommandsEnabled = true;
        }
        levelData.write(fOutput + string("/level.dat"));

        Db db(dbPath.string());
        if (!db.valid()) {
            return false;
        }

        bool ok = true;
        //for (auto dim : { Dimension::Overworld, Dimension::Nether, Dimension::End }) {
        for (auto dim : { Dimension::Overworld }) { //TODO(kbinani): debug
                auto dir = fInputOption.getWorldDirectory(fInput, dim);
            World world(dir);
            ok &= convertWorld(world, dim, db);
        }

        return ok;
    }

private:
    bool convertWorld(mcfile::World const& w, Dimension dim, Db &db) {
        using namespace std;
        using namespace mcfile;

        w.eachRegions([this, dim, &db](shared_ptr<Region> const& region) {
            if (region->fX != 0 || region->fZ != 0) { //TODO(kbinani): debug
                return true;
            }
            bool err;
            region->loadAllChunks(err, [this, dim, &db](Chunk const& chunk) {
                if (chunk.fChunkX != 0 || chunk.fChunkZ != 0) { //TODO(kbinani): debug
                    return true;
                }
                putChunk(chunk, dim, db);
                return true;
            });
            return true;
        });

        return false;
    }

    void putChunk(mcfile::Chunk const& chunk, Dimension dim, Db& db) {
        for (int chunkY = 0; chunkY < 16; chunkY++) {
            putSubChunk(chunk, dim, chunkY, db);
        }
        auto const& versionKey = Keys::Version(chunk.fChunkX, chunk.fChunkZ, dim);
        leveldb::Slice version(&kSubChunkVersion, 1);
        db.put(versionKey, version);
    }

    void putSubChunk(mcfile::Chunk const& chunk, Dimension dim, int chunkY, Db &db) {
        using namespace std;
        using namespace mcfile;
        using namespace mcfile::nbt;
        using namespace props;
        using namespace leveldb;

        if (chunk.fSections[chunkY] == nullptr) {
            return;
        }

        size_t const kNumBlocksInSubChunk = 16 * 16 * 16;

        vector<uint16_t> indices(kNumBlocksInSubChunk);
        // subchunk format
        // 0: version (0x8)
        // 1: num storage blocks
        // 2~: storage blocks repeated

        // storage block format
        // 0: bbbbbbbv (bit, b = bits per block, v = version(0x0))
        // 1-
        // 1~4: paltte size (uint32 LE)
        //

        vector<string> paletteKeys;
        vector<shared_ptr<CompoundTag>> palette;

        int idx = 0;
        bool empty = true;
        for (int x = 0; x < 16; x++) {
            int const bx = chunk.minBlockX() + x;
            for (int z = 0; z < 16; z++) {
                int const bz = chunk.minBlockZ() + z;
                for (int y = 0; y < 16; y++, idx++) {
                    int const by = chunkY * 16 + y;
                    auto block = chunk.blockAt(bx, by, bz);
                    if (block) {
                        empty = false;
                    }
                    string paletteKey = block ? block->toString() : "minecraft:air"s;
                    auto found = find(paletteKeys.begin(), paletteKeys.end(), paletteKey);
                    if (found == paletteKeys.end()) {
                        uint16_t index = (uint16_t)paletteKeys.size();
                        indices[idx] = index;

                        auto tag = block ? BlockData::From(block) : BlockData::Air();
                        //TODO: when waterlogged blocks exist, second storage block should be added
                        palette.push_back(tag);
                        paletteKeys.push_back(paletteKey);
                    } else {
                        auto index = (uint16_t)distance(paletteKeys.begin(), found);
                        indices[idx] = index;
                    }
                }
            }
        }
        if (empty) {
            return;
        }

        int const numPaletteEntries = (int)palette.size();
        uint8_t bitsPerBlock;
        int blocksPerWord;
        if (numPaletteEntries <= 2) {
            bitsPerBlock = 1;
            blocksPerWord = 32;
        } else if (numPaletteEntries <= 4) {
            bitsPerBlock = 2;
            blocksPerWord = 16;
        } else if (numPaletteEntries <= 8) {
            bitsPerBlock = 3;
            blocksPerWord = 10;
        } else if (numPaletteEntries <= 16) {
            bitsPerBlock = 4;
            blocksPerWord = 8;
        } else if (numPaletteEntries <= 32) {
            bitsPerBlock = 5;
            blocksPerWord = 6;
        } else if (numPaletteEntries <= 64) {
            bitsPerBlock = 6;
            blocksPerWord = 5;
        } else if (numPaletteEntries <= 256) {
            bitsPerBlock = 8;
            blocksPerWord = 4;
        } else {
            bitsPerBlock = 16;
            blocksPerWord = 2;
        }

        auto stream = make_shared<mcfile::stream::ByteStream>();
        mcfile::stream::OutputStreamWriter w(stream, { .fLittleEndian = true });

        w.write(kBlockStorageVersion);
        w.write((uint8_t)1); // num storage blocks
        w.write((uint8_t)(bitsPerBlock * 2));

        uint32_t const mask = ~((~((uint32_t)0)) << bitsPerBlock);
        for (size_t i = 0; i < indices.size(); i += blocksPerWord) {
            uint32_t v = 0;
            for (size_t j = 0; j < blocksPerWord && i + j < kNumBlocksInSubChunk; j++) {
                uint32_t const index = (uint32_t)indices[i + j];
                v = v | ((mask & index) << (j * bitsPerBlock));
            }
            w.write(v);
        }

        w.write((uint32_t)numPaletteEntries);

        for (int i = 0; i < palette.size(); i++) {
            shared_ptr<CompoundTag> const& tag = palette[i];
            w.write((uint8_t)Tag::TAG_Compound);
            w.write(std::string());
            tag->write(w);
        }

        vector<uint8_t> data;
        stream->drain(data);
        Slice subChunk((char *)data.data(), data.size());
        auto const& subChunkKey = Keys::SubChunk(chunk.fChunkX, chunkY, chunk.fChunkZ, dim);
        db.put(subChunkKey, subChunk);
    }

private:
    std::string const fInput;
    std::string const fOutput;
    InputOption const fInputOption;
    OutputOption const fOutputOption;

private:
    uint8_t const kBlockStorageVersion = 8;
    char const kSubChunkVersion = 19;
};

}
