#pragma once

TEST_CASE("shoulder-riders") {
  fs::path thisFile(__FILE__);

  // The situation is like this: https://gyazo.com/fe4669d4e655e80282484d961a17b167
  auto original = thisFile.parent_path() / "data" / "shoulder-riders";
  auto tmp = mcfile::File::CreateTempDir(fs::temp_directory_path());

  defer {
    fs::remove_all(*tmp);
  };

  auto be = *tmp / "be";
  fs::create_directories(be);
  je2be::tobe::Options optToBe;
  optToBe.fDimensionFilter.insert(mcfile::Dimension::Overworld);
  optToBe.fChunkFilter.insert(Pos2i(0, 0));
  je2be::tobe::Converter tobe(original, be, optToBe);
  CHECK(tobe.run(1).ok());

  auto je = *tmp / "je";
  fs::create_directories(je);
  je2be::toje::Options optToJe;
  optToJe.fDimensionFilter.insert(mcfile::Dimension::Overworld);
  optToJe.fChunkFilter.insert(Pos2i(0, 0));
  je2be::toje::Converter toje(be, je, optToJe);
  CHECK(toje.run(1).ok());

  auto level = je / "level.dat";
  auto stream = make_shared<mcfile::stream::GzFileInputStream>(level);
  auto dat = CompoundTag::Read(stream, Endian::Big);
  auto data = dat->compoundTag("Data");
  auto player = data->compoundTag("Player");
  CHECK(player);

  CHECK(player->compoundTag("ShoulderEntityLeft"));
  CHECK(player->compoundTag("ShoulderEntityRight"));
  CHECK(player->compoundTag("RootVehicle"));
}
