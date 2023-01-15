static void CheckLight(Pos3i const &origin, std::vector<uint8_t> &e, std::vector<uint8_t> &a) {
  auto dataE = mcfile::Data4b3dView::Make(origin, 16, 16, 16, &e);
  auto dataA = mcfile::Data4b3dView::Make(origin, 16, 16, 16, &a);
  REQUIRE(dataE);
  REQUIRE(dataA);
  for (int y = 0; y < 16; y++) {
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        auto vE = dataE->getUnchecked(origin + Pos3i{x, y, z});
        auto vA = dataA->getUnchecked(origin + Pos3i{x, y, z});
        CHECK(vE == vA);
      }
    }
  }
}

TEST_CASE("lighting") {
  fs::path root = ProjectRootDir();
  mcfile::je::World world(root / "test" / "data" / "je2be-test");
  auto expected = world.chunkAt(1, 1);
  REQUIRE(expected);
  auto chunk = world.writableChunkAt(1, 1);
  REQUIRE(chunk);
  for (auto &section : chunk->fSections) {
    section->fSkyLight.clear();
    section->fBlockLight.clear();
  }
  terraform::java::BlockAccessorJavaMca<3, 3> accessor(0, 0, *world.region(0, 0));
  accessor.set(chunk);
  toje::Lighting::Do(Dimension::Overworld, *chunk, accessor);

  REQUIRE(chunk->fSections.size() == expected->fSections.size());
  for (int i = 0; i < expected->fSections.size(); i++) {
    auto const &sectionE = expected->fSections[i];
    auto const &sectionA = chunk->fSections[i];
    Pos3i origin{expected->fChunkX * 16, sectionE->y() * 16, expected->fChunkZ * 16};
    REQUIRE(sectionE);
    REQUIRE(sectionA);
    REQUIRE(sectionE->y() == sectionA->y());
    if (!sectionE->fSkyLight.empty()) {
      CHECK(sectionE->fSkyLight.size() == sectionA->fSkyLight.size());
      CheckLight(origin, sectionE->fSkyLight, sectionA->fSkyLight);
    }
    if (!sectionE->fBlockLight.empty()) {
      CHECK(sectionE->fBlockLight.size() == sectionA->fBlockLight.size());
      CheckLight(origin, sectionE->fBlockLight, sectionA->fBlockLight);
    }
  }
}
