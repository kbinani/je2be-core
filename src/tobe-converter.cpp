#include <je2be/tobe/converter.hpp>

#if __has_include(<windows.h>)
#define NOMINMAX
#undef small
#include <windows.h>
#endif

#if __has_include(<fcntl.h>)
#include <fcntl.h>
#endif

#include <je2be/tobe/options.hpp>
#include <je2be/tobe/progress.hpp>

#include <je2be/_parallel.hpp>
#include <je2be/db/_raw-db.hpp>
#include <je2be/tobe/_context.hpp>
#include <je2be/tobe/_datapacks.hpp>
#include <je2be/tobe/_entity.hpp>
#include <je2be/tobe/_level.hpp>
#include <je2be/tobe/_region.hpp>
#include <je2be/tobe/_session-lock.hpp>
#include <je2be/tobe/_world-data.hpp>
#include <je2be/tobe/_world.hpp>

namespace je2be::tobe {

class Converter::Impl {
  Impl() = delete;

public:
  static Status Run(std::filesystem::path const &input, std::filesystem::path const &output, Options const &o, int concurrency, Progress *progress = nullptr) {
    using namespace std;
    namespace fs = std::filesystem;
    using namespace mcfile;

    SessionLock lock(input);
    if (!lock.lock()) {
      return JE2BE_ERROR;
    }

    double const numTotalChunks = GetTotalNumChunks(input, o);

    auto rootPath = output;
    auto dbPath = rootPath / "db";

    error_code ec;
    fs::create_directories(dbPath, ec);
    if (ec) {
      return JE2BE_ERROR_WHAT(ec.message());
    }

    auto data = Level::Read(o.getLevelDatFilePath(input));
    if (!data) {
      return JE2BE_ERROR;
    }
    Level level = Level::Import(*data);

    bool ok = Datapacks::Import(input, output);

    auto levelData = std::make_unique<LevelData>(input, o, level.fCurrentTick, level.fDifficulty, level.fCommandsEnabled, level.fGameType);
    RawDb db(dbPath);
    if (!db.valid()) {
      return JE2BE_ERROR;
    }

    auto localPlayerData = LocalPlayerData(*data, *levelData);
    if (localPlayerData) {
      auto k = mcfile::be::DbKey::LocalPlayer();
      db.put(k, *localPlayerData);
    }

    struct Work {
      Dimension fDim;
      shared_ptr<mcfile::je::Region> fRegion;
    };
    struct Result {
      map<mcfile::Dimension, shared_ptr<WorldData>> fData;

      void mergeInto(Result &out) const {
        for (auto const &it : fData) {
          if (auto found = out.fData.find(it.first); found != out.fData.end()) {
            it.second->drain(*found->second);
          } else {
            out.fData[it.first] = it.second;
          }
        }
      }
    };
    map<mcfile::Dimension, fs::path> worldTempDirs;
    vector<Work> works;
    for (auto dim : {Dimension::Overworld, Dimension::Nether, Dimension::End}) {
      if (!o.fDimensionFilter.empty()) [[unlikely]] {
        if (o.fDimensionFilter.find(dim) == o.fDimensionFilter.end()) {
          continue;
        }
      }
      auto worldTempDir = mcfile::File::CreateTempDir(o.getTempDirectory());
      if (!worldTempDir) {
        return JE2BE_ERROR;
      }
      worldTempDirs[dim] = *worldTempDir;
      auto dir = o.getWorldDirectory(input, dim);
      mcfile::je::World world(dir);
      world.eachRegions([dim, worldTempDir, &works](shared_ptr<mcfile::je::Region> const &region) {
        Work work;
        work.fRegion = region;
        work.fDim = dim;
        works.push_back(work);
        return true;
      });
    }
    atomic_uint32_t done(0);
    atomic_bool abortSignal(false);
    LevelData const *ldPtr = levelData.get();
    Result result = Parallel::Reduce<Work, Result>(
        works,
        Result(),
        [ldPtr, &db, progress, &done, numTotalChunks, &abortSignal, worldTempDirs, o](Work const &work) -> Result {
          auto found = worldTempDirs.find(work.fDim);
          assert(found != worldTempDirs.end());
          fs::path worldTempDir = found->second;
          auto worldData = Region::Convert(
              work.fDim,
              work.fRegion,
              o,
              worldTempDir,
              *ldPtr,
              db,
              progress,
              done,
              numTotalChunks,
              abortSignal);
          Result ret;
          ret.fData[work.fDim] = worldData;
          return ret;
        },
        [](Result const &from, Result &to) -> void {
          from.mergeInto(to);
        });
    for (auto const &it : result.fData) {
      it.second->drain(*levelData);

      mcfile::Dimension dim = it.first;
      fs::path worldTempDir = worldTempDirs[dim];
      World::PutWorldEntities(dim, db, worldTempDir, concurrency);
    }

    if (ok) {
      level.fCurrentTick = max(level.fCurrentTick, levelData->fMaxChunkLastUpdate);
      ok = level.write(output / "level.dat");
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

    if (levelData->fError) {
      return Status(Status::ErrorData(*levelData->fError));
    } else {
      return Status::Ok();
    }
  }

  static std::optional<std::string> LocalPlayerData(CompoundTag const &tag, LevelData &ld) {
    using namespace std;
    using namespace mcfile::stream;

    auto data = tag.compoundTag("Data");
    if (!data) {
      return std::nullopt;
    }
    auto playerJ = data->compoundTag("Player");
    if (!playerJ) {
      return std::nullopt;
    }

    WorldData wd(mcfile::Dimension::Overworld);
    Context ctx(ld.fJavaEditionMap, wd, ld.fGameTick, ld.fDifficultyBedrock, ld.fAllowCommand, ld.fGameType);
    auto playerB = Entity::LocalPlayer(*playerJ, ctx);
    if (!playerB) {
      return std::nullopt;
    }
    wd.drain(ld);

    LevelData::PlayerAttachedEntities pae;
    pae.fDim = playerB->fDimension;
    pae.fLocalPlayerUid = playerB->fUid;
    if (auto rootVehicle = playerJ->compoundTag("RootVehicle"); rootVehicle) {
      if (auto entityJ = rootVehicle->compoundTag("Entity"); entityJ) {
        if (auto entityB = Entity::From(*entityJ, ctx); entityB.fEntity) {
          LevelData::VehicleAndPassengers vap;
          vap.fChunk = playerB->fChunk;
          vap.fVehicle = entityB.fEntity;
          vap.fPassengers.swap(entityB.fPassengers);
          pae.fVehicle = vap;

          auto vehicleUid = entityB.fEntity->int64("UniqueID");
          if (vehicleUid) {
            playerB->fEntity->set("RideID", Long(*vehicleUid));
          }
        }
      }
    }

    struct Rider {
      string fBedrockKey;
      int fLinkId;
      float fRotation;

      Rider(string const &bedrockKey, int linkId, float rotation) : fBedrockKey(bedrockKey), fLinkId(linkId), fRotation(rotation) {}
    };
    double const kDistanceToPlayer = 0.4123145064147021;        // distance between player and parrot.
    double const kAngleAgainstPlayerFacing = 104.0364421885305; // angle in degrees
    auto linksTag = List<Tag::Type::Compound>();
    static unordered_map<string, Rider> const keys = {
        {"ShoulderEntityLeft", Rider("LeftShoulderRiderID", 0, -kAngleAgainstPlayerFacing)},
        {"ShoulderEntityRight", Rider("RightShoulderPassengerID", 1, kAngleAgainstPlayerFacing)}};
    for (auto const &key : keys) {
      auto keyJ = key.first;
      auto keyB = key.second.fBedrockKey;
      auto linkId = key.second.fLinkId;
      auto angle = key.second.fRotation / 180.0f * std::numbers::pi;

      // player: [9.0087, 72.62, 1.96865] yaw: -174.737
      // riderLeft: [8.60121, 72.42, 2.03154]
      // riderRight: [9.39784, 72.42, 2.10492]
      // https://gyazo.com/e797402bff04e5c3db9bae4bd1730629
      auto shoulderEntity = playerJ->compoundTag(keyJ);
      if (!shoulderEntity) {
        continue;
      }
      auto pos = props::GetPos3f(*playerB->fEntity, "Pos");
      if (!pos) {
        continue;
      }
      auto rot = props::GetRotation(*playerJ, "Rotation");
      if (!rot) {
        continue;
      }
      float yaw = (rot->fYaw + 90.0) / 180.0 * std::numbers::pi;
      Pos2d facing = Pos2d(kDistanceToPlayer, 0).rotated(yaw);
      Pos2d rider = facing.rotated(angle);
      Pos2d riderPos2d = Pos2d(pos->fX, pos->fZ) + rider;
      Pos3f riderPos3f(riderPos2d.fX, pos->fY - 0.2, riderPos2d.fZ);
      // ground: 71 -> boat: 71.375 -> player: 72.62 -> parrot: 72.42
      // ground: 71 -> player: 72.62 -> parrot: 72.42
      if (auto riderB = Entity::From(*shoulderEntity, ctx); riderB.fEntity) {
        auto id = riderB.fEntity->int64("UniqueID");
        if (id) {
          riderB.fEntity->set("Pos", riderPos3f.toListTag());

          int cx = mcfile::Coordinate::ChunkFromBlock((int)floorf(riderPos3f.fX));
          int cz = mcfile::Coordinate::ChunkFromBlock((int)floorf(riderPos3f.fZ));
          Pos2i chunkPos(cx, cz);
          pae.fShoulderRiders.push_back(make_pair(chunkPos, riderB.fEntity));

          playerB->fEntity->set(keyB, Long(*id));

          auto link = Compound();
          link->set("entityID", Long(*id));
          link->set("linkID", Int(linkId));
          linksTag->push_back(link);
        }
      }
    }
    if (!linksTag->empty()) {
      playerB->fEntity->set("LinksTag", linksTag);
    }
    if (pae.fVehicle || !pae.fShoulderRiders.empty()) {
      ld.fPlayerAttachedEntities = pae;
    }

    return CompoundTag::Write(*playerB->fEntity, mcfile::Endian::Little);
  }

private:
  static double GetTotalNumChunks(std::filesystem::path const &input, Options o) {
    namespace fs = std::filesystem;
    uint32_t num = 0;
    for (auto dim : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
      auto dir = o.getWorldDirectory(input, dim) / "region";
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
};

Status Converter::Run(std::filesystem::path const &input, std::filesystem::path const &output, Options const &o, int concurrency, Progress *progress) {
  return Impl::Run(input, output, o, concurrency, progress);
}

} // namespace je2be::tobe
