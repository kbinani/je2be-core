TEST_CASE("uuid") {
  std::string const input("49b21cb8-100b-44fa-96bd-6b3034283d37");

  auto uuid = Uuid::FromString(input);
  CHECK(uuid);

  auto intArray = uuid->toIntArrayTag();
  auto const &values = intArray->value();
  CHECK(values.size() == 4);
  CHECK(values[0] == 1236409528);
  CHECK(values[1] == 269174010);
  CHECK(values[2] == -1765971152);
  CHECK(values[3] == 875052343);

  auto uuid2 = Uuid::FromIntArrayTag(*intArray);
  auto values2 = uuid2->toIntArrayTag()->value();
  CHECK(values2[0] == values[0]);
  CHECK(values2[1] == values[1]);
  CHECK(values2[2] == values[2]);
  CHECK(values2[3] == values[3]);
  CHECK(uuid2->toString() == input);
}
