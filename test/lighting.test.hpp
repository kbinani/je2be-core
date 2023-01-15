static void CheckLight(Pos3i const &origin, std::vector<uint8_t> &e, std::vector<uint8_t> &a) {
  auto dataE = mcfile::Data4b3dView::Make(origin, 16, 16, 16, &e);
  auto dataA = mcfile::Data4b3dView::Make(origin, 16, 16, 16, &a);
  REQUIRE(dataE);
  REQUIRE(dataA);
#if 1
  int y = 0;
  printf("   ");
  for (int x = 0; x < 16; x++) {
    printf("%6d ", x + origin.fX);
  }
  cout << endl;
  for (int z = 0; z < 16; z++) {
    printf("%2d:", z + origin.fZ);
    for (int x = 0; x < 16; x++) {
      int vE = dataE->getUnchecked(origin + Pos3i{x, y, z});
      int vA = dataA->getUnchecked(origin + Pos3i{x, y, z});
      string bra = "[";
      string ket = "]";
      if (vE == vA) {
        bra = " ";
        ket = " ";
      }
      printf("%s%2d,%2d%s", bra.c_str(), vE, vA, ket.c_str());
    }
    cout << endl;
  }
#else
  for (int y = 0; y < 16; y++) {
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        auto vE = dataE->getUnchecked(origin + Pos3i{x, y, z});
        auto vA = dataA->getUnchecked(origin + Pos3i{x, y, z});
        CHECK(vE == vA);
        if (vE != vA) {
          //   cout << "[" << (origin.fX + x) << "," << (origin.fY + y) << "," << (origin.fZ + z) << "] " << (int)vE << " => " << (int)vA << endl;
        }
      }
    }
  }
#endif
}

TEST_CASE("lighting") {
  fs::path dir = ProjectRootDir() / "test" / "data" / "je2be-test";
  //fs::path dir("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/lighting");
  mcfile::je::World world(dir);
  int cx = 0;
  int cz = 0;
  auto expected = world.chunkAt(cx, cz);
  REQUIRE(expected);
  auto chunk = world.writableChunkAt(cx, cz);
  REQUIRE(chunk);
  for (auto &section : chunk->fSections) {
    section->fSkyLight.clear();
    section->fBlockLight.clear();
  }
  terraform::java::BlockAccessorJavaDirectory<3, 3> accessor(cx - 1, cz - 1, dir);
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
      //CheckLight(origin, sectionE->fSkyLight, sectionA->fSkyLight);
    }
    if (!sectionE->fBlockLight.empty()) {
      CHECK(sectionE->fBlockLight.size() == sectionA->fBlockLight.size());
      if (sectionE->y() == 4) {
        CheckLight(origin, sectionE->fBlockLight, sectionA->fBlockLight);
      }
    }
  }
}
