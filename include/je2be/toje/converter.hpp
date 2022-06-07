#pragma once

namespace je2be::toje {

class Converter {
public:
  Converter(std::string const &input, std::string const &output, Options) = delete;
  Converter(std::string const &input, std::wstring const &output, Options) = delete;
  Converter(std::wstring const &input, std::string const &output, Options) = delete;
  Converter(std::wstring const &input, std::wstring const &output, Options) = delete;
  Converter(std::filesystem::path const &input, std::filesystem::path const &output, Options o)
      : fInput(input), fOutput(output), fOptions(o) {
  }

  Status run(unsigned concurrency, Progress *progress = nullptr) {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;
    namespace fs = std::filesystem;

    unique_ptr<DB> db(Open(fInput / "db"));
    if (!db) {
      return JE2BE_ERROR;
    }

    if (!prepareOutputDirectory()) {
      return JE2BE_ERROR;
    }

    CompoundTagPtr dat;
    auto endian = LevelData::Read(fInput / "level.dat", dat);
    if (!endian || !dat) {
      return JE2BE_ERROR;
    }

    int total = 0;
    map<Dimension, unordered_map<Pos2i, Context::ChunksInRegion, Pos2iHasher>> regions;
    int64_t gameTick = dat->int64("currentTick", 0);
    auto bin = Context::Init(*db, fOptions, *endian, regions, total, gameTick, BlockEntity::FromBlockAndBlockEntity);

    auto levelDat = LevelData::Import(*dat, *db, fOptions, *bin);
    if (!levelDat) {
      return JE2BE_ERROR;
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
      if (!fOptions.fDimensionFilter.empty()) {
        if (fOptions.fDimensionFilter.find(d) == fOptions.fDimensionFilter.end()) {
          continue;
        }
      }
      shared_ptr<Context> result;
      if (auto st = World::Convert(d, regions[d], *db, fOutput, concurrency, *bin, result, reportProgress); !st.ok()) {
        return st;
      }
      if (result) {
        result->mergeInto(*bin);
      }
      if (cancelRequested.load()) {
        return JE2BE_ERROR;
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
          auto rootVehicleTag = Compound();
          rootVehicleTag->set("Entity", entity);
          rootVehicleTag->set("Attach", vehicleUuid.toIntArrayTag());
          player->set("RootVehicle", rootVehicleTag);
        }
      }
    }

    if (auto data = levelDat->compoundTag("Data"); data) {
      if (auto player = data->compoundTag("Player"); player) {
        CompoundTagPtr shoulderEntityLeft;
        CompoundTagPtr shoulderEntityRight;
        bin->drainShoulderEntities(shoulderEntityLeft, shoulderEntityRight);
        if (shoulderEntityLeft) {
          player->set("ShoulderEntityLeft", shoulderEntityLeft);
        }
        if (shoulderEntityRight) {
          player->set("ShoulderEntityRight", shoulderEntityRight);
        }
      }
    }

    if (!LevelData::Write(*levelDat, fOutput / "level.dat")) {
      return JE2BE_ERROR;
    }

    if (!Poi::Export(*db, fOutput)) {
      return JE2BE_ERROR;
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
    leveldb::Options o;
    o.compression = kZlibRawCompression;
    DB *db;
    leveldb::Status st = DB::Open(o, name, &db);
    if (!st.ok()) {
      return nullptr;
    }
    return db;
  }

private:
  std::filesystem::path const fInput;
  Options const fOptions;
  std::filesystem::path const fOutput;
};

} // namespace je2be::toje
