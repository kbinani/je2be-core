#pragma once

#include "java/structure/_structure-piece-collection.hpp"

namespace je2be::java {

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

  [[nodiscard]] Status put(DbInterface &db) {
    if (auto st = fOverworld.put(db, mcfile::Dimension::Overworld); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    if (auto st = fNether.put(db, mcfile::Dimension::Nether); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    if (auto st = fEnd.put(db, mcfile::Dimension::End); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    return Status::Ok();
  }

private:
  StructurePieceCollection fOverworld;
  StructurePieceCollection fNether;
  StructurePieceCollection fEnd;
};

} // namespace je2be::java
