//
// Created by per on 14.01.21.
//

#include "VertexExistsPrecondition.h"

vector<vertex_id_t> VertexExistsPrecondition::requires_vertex_locks() {
  return vector<vertex_id_t>();
}

bool VertexExistsPrecondition::assert_it(VersionedTopologyInterface &ds, version_t version) {
  return false;
}

VertexExistsPrecondition::~VertexExistsPrecondition() {
  // NOP
}
