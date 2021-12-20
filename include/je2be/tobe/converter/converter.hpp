#pragma once

namespace je2be::tobe {

class Converter {
public:
  Converter(std::string const &input, InputOption io, std::string const &output, OutputOption oo) = delete;
  Converter(std::string const &input, InputOption io, std::wstring const &output, OutputOption oo) = delete;
  Converter(std::wstring const &input, InputOption io, std::string const &output, OutputOption oo) = delete;
  Converter(std::wstring const &input, InputOption io, std::wstring const &output, OutputOption oo) = delete;
  Converter(std::filesystem::path const &input, InputOption io, std::filesystem::path const &output, OutputOption oo) : fInput(input), fOutput(output), fInputOption(io), fOutputOption(oo) {}

  std::optional<Statistics> run(unsigned int concurrency, Progress *progress = nullptr) {
    using namespace std;
    namespace fs = std::filesystem;
    using namespace mcfile;
    using namespace mcfile::je;

    double const numTotalChunks = getTotalNumChunks();

    auto rootPath = fOutput;
    auto dbPath = rootPath / "db";

    error_code ec;
    fs::create_directories(dbPath, ec);
    if (ec) {
      return nullopt;
    }

    auto data = LevelData::Read(fInputOption.getLevelDatFilePath(fInput));
    if (!data) {
      return nullopt;
    }
    LevelData levelData = LevelData::Import(*data);

    bool ok = Datapacks::Import(fInput, fOutput);

    auto worldData = std::make_unique<WorldData>(fInput, fInputOption);
    {
      RawDb db(dbPath, concurrency);
      if (!db.valid()) {
        return nullopt;
      }

      uint32_t done = 0;
      for (auto dim : {Dimension::Overworld, Dimension::Nether, Dimension::End}) {
        auto dir = fInputOption.getWorldDirectory(fInput, dim);
        mcfile::je::World world(dir);
        bool complete = World::Convert(world, dim, db, *worldData, concurrency, progress, done, numTotalChunks);
        ok &= complete;
        if (!complete) {
          break;
        }
      }

      auto localPlayerData = LocalPlayerData(*data, *worldData);
      if (localPlayerData) {
        auto k = mcfile::be::DbKey::LocalPlayer();
        db.put(k, *localPlayerData);
      }

      if (ok) {
        levelData.fCurrentTick = max(levelData.fCurrentTick, worldData->fMaxChunkLastUpdate);
        ok = levelData.write(fOutput / "level.dat");
        if (ok) {
          ok = worldData->put(db, *data);
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

    return worldData->fStat;
  }

private:
  static std::optional<std::string> LocalPlayerData(mcfile::nbt::CompoundTag const &tag, WorldData &wd) {
    using namespace mcfile::stream;
    using namespace mcfile::nbt;

    auto root = tag.compoundTag("");
    if (!root) {
      return std::nullopt;
    }
    auto data = root->compoundTag("Data");
    if (!data) {
      return std::nullopt;
    }
    auto player = data->compoundTag("Player");
    if (!player) {
      return std::nullopt;
    }

    WorldDataFragment wdf(mcfile::Dimension::Overworld);
    auto entity = Entity::LocalPlayer(*player, wd.fJavaEditionMap, wdf);
    if (!entity) {
      return std::nullopt;
    }
    wdf.drain(wd);

    auto s = std::make_shared<ByteStream>();
    OutputStreamWriter w(s, {.fLittleEndian = true});
    if (!entity->writeAsRoot(w)) {
      return std::nullopt;
    }

    std::vector<uint8_t> buffer;
    s->drain(buffer);
    std::string ret((char const *)buffer.data(), buffer.size());
    return ret;
  }

  double getTotalNumChunks() const {
    namespace fs = std::filesystem;
    uint32_t num = 0;
    for (auto dim : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
      auto dir = fInputOption.getWorldDirectory(fInput, dim) / "region";
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
        if (strings::StartsWith(name, "r.") && strings::EndsWith(name, ".mca")) {
          num++;
        }
      }
    }
    return num * 32.0 * 32.0;
  }

private:
  std::filesystem::path const fInput;
  std::filesystem::path const fOutput;
  InputOption const fInputOption;
  OutputOption const fOutputOption;
};

} // namespace je2be::tobe
