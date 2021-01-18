//
// Created by per on 23.12.20.
//

#include <versioning/VersioningBlockedSkipListAdjacencyList.h>

#include "SnapshotTransaction.h"
#include <iostream>

SnapshotTransaction::SnapshotTransaction(version_t version, VersionedTopologyInterface* ds)
  : version(version), ds(ds) {

}

bool SnapshotTransaction::execute() {
  aquire_locks();

  // TODO use some kind of with statement for aquire locks?
  if (assert_preconditions()) {
    // TODO check standard preconditions, e.g. I add an edge is the vertex existing?
    for (auto v: vertices_to_delete) {
      ds->delete_vertex_version(v, version);  // TODO should follow if exists
    }
    for (auto v : vertices_to_insert) {
      ds->insert_vertex_version(v, version);  // TODO should follow if not exists
    }
    for (auto e : edges_to_delete) {
      ds->delete_edge_version(e, version);   // TODO should follow if exists
    }
    auto i = 0;
    for (auto e : edges_to_insert) {
//      try {
        ds->insert_edge_version(e, version);  // TODO should follow if not exists
//        i++;
//        if (i % 1000 == 0) {
//        cout << ".";
//        cout.flush();
//        }
//      } catch (MultipleVersionException& e) {
//         NOP
//      }
    }
    cout << endl<< "done inserting" << endl;
    release_locks();
    return true;
  } else {
    release_locks();
    return false;
  }
}

void SnapshotTransaction::register_precondition(unique_ptr<Precondition> c) {
  for (vertex_id_t l : c->requires_vertex_locks()) {
    locks_to_aquire.insert(l);
  }
  preconditions.push_back(c.release());
}

bool SnapshotTransaction::assert_preconditions() {
  for (auto p: preconditions) {
    if (!p->assert_it(*ds, version)) {
      return false; // TODO should be handled with exceptions to allow for error messages?
    }
  }
  return true;
}

void SnapshotTransaction::aquire_locks() {
  for (auto & v : locks_to_aquire) {  // Relies on locks_to_aquire being a sorted data structure
    ds->aquire_vertex_lock(v);
  }
}

void SnapshotTransaction::release_locks() {
  for (auto & v : locks_to_aquire) {
    ds->release_vertex_lock(v);
  }
}

size_t SnapshotTransaction::vertex_count() {
  return ds->vertex_count_version(version);
}

void SnapshotTransaction::insert_vertex(vertex_id_t v) {
  locks_to_aquire.insert(v);
  vertices_to_insert.push_back(v);
}

void SnapshotTransaction::delete_vertex(vertex_id_t v) {
  locks_to_aquire.insert(v);
  vertices_to_delete.push_back(v);
}

void SnapshotTransaction::insert_edge(edge_t edge) {
  locks_to_aquire.insert(edge.src);
  edges_to_insert.push_back(edge);
}

void SnapshotTransaction::delete_edge(edge_t edge) {
  locks_to_aquire.insert(edge.src);
  edges_to_delete.push_back(edge);
}

size_t SnapshotTransaction::neighbourhood_size(vertex_id_t src) {
  ds->aquire_vertex_lock(src);
  auto ret = ds->neighbourhood_size_version(src, version);
  ds->release_vertex_lock(src);
  return ret;
}

void SnapshotTransaction::intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  ds->aquire_vertex_lock(min(a, b));
  ds->aquire_vertex_lock(max(a, b));
  ds->intersect_neighbourhood_version(a, b, out, version);
  ds->release_vertex_lock(a);
  ds->release_vertex_lock(b);
}

bool SnapshotTransaction::has_edge(edge_t edge) {
  ds->aquire_vertex_lock(edge.src);
  auto ret =  ds->has_edge_version(edge, version);
  ds->release_vertex_lock(edge.src);
  return ret;
}

void SnapshotTransaction::report_storage_size() {
  return ds->report_storage_size();
}

version_t SnapshotTransaction::get_version() const {
  return version;
}

SnapshotTransaction::~SnapshotTransaction() {
  for (auto p : preconditions) {
    delete p;
  }
}

void SnapshotTransaction::bulkload(const SortedCSRDataSource &src) {
  // TODO aquire all locks.
  // TODO document that we are not writing version during this process.
  ds->bulkload(src);
}

VersionedTopologyInterface *SnapshotTransaction::raw_ds() {
  return ds;
}
