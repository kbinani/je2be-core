#pragma once

#include <doctest/doctest.h>
#include <je2be.hpp>

class CheckTag {
public:
  static void Check(mcfile::nbt::Tag const *va, mcfile::nbt::Tag const *vb) {
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
        Check(la->at(i).get(), lb->at(i).get());
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
        Check(a.get(), b.get());
      }
      break;
    }
    default:
      CHECK(false);
      break;
    }
  }

private:
  CheckTag() = delete;
};
