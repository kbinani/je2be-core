#pragma once

namespace je2be::tobe {

class Converter {
public:
  Converter(std::string const &input, std::string const &output, Options o) = delete;
  Converter(std::string const &input, std::wstring const &output, Options o) = delete;
  Converter(std::wstring const &input, std::string const &output, Options o) = delete;
  Converter(std::wstring const &input, std::wstring const &output, Options o) = delete;
  Converter(std::filesystem::path const &input, std::filesystem::path const &output, Options o) : fInput(input), fOutput(output), fOptions(o) {}

  Status run(int concurrency, Progress *progress = nullptr) {
    using namespace std;
    namespace fs = std::filesystem;
    using namespace mcfile;
    using namespace mcfile::je;

    SessionLock lock(fInput);
    if (!lock.lock()) {
      return JE2BE_ERROR;
    }

    double const numTotalChunks = getTotalNumChunks();

    auto rootPath = fOutput;
    auto dbPath = rootPath / "db";

    error_code ec;
    fs::create_directories(dbPath, ec);
    if (ec) {
      return JE2BE_ERROR_WHAT(ec.message());
    }

    auto data = Level::Read(fOptions.getLevelDatFilePath(fInput));
    if (!data) {
      return JE2BE_ERROR;
    }
    Level level = Level::Import(*data);

    bool ok = Datapacks::Import(fInput, fOutput);

    auto levelData = std::make_unique<LevelData>(fInput, fOptions, level.fCurrentTick, level.fDifficulty, level.fCommandsEnabled);
    {
      RawDb db(dbPath, concurrency);
      if (!db.valid()) {
        return JE2BE_ERROR;
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
        if (levelData->fSpectatorUsed) {
          level.fExperiments["spectator_mode"] = true;
        }
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
    Context ctx(ld.fJavaEditionMap, wd, ld.fGameTick, ld.fDifficultyBedrock, ld.fAllowCommand, TileEntity::FromBlockAndTileEntity);
    auto playerB = Entity::LocalPlayer(*playerJ, ctx);
    if (!playerB) {
      return std::nullopt;
    }
    wd.drain(ld);
    ld.fSpectatorUsed = playerB->fSpectatorUsed;

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
