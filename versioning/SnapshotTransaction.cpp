//
// Created by per on 23.12.20.
//

#include "SnapshotTransaction.h"

SnapshotTransaction::SnapshotTransaction(version_t version, VersionedTopologyInterface &ds)
  : version(version), ds(ds) {

}

bool SnapshotTransaction::execute() {
  aquire_locks();

  // TODO use some kind of with statement for aquire locks?
  if (assert_preconditions()) {
    // TODO check standard preconditions, e.g. I add an edge is the vertex existing?

    for (auto v: vertices_to_delete) {
      ds.delete_vertex_version(v, version);  // TODO should follow if exists
    }
    for (auto v : vertices_to_insert) {
      ds.insert_vertex_version(v, version);  // TODO should follow if not exists
    }
    for (auto e : edges_to_delete) {
      ds.delete_edge_version(e, version);   // TODO should follow if exists
    }
    for (auto e : edges_to_insert) {
      ds.insert_edge_version(e, version);  // TODO should follow if not exists
    }
    release_locks();
    return true;
  } else {
    release_locks();
    return false;
  }
}

void SnapshotTransaction::register_precondition(unique_ptr<Precondition> c) {
  for (vertex_id_t l : c->requires_vertex_locks()) {
    locks_to_aquire.push_back(l);
  }
  preconditions.push_back(c.release());
}

bool SnapshotTransaction::assert_preconditions() {
  for (auto p: preconditions) {
    if (!p->assert_it(ds, version)) {
      return false; // TODO should be handled with exceptions to allow for error messages?
    }
  }
  return true;
}

void SnapshotTransaction::aquire_locks() {
  sort(locks_to_aquire.begin(), locks_to_aquire.end());
  for (auto & v : locks_to_aquire) {
    ds.aquire_vertex_lock(v);
  }
}

void SnapshotTransaction::release_locks() {
  for (auto & v : locks_to_aquire) {
    ds.release_vertex_lock(v);
  }

}

size_t SnapshotTransaction::vertex_count() {
  return ds.vertex_count_version(version);
}

void SnapshotTransaction::insert_vertex(vertex_id_t v) {
  vertices_to_insert.push_back(v);
}

void SnapshotTransaction::delete_vertex(vertex_id_t v) {
  vertices_to_delete.push_back(v);
}

void SnapshotTransaction::insert_edge(edge_t edge) {
  edges_to_insert.push_back(edge);
}

void SnapshotTransaction::delete_edge(edge_t edge) {
  edges_to_delete.push_back(edge);
}

size_t SnapshotTransaction::neighbourhood_size(vertex_id_t src) {
  return ds.neighbourhood_size_version(src, version);
}

void *SnapshotTransaction::raw_neighbourhood(vertex_id_t src) {
  return ds.raw_neighbourhood_version(src, version);
}

void SnapshotTransaction::intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  return ds.intersect_neighbourhood_version(a, b, out, version);
}

bool SnapshotTransaction::has_edge(edge_t edge) {
  return ds.has_edge_version(edge, version);
}

void SnapshotTransaction::report_storage_size() {
  return ds.report_storage_size();
}

version_t SnapshotTransaction::get_version() {
  return version;
}

SnapshotTransaction::~SnapshotTransaction() {
  for (auto p : preconditions) {
    delete p;
  }
}
