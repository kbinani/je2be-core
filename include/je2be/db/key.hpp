#pragma once

namespace j2b {

class Key {
public:
  enum class Tag : uint8_t {
    SubChunk = 0x2f,
    Version = 0x2c,
    VersionLegacy = 0x76,
    Data2D = 0x2d,
    BiomeState = 0x35,
    Checksums = 0x3b,
    BlockEntity = 0x31,
    Entity = 0x32,
    PendingTicks = 0x33,
    FinalizedState = 0x36,
    // HardCodedSpawnAreas
    StructureBounds = 0x39,
  };

  static std::string SubChunk(int32_t chunkX, int32_t chunkY, int32_t chunkZ, Dimension dim) {
    std::vector<char> b;
    PlaceXZTag(b, chunkX, chunkZ, Tag::SubChunk);
    SetDimension(b, dim);
    uint8_t const y = (uint8_t)(std::min)((std::max)(chunkY, 0), 255);
    b.push_back(y);
    return std::string(b.data(), b.size());
  }

  static std::string Version(int32_t chunkX, int32_t chunkZ, Dimension dim) {
    return ComposeChunkKey(chunkX, chunkZ, dim, Tag::Version);
  }

  static std::string VersionLegacy(int32_t chunkX, int32_t chunkZ, Dimension dim) {
    return ComposeChunkKey(chunkX, chunkZ, dim, Tag::VersionLegacy);
  }

  static std::string Data2D(int32_t chunkX, int32_t chunkZ, Dimension dim) {
    return ComposeChunkKey(chunkX, chunkZ, dim, Tag::Data2D);
  }

  static std::string BiomeState(int32_t chunkX, int32_t chunkZ, Dimension dim) {
    return ComposeChunkKey(chunkX, chunkZ, dim, Tag::BiomeState);
  }

  static std::string Checksums(int32_t chunkX, int32_t chunkZ, Dimension dim) {
    return ComposeChunkKey(chunkX, chunkZ, dim, Tag::Checksums);
  }

  static std::string Portals() { return "portals"; }

  static std::string BlockEntity(int32_t chunkX, int32_t chunkZ, Dimension dim) {
    return ComposeChunkKey(chunkX, chunkZ, dim, Tag::BlockEntity);
  }

  static std::string Entity(int32_t chunkX, int32_t chunkZ, Dimension dim) {
    return ComposeChunkKey(chunkX, chunkZ, dim, Tag::Entity);
  }

  static std::string PendingTicks(int32_t chunkX, int32_t chunkZ, Dimension dim) {
    return ComposeChunkKey(chunkX, chunkZ, dim, Tag::PendingTicks);
  }

  static std::string Map(int64_t id) { return std::string("map_") + std::to_string(id); }

  static std::string TheEnd() { return "TheEnd"; }

  static std::string AutonomousEntities() { return "AutonomousEntities"; }

  static std::string FinalizedState(int32_t chunkX, int32_t chunkZ, Dimension dim) {
    return ComposeChunkKey(chunkX, chunkZ, dim, Tag::FinalizedState);
  }

  static std::string StructureBounds(int32_t chunkX, int32_t chunkZ, Dimension dim) {
    return ComposeChunkKey(chunkX, chunkZ, dim, Tag::StructureBounds);
  }

  static std::string LocalPlayer() { return "~local_player"; }

  static std::string MobEvents() { return "mobevents"; }

  static std::string ComposeChunkKey(int32_t chunkX, int32_t chunkZ, Dimension dim, uint8_t tag) {
    std::vector<char> b;
    PlaceXZTag(b, chunkX, chunkZ, tag);
    SetDimension(b, dim);
    return std::string(b.data(), b.size());
  }

  static std::string ComposeChunkKey(int32_t chunkX, int32_t chunkZ, Dimension dim, Tag tag) {
    return ComposeChunkKey(chunkX, chunkZ, dim, static_cast<uint8_t>(tag));
  }

private:
  static void PlaceXZTag(std::vector<char> &out, int32_t chunkX, int32_t chunkZ, Tag tag) {
    PlaceXZTag(out, chunkX, chunkZ, static_cast<uint8_t>(tag));
  }

  static void PlaceXZTag(std::vector<char> &out, int32_t chunkX, int32_t chunkZ, uint8_t tag) {
    out.clear();
    out.resize(9);
    *(uint32_t *)out.data() = mcfile::Int32LEFromNative(*(uint32_t *)&chunkX);
    *(uint32_t *)(out.data() + 4) = mcfile::Int32LEFromNative(*(uint32_t *)&chunkZ);
    out[8] = tag;
  }

  static void SetDimension(std::vector<char> &out, Dimension dim) {
    if (dim == Dimension::Overworld) {
      return;
    }
    assert(out.size() == 9);
    uint8_t tag = out[8];
    out.resize(13);
    uint32_t const v = static_cast<uint8_t>(dim);
    *(uint32_t *)(out.data() + 8) = mcfile::Int32LEFromNative(v);
    out[12] = tag;
  }

private:
  Key() = delete;
};

} // namespace j2b
