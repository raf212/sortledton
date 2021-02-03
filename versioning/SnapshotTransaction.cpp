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

  try {
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
//    cout << endl<< "done inserting" << endl;
      release_locks();
      return true;
    } else {
      release_locks();
      return false;  // TODO remove return code. Preconditions should throw
    }
  } catch (exception& e) {
    release_locks();
    throw e;
  }
}

void SnapshotTransaction::register_precondition(Precondition* c) {
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

bool SnapshotTransaction::assert_preconditions() {
  for (auto p: preconditions) {
    if (!p->assert_it(*ds, version)) {
      return false; // TODO should be handled with exceptions to allow for error messages?
    }
  }
  return true;
}

void SnapshotTransaction::aquire_locks() {
  sort(locks_to_aquire.begin(), locks_to_aquire.end());
  vertex_id_t last_lock = numeric_limits<vertex_id_t>::max();
  for (const auto & v : locks_to_aquire) {  // Relies on locks_to_aquire being a sorted data structure
    if (v != last_lock) {
      ds->aquire_vertex_lock(v);
      last_lock = v;
    }
  }
}

void SnapshotTransaction::release_locks() {
  vertex_id_t last_lock = numeric_limits<vertex_id_t>::max();
  for (auto & v : locks_to_aquire) {  // Relies on locks_to_aquire being a sorted data structure
    if (v != last_lock) {
      ds->release_vertex_lock(v);
      last_lock = v;
    }
  }
}

size_t SnapshotTransaction::vertex_count() {
  return ds->vertex_count_version(version);
}

void SnapshotTransaction::insert_vertex(vertex_id_t v) {
  locks_to_aquire.push_back(v);
  vertices_to_insert.push_back(v);
}

void SnapshotTransaction::delete_vertex(vertex_id_t v) {
  locks_to_aquire.push_back(v);
  vertices_to_delete.push_back(v);
}

void SnapshotTransaction::insert_edge(edge_t edge) {
  locks_to_aquire.push_back(edge.src);
  edges_to_insert.push_back(edge);
}

void SnapshotTransaction::delete_edge(edge_t edge) {
  locks_to_aquire.push_back(edge.src);
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

void SnapshotTransaction::neighbourhood(vertex_id_t src, EdgeIterator &iter) {
  ds->neighbourhood_version(src, iter, version);
}
