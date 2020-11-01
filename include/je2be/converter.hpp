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

        bool ok = true;
        {
            RawDb db(dbPath.string(), concurrency);
            if (!db.valid()) {
                return false;
            }

            auto worldData = std::make_unique<WorldData>(fs::path(fInput));
            for (auto dim : { Dimension::Overworld, Dimension::Nether, Dimension::End }) {
                auto dir = fInputOption.getWorldDirectory(fInput, dim);
                World world(dir);
                ok &= convertWorld(world, dim, db, *worldData, concurrency);
            }

            worldData->put(db);
        }

        return ok;
    }

private:
    bool convertWorld(mcfile::World const& w, Dimension dim, DbInterface &db, WorldData &wd, unsigned int concurrency) {
        using namespace std;
        using namespace mcfile;

        ThreadPool pool(concurrency);
        pool.init();
        deque<future<shared_ptr<DimensionDataFragment>>> futures;

        w.eachRegions([this, dim, &db, &pool, &futures, concurrency, &wd](shared_ptr<Region> const& region) {
            JavaEditionMap const& mapInfo = wd.fJavaEditionMap;
            for (int cx = region->minChunkX(); cx <= region->maxChunkX(); cx++) {
                for (int cz = region->minChunkZ(); cz <= region->maxChunkZ(); cz++) {
                    if (futures.size() > 10 * size_t(concurrency)) {
                        for (unsigned int i = 0; i < 5 * concurrency; i++) {
                            auto const& pcr = futures.front().get();
                            futures.pop_front();
                            if (!pcr) continue;
                            pcr->drain(wd);
                        }
                    }

                    futures.push_back(move(pool.submit([this, dim, &db, region, cx, cz, mapInfo]() -> shared_ptr<DimensionDataFragment> {
                        auto const& chunk = region->chunkAt(cx, cz);
                        if (!chunk) {
                            return nullptr;
                        }
                        return putChunk(*chunk, dim, db, mapInfo);
                    })));
                }
            }
            return true;
        });

        for (auto& f : futures) {
            auto const& pcr = f.get();
            if (!pcr) continue;
            pcr->drain(wd);
        }

        pool.shutdown();

        return true;
    }

    std::shared_ptr<DimensionDataFragment> putChunk(mcfile::Chunk const& chunk, Dimension dim, DbInterface& db, JavaEditionMap const& mapInfo) {
        using namespace std;
        using namespace mcfile;
        using namespace mcfile::stream;
        using namespace mcfile::nbt;

        auto ret = make_shared<DimensionDataFragment>(dim);

        ChunkDataPackage cdp;
        ChunkData cd(chunk.fChunkX, chunk.fChunkZ, dim);

        for (int chunkY = 0; chunkY < 16; chunkY++) {
            putSubChunk(chunk, dim, chunkY, cd, cdp, *ret);
        }

        cdp.build(chunk, mapInfo, *ret);
        cdp.serialize(cd);

        cd.put(db);
        fNumChunks.fetch_add(1);

        return ret;
    }

    void putSubChunk(mcfile::Chunk const& chunk, Dimension dim, int chunkY, ChunkData &cd, ChunkDataPackage &cdp, DimensionDataFragment &ddf) {
        using namespace std;
        using namespace mcfile;
        using namespace mcfile::nbt;
        using namespace props;
        using namespace leveldb;

        size_t const kNumBlocksInSubChunk = 16 * 16 * 16;

        vector<uint16_t> indices(kNumBlocksInSubChunk);
        vector<bool> waterloggedIndices(kNumBlocksInSubChunk);

        // subchunk format
        // 0: version (0x8)
        // 1: num storage blocks
        // 2~: storage blocks repeated

        // storage block format
        // 0: bbbbbbbv (bit, b = bits per block, v = version(0x0))
        // 1-
        // 1~4: paltte size (uint32 LE)
        //

        vector<string> paletteKeys = {"minecraft:air"};
        vector<shared_ptr<CompoundTag>> palette = {BlockData::Air()};

        int idx = 0;
        bool empty = true;
        bool hasWaterlogged = false;

        if (chunk.fSections[chunkY] != nullptr) {
            for (int x = 0; x < 16; x++) {
                int const bx = chunk.minBlockX() + x;
                for (int z = 0; z < 16; z++) {
                    int const bz = chunk.minBlockZ() + z;
                    for (int y = 0; y < 16; y++, idx++) {
                        int const by = chunkY * 16 + y;
                        auto block = chunk.blockAt(bx, by, bz);
                        if (!block) {
                            continue;
                        }
                        empty = false;
                        if (!IsAir(block->fName)) {
                            cdp.updateAltitude(x, by, z);
                        }
                        static string const nether_portal("minecraft:nether_portal");
                        if (TileEntity::IsTileEntity(block->fName)) {
                            cdp.addTileBlock(bx, by, bz, block);
                        } else if (strings::Equals(block->fName, nether_portal)) {
                            bool xAxis = block->property("axis", "x") == "x";
                            ddf.addPortalBlock(bx, by, bz, xAxis);
                        }
                        string const& paletteKey = block->toString();
                        auto found = find(paletteKeys.begin(), paletteKeys.end(), paletteKey);
                        if (found == paletteKeys.end()) {
                            uint16_t index = (uint16_t)paletteKeys.size();
                            indices[idx] = index;

                            auto tag = block ? BlockData::From(block) : BlockData::Air();
                            palette.push_back(tag);
                            paletteKeys.push_back(paletteKey);
                        } else {
                            auto index = (uint16_t)distance(paletteKeys.begin(), found);
                            indices[idx] = index;
                        }
                        bool waterlogged = block ? IsWaterLogged(*block) : false;
                        waterloggedIndices[idx] = waterlogged;
                        hasWaterlogged |= waterlogged;
                    }
                }
            }
        }

        for (auto const& e : chunk.fEntities) {
            if (!Entity::IsTileEntity(*e)) {
                continue;
            }
            auto [pos, tag, paletteKey] = Entity::ToTileEntityBlock(*e);
            if (pos.fY < chunkY * 16 || chunkY * 16 + 16 <= pos.fY) {
                continue;
            }

            int32_t x = pos.fX - chunk.fChunkX * 16;
            int32_t y = pos.fY - chunkY * 16;
            int32_t z = pos.fZ - chunk.fChunkZ * 16;
            if (x <0 || 16 <= x || y < 0 || 16 <= y || z < 0 || 16 <= z) continue;
            idx = (x * 16 + z) * 16 + y;

            string paletteKey = paletteKeys[indices[idx]];
            if (!IsAir(paletteKey)) continue;

            empty = false;

            auto found = find(paletteKeys.begin(), paletteKeys.end(), paletteKey);
            if (found == paletteKeys.end()) {
                uint16_t index = (uint16_t)palette.size();
                indices[idx] = index;
                palette.push_back(tag);
                paletteKeys.push_back(paletteKey);
            } else {
                auto index = (uint16_t)distance(paletteKeys.begin(), found);
                indices[idx] = index;
            }
        }

        if (empty) {
            return;
        }

        int const numStorageBlocks = hasWaterlogged ? 2 : 1;

        auto stream = make_shared<mcfile::stream::ByteStream>();
        mcfile::stream::OutputStreamWriter w(stream, { .fLittleEndian = true });

        w.write(kBlockStorageVersion);
        w.write((uint8_t)numStorageBlocks);
        
        {
            // layer 0
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

            w.write((uint8_t)(bitsPerBlock * 2));
            uint32_t const mask = ~((~((uint32_t)0)) << bitsPerBlock);
            for (size_t i = 0; i < kNumBlocksInSubChunk; i += blocksPerWord) {
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
        }

        if (hasWaterlogged) {
            // layer 1
            //TODO(kbinani): flowing_water
            int const numPaletteEntries = 2; // air or water
            uint8_t bitsPerBlock = 1;
            int blocksPerWord = 32;
            
            uint32_t const paletteAir = 0;
            uint32_t const paletteWater = 1;
            
            w.write((uint8_t)(bitsPerBlock * 2));
            for (size_t i = 0; i < kNumBlocksInSubChunk; i += blocksPerWord) {
                uint32_t v = 0;
                for (size_t j = 0; j < blocksPerWord && i + j < kNumBlocksInSubChunk; j++) {
                    bool waterlogged = waterloggedIndices[i + j];
                    uint32_t const index = waterlogged ? paletteWater : paletteAir;
                    v = v | (index << (j * bitsPerBlock));
                }
                w.write(v);
            }
            
            w.write((uint32_t)numPaletteEntries);
            
            auto air = BlockData::Air();
            w.write((uint8_t)Tag::TAG_Compound);
            w.write(string());
            air->write(w);
            
            auto water = BlockData::Make("water");
            auto states = make_shared<CompoundTag>();
            states->fValue.emplace("liquid_depth", Int(0));
            water->fValue.emplace("states", states);
            w.write((uint8_t)Tag::TAG_Compound);
            w.write(string());
            water->write(w);
        }
        
        stream->drain(cd.fSubChunks[chunkY]);
    }

    static bool IsWaterLogged(mcfile::Block const& block) {
        using namespace std;
        static string const seagrass("minecraft:seagrass");
        static string const tall_seagrass("minecraft:tall_seagrass");
        static string const kelp("minecraft:kelp");
        static string const kelp_plant("minecraft:kelp_plant");
        static string const bubble_column("minecraft:bubble_column");
        if (!block.fProperties.empty()) {
            auto waterlogged = block.property("waterlogged", "");
            if (strings::Equals(waterlogged, "true")) {
                return true;
            }
        }
        auto const& name = block.fName;
        return strings::Equals(name, seagrass) || strings::Equals(name, tall_seagrass) || strings::Equals(name, kelp) || strings::Equals(name, kelp_plant) || strings::Equals(name, bubble_column);
    }

    static bool IsAir(std::string const& name) {
        static std::string const air("minecraft:air");
        static std::string const cave_air("minecraft:cave_air");
        return strings::Equals(name, air) || strings::Equals(name, cave_air);
    }

private:
    std::string const fInput;
    std::string const fOutput;
    InputOption const fInputOption;
    OutputOption const fOutputOption;

private:
    uint8_t const kBlockStorageVersion = 8;
};

}
