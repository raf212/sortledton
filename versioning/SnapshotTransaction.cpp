//
// Created by per on 23.12.20.
//

#include <versioning/VersioningBlockedSkipListAdjacencyList.h>

#include "SnapshotTransaction.h"
#include <iostream>

SnapshotTransaction::SnapshotTransaction(version_t version, VersionedTopologyInterface *ds)
        : version(version), ds(ds) {

}

bool SnapshotTransaction::execute() {
  try {
    aquire_locks_and_insert_vertices();

    assert_preconditions();
    assert_std_preconditions();
    for (auto v: vertices_to_delete) {
      if (vertex_does_not_exists_semantic_activated && !ds->has_vertex_version(v, version)) {
        continue;
      }
      ds->delete_vertex_version(v, version);
    }
    for (auto e : edges_to_delete) {
      edge_t p_edge (ds->physical_id(e.src), ds->physical_id(e.dst));
      if (edge_does_not_exists_semantic_activated && !ds->has_edge_version_p(p_edge, version)) {
        continue;
      }
      ds->delete_edge_version(p_edge, version);
    }
    auto i = 0;
    for (auto e : edges_to_insert) {
      edge_t p_edge (ds->physical_id(e.src), ds->physical_id(e.dst));

//      try {
      if (edge_does_not_exists_semantic_activated && ds->has_edge_version_p(p_edge, version)) {
        continue;
      }
      ds->insert_edge_version(p_edge, version);
//        i++;
//        if (i % 1000 == 0) {
//        cout << ".";
//        cout.flush();
//        }
//      } catch (MultipleVersionException& e) {
//         NOP
//      }
    }
//    cout << endl<< "done inserting" << endl;
    release_locks();
    return true;
  } catch (exception &e) {
    cout << "rolling back" << endl;
    rollback();
    release_locks();
    throw e;
  }
}

void SnapshotTransaction::register_precondition(Precondition *c) {
  vertex_id_t lock_to_aquire = c->requires_vertex_lock();
  if (lock_to_aquire != numeric_limits<vertex_id_t>::max()) {
    locks_to_aquire.push_back(lock_to_aquire);
  } else {
    for (vertex_id_t l : c->requires_vertex_locks()) {
      locks_to_aquire.push_back(l);
    }
  }
  preconditions.push_back(c);
}

void SnapshotTransaction::assert_preconditions() {
  for (auto p: preconditions) {
    p->assert_it(*ds, version);
  }
}

void SnapshotTransaction::aquire_locks_and_insert_vertices() {
  sort(locks_to_aquire.begin(), locks_to_aquire.end());
  vertex_id_t last_lock = numeric_limits<vertex_id_t>::max();
  for (const auto &v : locks_to_aquire) {  // Relies on locks_to_aquire being a sorted data structure
    if (v != last_lock) {  // Dedup locks
      if (!ds->aquire_vertex_lock(v)) {
        if (find(vertices_to_insert.begin(), vertices_to_insert.end(), v) != vertices_to_insert.end()) {
          last_lock_aquired = v;
          if(ds->insert_vertex_version(v, version)) {
            rollbacks.push_back(RollbackAction::generate_rollback_insert_vertex(v));
          } else if (!vertex_does_not_exists_semantic_activated ) {
            throw VertexExistsException(v);
          }
        } else {
          throw VertexDoesNotExistsException(v);
        }
      } else {
        last_lock_aquired = v;
      }
      last_lock = v;
    }
  }
}

void SnapshotTransaction::release_locks() {
  vertex_id_t last_lock = numeric_limits<vertex_id_t>::max();
  for (auto &v : locks_to_aquire) {  // Relies on locks_to_aquire being a sorted data structure
    if (v != last_lock && v <= last_lock_aquired) { // Dedup locks && do not release locks which have not been aquired.
        ds->release_vertex_lock(v);
        last_lock = v;
    } else if (v > last_lock_aquired) {
      cout << "did not release lock " << v << endl;
    }
  }
}

size_t SnapshotTransaction::vertex_count() {
  return ds->vertex_count_version(version);
}

bool SnapshotTransaction::insert_vertex(vertex_id_t v) {
  locks_to_aquire.push_back(v);
  vertices_to_insert.push_back(v);
  return false;
}

bool SnapshotTransaction::delete_vertex(vertex_id_t v) {
  locks_to_aquire.push_back(v);
  vertices_to_delete.push_back(v);
  return false;
}

bool SnapshotTransaction::insert_edge(edge_t edge) {
  locks_to_aquire.push_back(edge.src);
  edges_to_insert.push_back(edge);
  return false;
}

bool SnapshotTransaction::delete_edge(edge_t edge) {
  locks_to_aquire.push_back(edge.src);
  edges_to_delete.push_back(edge);
  return false;
}

