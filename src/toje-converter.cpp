#include <je2be/toje/converter.hpp>

#include <je2be/nbt.hpp>
#include <je2be/toje/options.hpp>
#include <je2be/toje/progress.hpp>

#include "_props.hpp"
#include "_queue2d.hpp"
#include "_walk.hpp"
#include "enums/_game-mode.hpp"
#include "terraform/_leaves.hpp"
#include "terraform/java/_block-accessor-java-directory.hpp"
#include "toje/_context.hpp"
#include "toje/_level-data.hpp"
#include "toje/_world.hpp"
#include "toje/terraform/_lighting.hpp"

#include <atomic>
#include <latch>
#include <thread>

using namespace std;
namespace fs = std::filesystem;

namespace je2be::toje {

class Converter::Impl {
public:
  static Status Run(std::filesystem::path const &input, std::filesystem::path const &output, Options const &options, unsigned concurrency, Progress *progress = nullptr) {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;
    namespace fs = std::filesystem;

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
    auto bin = Context::Init(input / "db", options, *endian, regions, total, gameTick, gameMode, concurrency);

    auto db = make_unique<mcfile::be::Db>(input / "db");
    if (!db) {
      return JE2BE_ERROR;
    }
    auto levelDat = LevelData::Import(*dat, *db, options, *bin);
    if (!levelDat) {
      return JE2BE_ERROR;
    }

    atomic<int> done = 0;
    atomic<bool> cancelRequested = false;
    atomic_uint64_t numConvertedChunks(0);
    auto reportProgress = [progress, &done, total, &cancelRequested, &numConvertedChunks]() -> bool {
      double p = double(done.fetch_add(1) + 1) / total;
      if (progress) {
        bool ok = progress->report(p, numConvertedChunks.load());
        if (!ok) {
          cancelRequested = true;
        }
        return ok;
      } else {
        return true;
      }
    };

    map<Dimension, fs::path> terrainTempDirs;
    for (Dimension d : {Dimension::Overworld, Dimension::Nether, Dimension::End}) {
      if (!options.fDimensionFilter.empty()) {
        if (options.fDimensionFilter.find(d) == options.fDimensionFilter.end()) {
          continue;
        }
      }
      auto terrainTempDir = File::CreateTempDir(options.getTempDirectory());
      if (!terrainTempDir) {
        return JE2BE_ERROR;
      }
      terrainTempDirs[d] = *terrainTempDir;
      shared_ptr<Context> result;
      if (auto st = World::Convert(d, regions[d], *db, output, concurrency, *bin, result, reportProgress, numConvertedChunks, *terrainTempDir); !st.ok()) {
        return st;
      }
      if (result) {
        result->mergeInto(*bin);
      }
      if (cancelRequested.load()) {
        return JE2BE_ERROR;
      }
    }

    if (auto st = Terraform(regions, output, terrainTempDirs, concurrency); !st.ok()) {
      return st;
    }
    for (auto const &[_, dir] : terrainTempDirs) {
      Fs::DeleteAll(dir);
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
  static Status Terraform(map<mcfile::Dimension, vector<pair<Pos2i, Context::ChunksInRegion>>> const &regions, fs::path const &output, map<mcfile::Dimension, fs::path> const &terrainTempDirs, unsigned int concurrency) {
    if (regions.empty()) {
      return Status::Ok();
    }

    map<mcfile::Dimension, shared_ptr<Queue2d>> queues;

    for (auto const &i : regions) {
      if (i.second.empty()) {
        continue;
      }
      optional<Pos2i> minR;
      optional<Pos2i> maxR;
      for (auto const &j : i.second) {
        Pos2i const &region = j.first;
        if (minR && maxR) {
          int minRx = (std::min)(minR->fX, region.fX);
          int maxRx = (std::max)(maxR->fX, region.fX);
          int minRz = (std::min)(minR->fZ, region.fZ);
          int maxRz = (std::max)(maxR->fZ, region.fZ);
          minR = Pos2i(minRx, minRz);
          maxR = Pos2i(maxRx, maxRz);
        } else {
          minR = region;
          maxR = region;
        }
      }
      assert(minR && maxR);

      int width = maxR->fX - minR->fX + 1;
      int height = maxR->fZ - minR->fZ + 1;
      auto queue = make_shared<Queue2d>(*minR, width, height);
      for (int x = minR->fX; x <= maxR->fX; x++) {
        for (int z = minR->fZ; z <= maxR->fZ; z++) {
          queue->setDone({x, z}, true);
        }
      }
      for (auto const &j : i.second) {
        Pos2i const &region = j.first;
        queue->setDone(region, false);
        queue->setWeight(region, j.second.fChunks.size());
      }
      queues[i.first] = queue;
    }

    int numThreads = concurrency - 1;
    std::latch latch(concurrency);
    atomic_bool ok(true);
    mutex mut;

    auto action = [&latch, &queues, &mut, output, &ok, terrainTempDirs]() {
      shared_ptr<terraform::java::BlockAccessorJavaDirectory<3, 3>> blockAccessor;
      optional<mcfile::Dimension> prevDimension;

      while (ok) {
        optional<pair<mcfile::Dimension, Pos2i>> next;
        {
          lock_guard<mutex> lock(mut);
          for (auto const &it : queues) {
            if (auto n = it.second->next(); n) {
              next = make_pair(it.first, *n);
              break;
            }
          }
        }
        if (!next) {
          break;
        }

        mcfile::Dimension dim = next->first;
        if (dim != prevDimension) {
          blockAccessor.reset();
          prevDimension = dim;
        }

        Pos2i region = next->second;
        int rx = region.fX;
        int rz = region.fZ;

        fs::path directory;
        switch (dim) {
        case mcfile::Dimension::Nether:
          directory = output / "DIM-1" / "region";
          break;
        case mcfile::Dimension::End:
          directory = output / "DIM1" / "region";
          break;
        case mcfile::Dimension::Overworld:
        default:
          directory = output / "region";
          break;
        }

        auto found = terrainTempDirs.find(dim);
        if (found == terrainTempDirs.end()) {
          ok = false;
          break;
        }

        auto name = mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
        auto mcaIn = found->second / name;
        auto mcaOut = directory / name;
        auto editor = mcfile::je::McaEditor::Open(mcaIn);
        if (!editor) {
          ok = false;
          break;
        }

        for (int x = 0; x < 32; x++) {
          for (int z = 0; z < 32; z++) {
            int cx = x + rx * 32;
            int cz = z + rz * 32;
            if (!TerraformRegion(cx, cz, *editor, found->second, blockAccessor, dim).ok()) {
              ok = false;
              break;
            }
          }
          if (!ok) {
            break;
          }
        }

        if (!editor->write(mcaOut)) {
          ok = false;
        }

        {
          lock_guard<mutex> lock(mut);
          queues[dim]->unlock({region});
        }
      }
      latch.count_down();
    };

    vector<thread> threads;
    for (int i = 0; i < numThreads; i++) {
      threads.push_back(thread(action));
    }
    action();
    latch.wait();
    for (auto &th : threads) {
      th.join();
    }
    return Status::Ok();
  }

  static Status TerraformRegion(int cx, int cz, mcfile::je::McaEditor &editor, fs::path inputDirectory, std::shared_ptr<terraform::java::BlockAccessorJavaDirectory<3, 3>> &blockAccessor, mcfile::Dimension dim) {
    int rx = mcfile::Coordinate::RegionFromChunk(cx);
    int rz = mcfile::Coordinate::RegionFromChunk(cz);

    int x = cx - rx * 32;
    int z = cz - rz * 32;

    auto current = editor.extract(x, z);
    if (!current) {
      return Status::Ok();
    }

    if (!blockAccessor) {
      blockAccessor.reset(new terraform::java::BlockAccessorJavaDirectory<3, 3>(cx - 1, cz - 1, inputDirectory));
    }
    if (blockAccessor->fChunkX != cx - 1 || blockAccessor->fChunkZ != cz - 1) {
      auto next = blockAccessor->makeRelocated(cx - 1, cz - 1);
      blockAccessor.reset(next);
    }
    blockAccessor->loadAllWith(editor, mcfile::Coordinate::RegionFromChunk(cx), mcfile::Coordinate::RegionFromChunk(cz));

    auto copy = current->copy();
    auto ch = mcfile::je::Chunk::MakeChunk(cx, cz, current);
    if (!ch) {
      return JE2BE_ERROR;
    }
    blockAccessor->set(ch);

    auto writable = mcfile::je::WritableChunk::MakeChunk(cx, cz, copy);
    if (!writable) {
      return JE2BE_ERROR;
    }

    terraform::BlockPropertyAccessorJava propertyAccessor(*ch);
    terraform::Leaves::Do(*writable, *blockAccessor, propertyAccessor);
    Lighting::Do(dim, *writable, *blockAccessor);

    auto tag = writable->toCompoundTag();
    if (!tag) {
      return JE2BE_ERROR;
    }
    if (!editor.insert(x, z, *tag)) {
      return JE2BE_ERROR;
    }

    return Status::Ok();
  }

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
};

Status Converter::Run(std::filesystem::path const &input, std::filesystem::path const &output, Options const &options, unsigned concurrency, Progress *progress) {
  return Impl::Run(input, output, options, concurrency, progress);
}

} // namespace je2be::toje
