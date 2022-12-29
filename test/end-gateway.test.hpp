TEST_CASE("end-gateway") {
  SUBCASE("bedrock") {
    fs::path thisFile(__FILE__);
    auto mcworld = thisFile.parent_path() / "data" / "end-gateway" / "bedrock" / "end-gateway.mcworld";
    auto tmp = mcfile::File::CreateTempDir(fs::temp_directory_path());
    defer {
      fs::remove_all(*tmp);
    };
    auto in = *tmp / "in";
    REQUIRE(ZipFile::Unzip(mcworld, in));
    auto out = *tmp / "out";
    toje::Options opt;
    opt.fDimensionFilter.insert(Dimension::End);
    opt.fChunkFilter.insert({-5, -4});
    opt.fChunkFilter.insert({-51, -39});
    toje::Converter converter(in, out, opt);
    auto st = converter.run(thread::hardware_concurrency());
    CHECK(st.ok());
    mcfile::je::World world(out / "DIM1");
    {
      auto chunk = world.chunkAt(-5, -4);
      REQUIRE(chunk);
      auto block = chunk->blockAt(-77, 75, -56);
      REQUIRE(block);
      CHECK(block->fId == mcfile::blocks::minecraft::end_gateway);
      auto tile = chunk->tileEntityAt(-77, 75, -56);
      REQUIRE(tile);
      CHECK(tile->string("id") == "minecraft:end_gateway");
      auto exitPortal = tile->compoundTag("ExitPortal");
      REQUIRE(exitPortal);
      CHECK(exitPortal->int32("X") == -814);
      CHECK(exitPortal->int32("Y") == 59);
      CHECK(exitPortal->int32("Z") == -613);
      CHECK(tile->int64("Age") == 725);
    }
    {
      auto chunk = world.chunkAt(-51, -39);
      REQUIRE(chunk);
      auto block = chunk->blockAt(-814, 69, -613);
      REQUIRE(block);
      CHECK(block->fId == mcfile::blocks::minecraft::end_gateway);
      auto tile = chunk->tileEntityAt(-814, 69, -613);
      REQUIRE(tile);
      CHECK(tile->string("id") == "minecraft:end_gateway");
      auto exitPortal = tile->compoundTag("ExitPortal");
      REQUIRE(exitPortal);
      CHECK(exitPortal->int32("X") == -74);
      CHECK(exitPortal->int32("Y") == 58);
      CHECK(exitPortal->int32("Z") == -52);
      CHECK(tile->int64("Age") == 409);
    }
  }
}
