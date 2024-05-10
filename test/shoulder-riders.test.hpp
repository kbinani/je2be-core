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
  je2be::java::Options optToBe;
  optToBe.fDimensionFilter.insert(mcfile::Dimension::Overworld);
  optToBe.fChunkFilter.insert(Pos2i(0, 0));
  CHECK(je2be::java::Converter::Run(original, be, optToBe, 1).ok());

  auto je = *tmp / "je";
  fs::create_directories(je);
  je2be::bedrock::Options optToJe;
  optToJe.fDimensionFilter.insert(mcfile::Dimension::Overworld);
  optToJe.fChunkFilter.insert(Pos2i(0, 0));
  CHECK(je2be::bedrock::Converter::Run(be, je, optToJe, 1).ok());

  auto level = je / "level.dat";
  auto stream = make_shared<mcfile::stream::GzFileInputStream>(level);
  auto dat = CompoundTag::Read(stream, Encoding::Java);
  auto data = dat->compoundTag(u8"Data");
  auto player = data->compoundTag(u8"Player");
  CHECK(player);

  CHECK(player->compoundTag(u8"ShoulderEntityLeft"));
  CHECK(player->compoundTag(u8"ShoulderEntityRight"));
  CHECK(player->compoundTag(u8"RootVehicle"));
}