size_t SnapshotTransaction::neighbourhood_size_p(vertex_id_t src) {
  ds->aquire_vertex_lock_p(src);
  auto ret = ds->neighbourhood_size_version_p(src, version);
  ds->release_vertex_lock_p(src);
  return ret;
}

void SnapshotTransaction::intersect_neighbourhood_p(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  ds->aquire_vertex_lock_p(min(a, b));
  ds->aquire_vertex_lock_p(max(a, b));
  ds->intersect_neighbourhood_version_p(a, b, out, version);
  ds->release_vertex_lock_p(a);
  ds->release_vertex_lock_p(b);
}

bool SnapshotTransaction::has_edge_p(edge_t edge) {
  ds->aquire_vertex_lock_p(edge.src);
  auto ret = ds->has_edge_version_p(edge, version);
  ds->release_vertex_lock_p(edge.src);
  return ret;
}

void SnapshotTransaction::report_storage_size() {
  return ds->report_storage_size();
}

version_t SnapshotTransaction::get_version() const {
  return version;
}

SnapshotTransaction::~SnapshotTransaction() {
//  for (auto p : preconditions) {
//    delete p;
//  }
}

void SnapshotTransaction::bulkload(const SortedCSRDataSource &src) {
  for (auto v = 0; v < ds->vertex_count_version(version); v++) {
    ds->aquire_vertex_lock(v);
  }
  ds->bulkload(src);
  for (auto v = 0; v < ds->vertex_count_version(version); v++) {
    ds->release_vertex_lock(v);
  }
}

VersionedTopologyInterface *SnapshotTransaction::raw_ds() {
  return ds;
}

void SnapshotTransaction::clear() {
  last_lock_aquired = 0;
  preconditions.clear();
  locks_to_aquire.clear();
  vertices_to_insert.clear();
  vertices_to_delete.clear();
  edges_to_insert.clear();
  edges_to_delete.clear();
}

void SnapshotTransaction::set_version(version_t v) {
  version = v;
}

void SnapshotTransaction::neighbourhood_p(vertex_id_t src, EdgeIterator &iter) {
  ds->neighbourhood_version_p(src, iter, version);
}

bool SnapshotTransaction::has_vertex_p(vertex_id_t v) {
  ds->aquire_vertex_lock_p(v);
  bool ret = ds->has_vertex_version_p(v, version);
  ds->release_vertex_lock_p(v);
  return ret;
}

size_t SnapshotTransaction::edge_count() {
  return ds->edge_count_version(version);
}

void SnapshotTransaction::use_vertex_does_not_exists_semantics() {
  vertex_does_not_exists_semantic_activated = true;
}

void SnapshotTransaction::use_edge_does_not_exists_semantics() {
  edge_does_not_exists_semantic_activated = true;
}

vertex_id_t SnapshotTransaction::physical_id(vertex_id_t v) {
  return ds->physical_id(v);
}

vertex_id_t SnapshotTransaction::logical_id(vertex_id_t v) {
  return ds->logical_id(v);
}

void SnapshotTransaction::assert_std_preconditions() {

  // Vertices of each edge to insert need to exists.
  for (auto e : edges_to_insert) {
    // TODO is that to slow?
    if (!ds->has_vertex_version(e.src, version) && find(vertices_to_insert.begin(), vertices_to_insert.end(), e.src) == vertices_to_insert.end()) {
      throw VertexDoesNotExistsException(e.src);
    }
    if (!ds->has_vertex_version(e.dst, version) && find(vertices_to_insert.begin(), vertices_to_insert.end(), e.src) == vertices_to_insert.end()) {
      throw VertexDoesNotExistsException(e.dst);
    }
  }

  if (!vertex_does_not_exists_semantic_activated) {
    // Vertices to delete have to exists
    for (auto v : vertices_to_delete) {
      if (!ds->has_vertex_version(v, version)) {
        throw VertexDoesNotExistsException(v);
      }
    }
  }

  if (!edge_does_not_exists_semantic_activated) {
    // New edges cannot exist already
    for (auto e : edges_to_insert) {
      if (ds->has_edge_version(e, version)) {
        throw EdgeExistsException(e);
      }
    }

    // Edges to delete have to exists
    for (auto e : edges_to_delete) {
      if (!ds->has_edge_version(e, version)) {
        throw EdgeDoesNotExistsException(e);
      }
    }
  }
}

void SnapshotTransaction::rollback() {
  for (auto rb : rollbacks) {
    switch (rb.type) {
      case (RollbackAction::INSERT_VERTEX): {
        ds->rollback_vertex_insert(rb.vertex);
      }
      default: {
        throw NotImplemented();
      }
    }
  }
}
