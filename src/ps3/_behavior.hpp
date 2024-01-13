#pragma once

#include <je2be/lce/behavior.hpp>

#include "_directory-iterator.hpp"

namespace je2be::ps3 {

class ConverterBehavior : public je2be::lce::Behavior {
public:
  Status decompressChunk(std::vector<uint8_t> &buffer) const override {
    if (buffer.size() < 9) {
      return JE2BE_ERROR;
    }
    std::vector<uint8_t> decoded;
    if (!mcfile::Compression::DecompressDeflate<decltype(decoded)>(buffer.data() + 8, buffer.size() - 8, decoded)) {
      return JE2BE_ERROR;
    }
    decoded.swap(buffer);
    return Status::Ok();
  }

  Status loadPlayers(std::filesystem::path const &inputDirectory, std::map<std::filesystem::path, CompoundTagPtr> &outBuffer) const override {
    for (DirectoryIterator it(inputDirectory); it.valid(); it.next()) {
      if (!it->is_regular_file()) {
        continue;
      }
      auto p = it->path();
      if (!p.has_filename()) {
        continue;
      }
      auto name = p.filename().u8string();
      if (!name.starts_with(u8"P_") || !name.ends_with(u8".dat")) {
        continue;
      }
      auto fs = std::make_shared<mcfile::stream::FileInputStream>(p);
      if (auto tag = CompoundTag::Read(fs, mcfile::Endian::Big); tag) {
        outBuffer[p] = tag;
      }
    }
    return Status::Ok();
  }
};
} // namespace je2be::ps3
