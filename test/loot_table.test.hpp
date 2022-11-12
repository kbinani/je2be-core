#pragma once

TEST_CASE("loot_table") {
  SUBCASE("j2b2j") {
    auto java = "minecraft:chests/village/village_plains_house";
    auto bedrock = "loot_tables/chests/village/village_plains_house.json";
    CHECK(LootTable::BedrockTableNameFromJava(java) == bedrock);
    CHECK(LootTable::JavaTableNameFromBedrock(bedrock) == java);
  }
#if 0
  SUBCASE("bedrock") {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;
    using namespace mcfile::stream;
    using namespace mcfile::be;
    using namespace mcfile::nbt;

    unique_ptr<DB> db(Open("fgdwYwGrHgI=")); //"vXxvY4enAAA="));
    REQUIRE(db);
    unique_ptr<Iterator> itr(db->NewIterator({}));
    REQUIRE(itr);
    set<string> lootTables;

    for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
      string key = itr->key().ToString();
      auto parsed = mcfile::be::DbKey::Parse(key);
      if (!parsed) {
        continue;
      }
      if (!parsed->fIsTagged) {
        continue;
      }
      switch (static_cast<DbKey::Tag>(parsed->fTagged.fTag)) {
      case DbKey::Tag::BlockEntity:
        CompoundTag::ReadUntilEos(itr->value().ToString(), Endian::Little, [&lootTables](shared_ptr<CompoundTag> const &tag) {
          auto lootTable = tag->string("LootTable");
          if (!lootTable) {
            return;
          }
          if (lootTables.find(*lootTable) == lootTables.end()) {
            lootTables.insert(*lootTable);
            cout << "--" << endl;
            for (string const &t : lootTables) {
              cout << t << endl;
            }
          }
        });
        break;
      }
    }
  }
#endif
}
