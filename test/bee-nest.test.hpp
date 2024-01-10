TEST_CASE("bee-nest") {
  SUBCASE("bedrock") {
    fs::path thisFile(__FILE__);
    auto mcworld = thisFile.parent_path() / "data" / "bee-nest" / "bedrock" / "bee-nest.mcworld";
    auto tmp = mcfile::File::CreateTempDir(fs::temp_directory_path());
    defer {
      fs::remove_all(*tmp);
    };
    auto in = *tmp / "in";
    REQUIRE(ZipFile::Unzip(mcworld, in).ok());
    auto out = *tmp / "out";
    bedrock::Options opt;
    opt.fDimensionFilter.insert(Dimension::Overworld);
    opt.fChunkFilter.insert({0, 0});

    auto st = bedrock::Converter::Run(in, out, opt, thread::hardware_concurrency());
    CHECK(st.ok());
    mcfile::je::World world(out);
    auto chunk = world.chunkAt(0, 0);
    REQUIRE(chunk);
    auto tile = chunk->tileEntityAt(2, -60, 2);
    REQUIRE(tile);
    CHECK(tile->string(u8"id") == u8"minecraft:beehive");
    auto bees = tile->listTag(u8"Bees");
    REQUIRE(bees);
    CHECK(bees->size() == 1);
    auto bee = bees->at(0);
    REQUIRE(bee->asCompound());
    auto entityData = bee->asCompound()->compoundTag(u8"EntityData");
    REQUIRE(entityData);
    CHECK(entityData->string(u8"id") == u8"minecraft:bee");
  }
}
