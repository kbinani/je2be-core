#pragma once

namespace je2be::tobe {

enum class StructureType : uint8_t {
  Fortress = 1,
  Monument = 3,
  Outpost = 5,
};

struct StructurePiece {
  StructurePiece(Pos3i start, Pos3i end, StructureType type) : fVolume(start, end), fType(type) {}

  Volume fVolume;
  StructureType fType;

  std::shared_ptr<mcfile::nbt::Tag> toNbt() const {
    using namespace std;
    using namespace mcfile::nbt;
    auto tag = make_shared<CompoundTag>();
    (*tag)["type"] = make_shared<ByteTag>(static_cast<uint8_t>(fType));
    (*tag)["volume"] = fVolume.toNbt();
    return tag;
  }

  static std::optional<StructurePiece> FromNbt(mcfile::nbt::Tag const &tag) {
    using namespace std;
    using namespace mcfile::nbt;
    auto c = tag.asCompound();
    if (!c) {
      return nullopt;
    }
    auto typeTag = c->byte("type");
    auto volumeTag = c->compoundTag("volume");
    if (!typeTag || !volumeTag) {
      return nullopt;
    }
    StructureType type;
    switch (*typeTag) {
    case static_cast<uint8_t>(StructureType::Fortress):
      type = StructureType::Fortress;
      break;
    case static_cast<uint8_t>(StructureType::Monument):
      type = StructureType::Monument;
      break;
    case static_cast<uint8_t>(StructureType::Outpost):
      type = StructureType::Outpost;
      break;
    default:
      return nullopt;
    }
    auto volume = Volume::FromNbt(*volumeTag);
    if (!volume) {
      return nullopt;
    }
    return StructurePiece(volume->fStart, volume->fEnd, type);
  }
};

} // namespace je2be::tobe
