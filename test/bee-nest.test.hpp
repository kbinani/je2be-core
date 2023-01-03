TEST_CASE("bee-nest") {
  SUBCASE("bedrock") {
    fs::path thisFile(__FILE__);
    auto mcworld = thisFile.parent_path() / "data" / "bee-nest" / "bedrock" / "bee-nest.mcworld";
    auto tmp = mcfile::File::CreateTempDir(fs::temp_directory_path());
    defer {
      fs::remove_all(*tmp);
    };
    auto in = *tmp / "in";
    REQUIRE(ZipFile::Unzip(mcworld, in));
    auto out = *tmp / "out";
    toje::Options opt;
    opt.fDimensionFilter.insert(Dimension::Overworld);
    opt.fChunkFilter.insert({0, 0});

    auto st = toje::Converter::Run(in, out, opt, thread::hardware_concurrency());
    CHECK(st.ok());
    mcfile::je::World world(out);
    auto chunk = world.chunkAt(0, 0);
    CHECK(chunk);
    auto tile = chunk->tileEntityAt(2, -60, 2);
    CHECK(tile);
    CHECK(tile->string("id") == "minecraft:beehive");
    auto bees = tile->listTag("Bees");
    CHECK(bees);
    CHECK(bees->size() == 1);
    auto bee = bees->at(0);
    CHECK(bee->asCompound());
    CHECK(bee->asCompound()->string("id") == "minecraft:bee");
  }
}
