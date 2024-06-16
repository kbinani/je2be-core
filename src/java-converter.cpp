#include <je2be/java/converter.hpp>

#if defined(_MSC_VER)
#define NOMINMAX
#undef small
#include <windows.h>
#endif

#if defined(__GNUC__)
#include <fcntl.h>
#endif

#include <je2be/java/options.hpp>
#include <je2be/java/progress.hpp>

#include "_directory-iterator.hpp"
#include "_parallel.hpp"
#include "db/_concurrent-db.hpp"
#include "java/_context.hpp"
#include "java/_datapacks.hpp"
#include "java/_entity-store.hpp"
#include "java/_entity.hpp"
#include "java/_level.hpp"
#include "java/_region.hpp"
#include "java/_session-lock.hpp"
#include "java/_world-data.hpp"
#include "java/_world.hpp"

namespace je2be::java {

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

    auto levelData = std::make_unique<LevelData>(input, o, level.fCurrentTick, level.fDifficulty, level.fCommandsEnabled, level.fGameType, level.fDataVersion);
    ConcurrentDb db(dbPath, concurrency, o.fDbTempDirectory);
    if (!db.valid()) {
      return JE2BE_ERROR;
    }

    auto localPlayerData = LocalPlayerData(*data, *levelData);
    if (localPlayerData) {
      auto k = mcfile::be::DbKey::LocalPlayer();
      if (auto st = db.put(k, *localPlayerData); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
    }

    struct Work {
      Dimension fDim;
      shared_ptr<mcfile::je::Region> fRegion;

      Work(Dimension dim, shared_ptr<mcfile::je::Region> const &region) : fDim(dim), fRegion(region) {}
    };
    struct Result {
      map<mcfile::Dimension, shared_ptr<WorldData>> fData;

      void mergeInto(Result &out) const {
        for (auto const &it : fData) {
          if (!it.second) {
            continue;
          }
          if (auto found = out.fData.find(it.first); found != out.fData.end()) {
            if (found->second) {
              it.second->drain(*found->second);
            }
          } else {
            out.fData[it.first] = it.second;
          }
        }
      }
    };
    map<mcfile::Dimension, std::shared_ptr<EntityStore>> entityStores;
    vector<Work> works;
    for (auto dim : {Dimension::Overworld, Dimension::Nether, Dimension::End}) {
      if (!o.fDimensionFilter.empty()) [[unlikely]] {
        if (o.fDimensionFilter.find(dim) == o.fDimensionFilter.end()) {
          continue;
        }
      }
      auto entityStoreDir = mcfile::File::CreateTempDir(o.getTempDirectory());
      if (!entityStoreDir) {
        return JE2BE_ERROR;
      }
      auto entityStore = EntityStore::Open(*entityStoreDir);
      if (!entityStore) {
        return JE2BE_ERROR;
      }
      entityStores[dim].reset(entityStore);
      auto dir = o.getWorldDirectory(input, dim);
      mcfile::je::World world(dir);
      world.eachRegions([dim, &works](shared_ptr<mcfile::je::Region> const &region) {
        Work work(dim, region);
        works.push_back(work);
        return true;
      });
    }
    atomic_uint32_t done(0);
    atomic_bool abortSignal(false);
    atomic_uint64_t numConvertedChunks(0);
    LevelData const *ldPtr = levelData.get();
    auto [result, status] = Parallel::Reduce<Work, Result>(
        works,
        concurrency,
        Result(),
        [ldPtr, &db, progress, &done, numTotalChunks, &abortSignal, entityStores, o, &numConvertedChunks](Work const &work) -> pair<Result, Status> {
          auto found = entityStores.find(work.fDim);
          assert(found != entityStores.end());
          shared_ptr<EntityStore> entityStore = found->second;
          auto worldData = Region::Convert(
              work.fDim,
              work.fRegion,
              o,
              entityStore,
              *ldPtr,
              db,
              progress,
              done,
              numTotalChunks,
              abortSignal,
              numConvertedChunks);
          Result ret;
          ret.fData[work.fDim] = worldData;
          return make_pair(ret, Status::Ok());
        },
        [](Result const &from, Result &to) -> void {
          from.mergeInto(to);
        });
    if (!status.ok()) {
      return JE2BE_ERROR_PUSH(status);
    }
    u64 totalEntityChunks = 0;
    for (auto const &it : entityStores) {
      totalEntityChunks += it.second->fChunks.size();
    }
    atomic<u64> doneEntityChunks(0);
    for (auto const &it : result.fData) {
      mcfile::Dimension dim = it.first;

      it.second->drain(*levelData);
      auto entityStore = entityStores[dim];
      assert(entityStore);
      if (!entityStore) {
        continue;
      }

      if (auto st = World::PutWorldEntities(dim, db, entityStore, concurrency, progress, doneEntityChunks, totalEntityChunks); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
    }
    if (progress) {
      u64 v = std::max<u64>(1, totalEntityChunks);
      if (!progress->reportEntityPostProcess({v, v})) {
        return JE2BE_ERROR;
      }
    }

    if (ok) {
      level.fCurrentTick = max(level.fCurrentTick, levelData->fMaxChunkLastUpdate);
      for (auto const &ex : levelData->fExperiments) {
        level.fExperiments[ex] = true;
      }
      level.fCheatsEnabled = levelData->fAllowCommand;
      ok = level.write(output / "level.dat");
      if (ok) {
        if (auto st = levelData->put(db, *data, levelData->fUuids); !st.ok()) {
          return JE2BE_ERROR_PUSH(st);
        }
      }
    }

    if (ok) {
      auto st = db.close([progress](Rational<u64> const &p) {
        if (!progress) {
          return true;
        }
        return progress->reportCompaction(p);
      });
      if (!st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
    } else {
      db.abandon();
    }
    if (!ok) {
      return JE2BE_ERROR;
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

    auto data = tag.compoundTag(u8"Data");
    if (!data) {
      return std::nullopt;
    }
    auto playerJ = data->compoundTag(u8"Player");
    if (!playerJ) {
      return std::nullopt;
    }

    WorldData wd(mcfile::Dimension::Overworld);
    Context ctx(ld.fJavaEditionMap, ld.fLodestones, ld.fUuids, wd, ld.fGameTick, ld.fDifficultyBedrock, ld.fAllowCommand, ld.fGameType);
    DataVersion dataVersion(ld.fDataVersion, ld.fDataVersion);
    auto playerB = Entity::LocalPlayer(*playerJ, ctx, dataVersion, {});
    if (!playerB) {
      return std::nullopt;
    }
    wd.drain(ld);

    LevelData::PlayerAttachedEntities pae;
    pae.fDim = playerB->fDimension;
    pae.fLocalPlayerUid = playerB->fUid;
    if (auto rootVehicle = playerJ->compoundTag(u8"RootVehicle"); rootVehicle) {
      if (auto entityJ = rootVehicle->compoundTag(u8"Entity"); entityJ) {
        if (auto entityB = Entity::From(*entityJ, ctx, dataVersion, {}); entityB.fEntity) {
          LevelData::VehicleAndPassengers vap;
          vap.fChunk = playerB->fChunk;
          vap.fVehicle = entityB.fEntity;
          vap.fPassengers.swap(entityB.fPassengers);
          pae.fVehicle = vap;

          auto vehicleUid = entityB.fEntity->int64(u8"UniqueID");
          if (vehicleUid) {
            playerB->fEntity->set(u8"RideID", Long(*vehicleUid));
          }
        }
      }
    }

    struct Rider {
      u8string fBedrockKey;
      int fLinkId;
      float fRotation;

      Rider(u8string const &bedrockKey, int linkId, float rotation) : fBedrockKey(bedrockKey), fLinkId(linkId), fRotation(rotation) {}
    };
    double const kDistanceToPlayer = 0.4123145064147021;        // distance between player and parrot.
    double const kAngleAgainstPlayerFacing = 104.0364421885305; // angle in degrees
    auto linksTag = List<Tag::Type::Compound>();
    static unordered_map<u8string, Rider> const keys = {
        {u8"ShoulderEntityLeft", Rider(u8"LeftShoulderRiderID", 0, -kAngleAgainstPlayerFacing)},
        {u8"ShoulderEntityRight", Rider(u8"RightShoulderPassengerID", 1, kAngleAgainstPlayerFacing)}};
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
      auto pos = props::GetPos3f(*playerB->fEntity, u8"Pos");
      if (!pos) {
        continue;
      }
      auto rot = props::GetRotation(*playerJ, u8"Rotation");
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
      if (auto riderB = Entity::From(*shoulderEntity, ctx, dataVersion, {Entity::Flag::ShoulderRider}); riderB.fEntity) {
        auto id = riderB.fEntity->int64(u8"UniqueID");
        if (id) {
          riderB.fEntity->set(u8"Pos", riderPos3f.toListTag());

          int cx = mcfile::Coordinate::ChunkFromBlock((int)floorf(riderPos3f.fX));
          int cz = mcfile::Coordinate::ChunkFromBlock((int)floorf(riderPos3f.fZ));
          Pos2i chunkPos(cx, cz);
          pae.fShoulderRiders.push_back(make_pair(chunkPos, riderB.fEntity));

          playerB->fEntity->set(keyB, Long(*id));

          auto link = Compound();
          link->set(u8"entityID", Long(*id));
          link->set(u8"linkID", Int(linkId));
          linksTag->push_back(link);
        }
      }
    }
    if (!linksTag->empty()) {
      playerB->fEntity->set(u8"LinksTag", linksTag);
    }
    if (pae.fVehicle || !pae.fShoulderRiders.empty()) {
      ld.fPlayerAttachedEntities = pae;
    }

    return CompoundTag::Write(*playerB->fEntity, mcfile::Encoding::LittleEndian);
  }

private:
  static double GetTotalNumChunks(std::filesystem::path const &input, Options o) {
    namespace fs = std::filesystem;
    u32 num = 0;
    for (auto dim : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
      auto dir = o.getWorldDirectory(input, dim) / "region";
      if (!fs::exists(dir)) {
        continue;
      }
      for (DirectoryIterator itr(dir); itr.valid(); itr.next()) {
        auto name = itr->path().filename().string();
        if (!name.starts_with("r.") || !name.ends_with(".mca")) {
          continue;
        }
        if (!itr->is_regular_file()) {
          continue;
        }
        num++;
      }
    }
    return num * 32.0 * 32.0;
  }
};

Status Converter::Run(std::filesystem::path const &input, std::filesystem::path const &output, Options const &o, int concurrency, Progress *progress) {
  return Impl::Run(input, output, o, concurrency, progress);
}

} // namespace je2be::java
