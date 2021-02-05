//
// Created by per on 14.01.21.
//

#include "VertexExistsPrecondition.h"

vector<vertex_id_t> VertexExistsPrecondition::requires_vertex_locks() {
  return vector<vertex_id_t>(1, v);
}

bool VertexExistsPrecondition::assert_it(VersionedTopologyInterface &ds, version_t version) {
  return v < ds.vertex_count_version(version);
}

VertexExistsPrecondition::VertexExistsPrecondition(vertex_id_t v) : v(v) {

}

VertexExistsPrecondition::~VertexExistsPrecondition() {
  // NOP
}

vertex_id_t VertexExistsPrecondition::requires_vertex_lock() {
  return v;
}
