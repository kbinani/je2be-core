TEST_CASE("system") {
  CHECK(System::GetInstalledMemory() > 0);
  CHECK(System::GetAvailableMemory() > 0);
}
