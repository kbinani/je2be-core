#pragma once

namespace je2be {

using CompoundTag = mcfile::nbt::CompoundTag;
using ByteArrayTag = mcfile::nbt::ByteArrayTag;
using ByteTag = mcfile::nbt::ByteTag;
using DoubleTag = mcfile::nbt::DoubleTag;
using FloatTag = mcfile::nbt::FloatTag;
using IntArrayTag = mcfile::nbt::IntArrayTag;
using IntTag = mcfile::nbt::IntTag;
using ListTag = mcfile::nbt::ListTag;
using LongArrayTag = mcfile::nbt::LongArrayTag;
using LongTag = mcfile::nbt::LongTag;
using ShortTag = mcfile::nbt::ShortTag;
using StringTag = mcfile::nbt::StringTag;
using Tag = mcfile::nbt::Tag;

namespace nbt {

static inline std::shared_ptr<ByteTag> Bool(bool b) {
  return std::make_shared<ByteTag>(b ? 1 : 0);
}

static inline std::shared_ptr<ByteTag> Byte(int8_t v) {
  uint8_t t = *(uint8_t *)&v;
  return std::make_shared<ByteTag>(t);
}

static inline std::shared_ptr<IntTag> Int(int32_t v) {
  return std::make_shared<IntTag>(v);
}

static inline std::shared_ptr<LongTag> Long(int64_t v) {
  return std::make_shared<LongTag>(v);
}

static inline std::shared_ptr<StringTag> String(std::string v) {
  return std::make_shared<StringTag>(v);
}

static inline std::shared_ptr<FloatTag> Float(float v) {
  return std::make_shared<FloatTag>(v);
}

static inline std::shared_ptr<DoubleTag> Double(double v) {
  return std::make_shared<DoubleTag>(v);
}

static inline std::shared_ptr<ShortTag> Short(int16_t v) {
  return std::make_shared<ShortTag>(v);
}

static inline std::shared_ptr<CompoundTag> Compound() {
  return std::make_shared<CompoundTag>();
}

template <Tag::Type type>
static inline std::shared_ptr<ListTag> List() {
  return std::make_shared<ListTag>(type);
};

} // namespace nbt

} // namespace je2be
