#pragma once

#include <je2be/integers.hpp>
#include <je2be/status.hpp>

#include <minecraft-file.hpp>

#include <filesystem>

namespace je2be::lce {

class Progress;

class Terraform {
  Terraform() = delete;
  class Impl;

public:
  static Status Do(mcfile::Dimension dim, std::filesystem::path const &poiDirectory, std::filesystem::path const &directory, unsigned int concurrency, Progress *progress, u64 progressChunksOffset);
};

} // namespace je2be::lce
