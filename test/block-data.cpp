#include <doctest/doctest.h>
#include <je2be.hpp>

#include <iostream>

using namespace std;
using namespace mcfile;
using namespace mcfile::je;
namespace fs = std::filesystem;

static void CheckTag(mcfile::nbt::Tag const *va, mcfile::nbt::Tag const *vb) {
  using namespace mcfile::nbt;
  CHECK(va->type() == vb->type());
  switch (va->type()) {
  case Tag::Type::Byte:
    CHECK(va->asByte()->fValue == vb->asByte()->fValue);
    break;
  case Tag::Type::Double:
    CHECK(va->asDouble()->fValue == vb->asDouble()->fValue);
    break;
  case Tag::Type::Float:
    CHECK(va->asFloat()->fValue == vb->asFloat()->fValue);
    break;
  case Tag::Type::Int:
    CHECK(va->asInt()->fValue == vb->asInt()->fValue);
    break;
  case Tag::Type::Long:
    CHECK(va->asLong()->fValue == vb->asLong()->fValue);
    break;
  case Tag::Type::Short:
    CHECK(va->asShort()->fValue == vb->asShort()->fValue);
    break;
  case Tag::Type::String:
    CHECK(va->asString()->fValue == vb->asString()->fValue);
    break;
  case Tag::Type::ByteArray: {
    auto const &la = va->asByteArray()->value();
    auto const &lb = vb->asByteArray()->value();
    CHECK(la.size() == lb.size());
    for (int i = 0; i < la.size(); i++) {
      CHECK(la[i] == lb[i]);
    }
    break;
  }
  case Tag::Type::IntArray: {
    auto const &la = va->asIntArray()->value();
    auto const &lb = vb->asIntArray()->value();
    CHECK(la.size() == lb.size());
    for (int i = 0; i < la.size(); i++) {
      CHECK(la[i] == lb[i]);
    }
    break;
  }
  case Tag::Type::LongArray: {
    auto const &la = va->asLongArray()->value();
    auto const &lb = vb->asLongArray()->value();
    CHECK(la.size() == lb.size());
    for (int i = 0; i < la.size(); i++) {
      CHECK(la[i] == lb[i]);
    }
    break;
  }
  case Tag::Type::List: {
    auto la = va->asList();
    auto lb = vb->asList();
    CHECK(la->size() == lb->size());
    for (int i = 0; i < la->size(); i++) {
      CheckTag(la->at(i).get(), lb->at(i).get());
    }
    break;
  }
  case Tag::Type::Compound: {
    auto ca = va->asCompound();
    auto cb = vb->asCompound();
    CHECK(ca->size() == cb->size());
    for (auto it : *ca) {
      auto a = it.second;
      auto found = cb->find(it.first);
      CHECK(found != cb->end());
      auto b = found->second;
      CheckTag(a.get(), b.get());
    }
    break;
  }
  default:
    CHECK(false);
    break;
  }
}

TEST_CASE("block-data") {
  fs::path thisFile(__FILE__);
  fs::path dataDir = thisFile.parent_path() / "data";
  for (auto it : fs::recursive_directory_iterator(dataDir / "block-data" / "1.18.1")) {
    auto path = it.path();
    if (!fs::is_regular_file(path)) {
      continue;
    }
    fs::path filename = path.filename();
    string javaBlockData = string("minecraft:") + filename.replace_extension().string();

    auto fis = make_shared<mcfile::stream::FileInputStream>(path);
    mcfile::stream::InputStreamReader isr(fis);
    auto tag = make_shared<mcfile::nbt::CompoundTag>();
    CHECK(tag->read(isr));
    shared_ptr<mcfile::nbt::CompoundTag> bedrockBlockData = tag->compoundTag("");
    CHECK(bedrockBlockData);

    // java -> bedrock
    auto block = mcfile::je::Block::FromBlockData(javaBlockData, 2865);
    CHECK(block);
    auto convertedToBe = je2be::tobe::BlockData::From(block);
    CheckTag(convertedToBe.get(), bedrockBlockData.get());

    // bedrock -> java
    auto convertedToJe = je2be::toje::BlockData::From(*bedrockBlockData);
    //TODO: CHECK(convertedToJe);
    //TODO: CHECK(convertedToJe->toString() == javaBlockData);
  }
}

#if 0
TEST_CASE("prepare-test-data") {
  fs::path thisFile(__FILE__);
  fs::path dataDir = thisFile.parent_path() / "data";
  auto file = dataDir / "debug-mode" / "1.18.1" / "r.0.0.mca";
  auto region = Region::MakeRegion(file);

  fs::create_directories(dataDir / "block-data" / "1.18.1");

  for (int cz = region->minChunkZ(); cz <= region->maxChunkZ(); cz++) {
    for (int cx = region->minChunkX(); cx <= region->maxChunkX(); cx++) {
      auto chunk = region->chunkAt(cx, cz);
      if (!chunk) {
        continue;
      }
      for (auto const &section : chunk->fSections) {
        section->eachBlockPalette([dataDir](Block const &b) {
          string s = b.toString().substr(string("minecraft:").size());
          fs::path dir = dataDir / "block-data" / "1.18.1" / s.substr(0, 1);
          fs::create_directories(dir);
          fs::path nbt = dir / (s + ".nbt");
          if (fs::exists(nbt)) {
            return true;
          }
          auto converted = je2be::tobe::BlockData::From(make_shared<Block const>(b.fName, b.fProperties));
          CHECK(converted != nullptr);
          auto fos = make_shared<mcfile::stream::FileOutputStream>(nbt);
          mcfile::stream::OutputStreamWriter osw(fos);
          CHECK(converted->writeAsRoot(osw));
          return true;
        });
      }
    }
  }
}
#endif
