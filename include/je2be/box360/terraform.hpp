#pragma once

#include <je2be/status.hpp>

#include <minecraft-file.hpp>

#include <filesystem>

namespace je2be::box360 {

class Progress;

class Terraform {
  Terraform() = delete;
  class Impl;

public:
  static Status Do(mcfile::Dimension dim, std::filesystem::path const &poiDirectory, std::filesystem::path const &directory, unsigned int concurrency, Progress *progress, int progressChunksOffset);
};

} // namespace je2be::box360
