#pragma once

namespace je2be::tobe {

class Converter {
public:
  Converter(std::string const &input, std::string const &output, Options o) = delete;
  Converter(std::string const &input, std::wstring const &output, Options o) = delete;
  Converter(std::wstring const &input, std::string const &output, Options o) = delete;
  Converter(std::wstring const &input, std::wstring const &output, Options o) = delete;
  Converter(std::filesystem::path const &input, std::filesystem::path const &output, Options o) : fInput(input), fOutput(output), fOptions(o) {}

  std::optional<Statistics> run(int concurrency, Progress *progress = nullptr) {
    using namespace std;
    namespace fs = std::filesystem;
    using namespace mcfile;
    using namespace mcfile::je;

    SessionLock lock(fInput);
    if (!lock.lock()) {
      return nullopt;
    }

    double const numTotalChunks = getTotalNumChunks();

    auto rootPath = fOutput;
    auto dbPath = rootPath / "db";

    error_code ec;
    fs::create_directories(dbPath, ec);
    if (ec) {
      return nullopt;
    }

    auto data = Level::Read(fOptions.getLevelDatFilePath(fInput));
    if (!data) {
      return nullopt;
    }
    Level level = Level::Import(*data);

    bool ok = Datapacks::Import(fInput, fOutput);

    auto levelData = std::make_unique<LevelData>(fInput, fOptions, level.fCurrentTick);
    {
      RawDb db(dbPath, concurrency);
      if (!db.valid()) {
        return nullopt;
      }

      auto localPlayerData = LocalPlayerData(*data, *levelData);
      if (localPlayerData) {
        auto k = mcfile::be::DbKey::LocalPlayer();
        db.put(k, *localPlayerData);
      }

      uint32_t done = 0;
      for (auto dim : {Dimension::Overworld, Dimension::Nether, Dimension::End}) {
        if (!fOptions.fDimensionFilter.empty()) [[unlikely]] {
          if (fOptions.fDimensionFilter.find(dim) == fOptions.fDimensionFilter.end()) {
            continue;
          }
        }
        auto dir = fOptions.getWorldDirectory(fInput, dim);
        mcfile::je::World world(dir);
        bool complete;
        if (concurrency > 0) {
          complete = World::ConvertMultiThread(world, dim, db, *levelData, concurrency, progress, done, numTotalChunks, fOptions);
        } else {
          complete = World::ConvertSingleThread(world, dim, db, *levelData, progress, done, numTotalChunks, fOptions);
        }
        ok &= complete;
        if (!complete) {
          break;
        }
      }

      if (ok) {
        level.fCurrentTick = max(level.fCurrentTick, levelData->fMaxChunkLastUpdate);
        ok = level.write(fOutput / "level.dat");
        if (ok) {
          ok = levelData->put(db, *data);
        }
      }

      if (ok) {
        ok = db.close([progress](double p) {
          if (!progress) {
            return;
          }
          progress->report(Progress::Phase::LevelDbCompaction, p, 1);
        });
      } else {
        db.abandon();
      }
    }

    return levelData->fStat;
  }

  static std::optional<std::string> LocalPlayerData(CompoundTag const &tag, LevelData &ld) {
    using namespace mcfile::stream;

    auto data = tag.compoundTag("Data");
    if (!data) {
      return std::nullopt;
    }
    auto player = data->compoundTag("Player");
    if (!player) {
      return std::nullopt;
    }

    WorldData wd(mcfile::Dimension::Overworld);
    Context ctx(ld.fJavaEditionMap, wd, ld.fGameTick, TileEntity::FromBlockAndTileEntity);
    auto converted = Entity::LocalPlayer(*player, ctx);
    if (!converted) {
      return std::nullopt;
    }
    wd.drain(ld);

    if (auto rootVehicle = player->compoundTag("RootVehicle"); rootVehicle) {
      if (auto entity = rootVehicle->compoundTag("Entity"); entity) {
        LevelData::RootVehicle rv;
        rv.fDim = converted->fDimension;
        rv.fLocalPlayerUid = converted->fUid;
        rv.fVehicle = entity;
        rv.fChunk = converted->fChunk;
        ld.fRootVehicle = rv;
      }
    }

    auto s = std::make_shared<ByteStream>();
    OutputStreamWriter w(s, std::endian::little);
    if (!converted->fEntity->writeAsRoot(w)) {
      return std::nullopt;
    }

    std::vector<uint8_t> buffer;
    s->drain(buffer);
    std::string ret((char const *)buffer.data(), buffer.size());
    return ret;
  }

private:
  double getTotalNumChunks() const {
    namespace fs = std::filesystem;
    uint32_t num = 0;
    for (auto dim : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
      auto dir = fOptions.getWorldDirectory(fInput, dim) / "region";
      if (!fs::exists(dir)) {
        continue;
      }
      std::error_code ec;
      fs::directory_iterator itr(dir, ec);
      if (ec) {
        continue;
      }
      for (auto const &e : itr) {
        ec.clear();
        if (!fs::is_regular_file(e.path(), ec)) {
          continue;
        }
        if (ec) {
          continue;
        }
        auto name = e.path().filename().string();
        if (name.starts_with("r.") && name.ends_with(".mca")) {
          num++;
        }
      }
    }
    return num * 32.0 * 32.0;
  }

private:
  std::filesystem::path const fInput;
  std::filesystem::path const fOutput;
  Options const fOptions;
};

} // namespace je2be::tobe
