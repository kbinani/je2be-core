TEST_CASE("pos2i-set") {
  SUBCASE("basic") {
    Pos2iSet v;
    CHECK(v.empty());
    v.insert({0, 0});
    v.insert({2, 0});
    v.insert({1, 0});
    v.insert({1, 0});
    CHECK(v.find({0, 0}) != v.end());
    CHECK(v.count({0, 0}) == 1);
    CHECK(v.find({2, 0}) != v.end());
    CHECK(v.count({2, 0}) == 1);
    CHECK(v.find({1, 0}) != v.end());
    CHECK(v.count({1, 0}) == 1);
    CHECK(v.size() == 3);
    CHECK(v.find({100, 100}) == v.end());
    CHECK(v.count({100, 100}) == 0);

    v.insert({5, 0});
    CHECK(v.size() == 4);
    v.insert({3, 0});
    CHECK(v.size() == 5);
    v.insert({8, 0});
    CHECK(v.size() == 6);
    v.insert({7, 0});
    CHECK(v.size() == 7);
    v.insert({9, 0});
    CHECK(v.size() == 8);
    v.insert({13, 0});
    CHECK(v.size() == 9);
    v.insert({11, 0});
    CHECK(v.size() == 10);

    unordered_set<Pos2i, Pos2iHasher> all;
    for (auto const &p : v) {
      all.insert(p);
    }
    CHECK(all.size() == v.size());
    for (auto const &p : all) {
      CHECK(v.count(p) == 1);
    }
  }
  SUBCASE("random") {
    random_device rd;
    mt19937 engine(rd());
    i32 min = -1024;
    i32 max = 1024;
    uniform_int_distribution<int32_t> dist(min, max);
    Pos2iSet v;
    unordered_set<Pos2i, Pos2iHasher> expected;
    for (int i = 0; i < 1024; i++) {
      i32 x = dist(engine);
      i32 z = dist(engine);
      expected.insert({x, z});
      v.insert({x, z});
    }
    CHECK(expected.size() == v.size());
    for (int x = min; x <= max; x++) {
      for (int z = min; z <= max; z++) {
        if (expected.count({x, z}) == 0) {
          CHECK(v.find({x, z}) == v.end());
        } else {
          CHECK(v.find({x, z}) != v.end());
        }
      }
    }
    unordered_set<Pos2i, Pos2iHasher> actual;
    for (Pos2i a : v) {
      actual.insert(a);
    }
    for (auto const &p : actual) {
      CHECK(expected.count(p) == 1);
    }
  }
}
