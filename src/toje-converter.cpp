#include <je2be/toje/converter.hpp>

#include <je2be/nbt.hpp>
#include <je2be/toje/options.hpp>
#include <je2be/toje/progress.hpp>

#include "_props.hpp"
#include "enums/_game-mode.hpp"
#include "toje/_context.hpp"
#include "toje/_level-data.hpp"
#include "toje/_world.hpp"

#include <atomic>

namespace je2be::toje {

class Converter::Impl {
public:
  static Status Run(std::filesystem::path const &input, std::filesystem::path const &output, Options const &options, unsigned concurrency, Progress *progress = nullptr) {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;
    namespace fs = std::filesystem;

    unique_ptr<DB> db(Open(input / "db"));
    if (!db) {
      return JE2BE_ERROR;
    }

    if (!PrepareOutputDirectory(output)) {
      return JE2BE_ERROR;
    }

    CompoundTagPtr dat;
    auto endian = LevelData::Read(input / "level.dat", dat);
    if (!endian || !dat) {
      return JE2BE_ERROR;
    }

    int total = 0;
    map<Dimension, vector<pair<Pos2i, Context::ChunksInRegion>>> regions;
    int64_t gameTick = dat->int64("currentTick", 0);
    int32_t gameTypeB = dat->int32("GameType", 0);
    GameMode gameMode = GameMode::Survival;
    if (auto t = GameModeFromBedrock(gameTypeB); t) {
      gameMode = *t;
    }
    auto bin = Context::Init(*db, options, *endian, regions, total, gameTick, gameMode, concurrency);

    auto levelDat = LevelData::Import(*dat, *db, options, *bin);
    if (!levelDat) {
      return JE2BE_ERROR;
    }

    atomic<int> done = 0;
    atomic<bool> cancelRequested = false;
    auto reportProgress = [progress, &done, total, &cancelRequested]() -> bool {
      int d = done.fetch_add(1) + 1;
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
      if (!options.fDimensionFilter.empty()) {
        if (options.fDimensionFilter.find(d) == options.fDimensionFilter.end()) {
          continue;
        }
      }
      shared_ptr<Context> result;
      if (auto st = World::Convert(d, regions[d], *db, output, concurrency, *bin, result, reportProgress); !st.ok()) {
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

    LevelData::UpdateDataPacksAndEnabledFeatures(*levelDat, *bin);

    if (!LevelData::Write(*levelDat, output / "level.dat")) {
      return JE2BE_ERROR;
    }

    return bin->postProcess(output, *db);
  }

private:
  static bool PrepareOutputDirectory(std::filesystem::path const &output) {
    using namespace std;
    namespace fs = std::filesystem;
    error_code ec;
    fs::create_directories(output, ec);
    if (ec) {
      return false;
    }
    ec.clear();

    fs::directory_iterator iterator(output, ec);
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
};

Status Converter::Run(std::filesystem::path const &input, std::filesystem::path const &output, Options const &options, unsigned concurrency, Progress *progress) {
  return Impl::Run(input, output, options, concurrency, progress);
}

} // namespace je2be::toje
