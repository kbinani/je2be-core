#pragma once

#include <je2be/tobe/structure/_structure-piece-collection.hpp>

namespace je2be::tobe {

class Structures {
public:
  void add(StructurePiece p, mcfile::Dimension dim) {
    switch (dim) {
    case mcfile::Dimension::Overworld:
      fOverworld.add(p);
      break;
    case mcfile::Dimension::Nether:
      fNether.add(p);
      break;
    case mcfile::Dimension::End:
      fEnd.add(p);
      break;
    }
  }

  [[nodiscard]] bool put(DbInterface &db) {
    if (!fOverworld.put(db, mcfile::Dimension::Overworld)) {
      return false;
    }
    if (!fNether.put(db, mcfile::Dimension::Nether)) {
      return false;
    }
    if (!fEnd.put(db, mcfile::Dimension::End)) {
      return false;
    }
    return true;
  }

private:
  StructurePieceCollection fOverworld;
  StructurePieceCollection fNether;
  StructurePieceCollection fEnd;
};

} // namespace je2be::tobe
