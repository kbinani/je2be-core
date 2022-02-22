#pragma once

namespace je2be::toje {

class Converter {
public:
  Converter(std::string const &input, InputOption io, std::string const &output, OutputOption oo) = delete;
  Converter(std::string const &input, InputOption io, std::wstring const &output, OutputOption oo) = delete;
  Converter(std::wstring const &input, InputOption io, std::string const &output, OutputOption oo) = delete;
  Converter(std::wstring const &input, InputOption io, std::wstring const &output, OutputOption oo) = delete;
  Converter(std::filesystem::path const &input, InputOption io, std::filesystem::path const &output, OutputOption oo)
      : fInput(input), fInputOption(io), fOutput(output), fOutputOption(oo) {
  }

  bool run(unsigned concurrency, Progress *progress = nullptr) {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;
    namespace fs = std::filesystem;

    unique_ptr<DB> db(Open(fInput / "db"));
    if (!db) {
      return false;
    }

    if (!prepareOutputDirectory()) {
      return false;
    }

    auto dat = LevelData::Read(fInput / "level.dat");
    if (!dat) {
      return false;
    }

    int total = 0;
    map<Dimension, unordered_map<Pos2i, Context::ChunksInRegion, Pos2iHasher>> regions;
    auto bin = Context::Init(*db, fInputOption, regions, total);

    auto levelDat = LevelData::Import(*dat, *db, fInputOption, *bin);
    if (!levelDat) {
      return false;
    }

    atomic<int> done = 0;
    atomic<bool> cancelRequested = false;
    auto reportProgress = [progress, &done, total, &cancelRequested]() -> bool {
      int d = done.fetch_add(1);
      if (progress) {
        bool ok = progress->report(d, total);
        if (!ok) {
          cancelRequested = true;
        }
        return ok;
      } else {
        return true;
      }
    };

    for (Dimension d : {Dimension::Overworld, Dimension::Nether, Dimension::End}) {
      if (!fInputOption.fDimensionFilter.empty()) {
        if (fInputOption.fDimensionFilter.find(d) == fInputOption.fDimensionFilter.end()) {
          continue;
        }
      }
      auto result = World::Convert(d, regions[d], *db, fOutput, concurrency, *bin, reportProgress);
      if (result) {
        result->mergeInto(*bin);
      } else {
        return false;
      }
      if (cancelRequested.load()) {
        return false;
      }
    }

    if (auto rootVehicle = bin->drainRootVehicle(); rootVehicle) {
      Uuid vehicleUuid = rootVehicle->first;
      auto entity = rootVehicle->second;
      if (auto data = levelDat->compoundTag("Data"); data) {
        if (auto player = data->compoundTag("Player"); player) {
          if (auto vehiclePos = props::GetPos3d(*entity, "Pos"); vehiclePos) {
            if (auto playerPos = props::GetPos3d(*player, "Pos"); playerPos) {
              auto vehicleId = entity->string("id", "");
              if (vehicleId == "minecraft:boat") {
                playerPos->fY = vehiclePos->fY - 0.45;
              } else if (vehicleId == "minecraft:minecart") {
                if (entity->boolean("OnGround", false)) {
                  playerPos->fY = vehiclePos->fY - 0.35;
                } else {
                  playerPos->fY = vehiclePos->fY - 0.2875;
                }
              }
              player->set("Pos", playerPos->toListTag());
            }
          }
          auto rootVehicleTag = make_shared<CompoundTag>();
          rootVehicleTag->set("Entity", entity);
          rootVehicleTag->set("Attach", vehicleUuid.toIntArrayTag());
          player->set("RootVehicle", rootVehicleTag);
        }
      }
    }
    if (!LevelData::Write(*levelDat, fOutput / "level.dat")) {
      return false;
    }

    return bin->postProcess(fOutput, *db);
  }

private:
  bool prepareOutputDirectory() {
    using namespace std;
    namespace fs = std::filesystem;
    error_code ec;
    fs::create_directories(fOutput, ec);
    if (ec) {
      return false;
    }
    ec.clear();

    fs::directory_iterator iterator(fOutput, ec);
    if (ec) {
      return false;
    }
    ec.clear();
    for (auto it : iterator) {
      fs::remove_all(it.path(), ec);
      ec.clear();
    }
    return true;
  }

  static leveldb::DB *Open(std::filesystem::path name) {
    using namespace leveldb;
    Options o;
    o.compression = kZlibRawCompression;
    DB *db;
    Status st = DB::Open(o, name, &db);
    if (!st.ok()) {
      return nullptr;
    }
    return db;
  }

private:
  std::filesystem::path const fInput;
  InputOption const fInputOption;
  std::filesystem::path const fOutput;
  OutputOption const fOutputOption;
};

} // namespace je2be::toje
