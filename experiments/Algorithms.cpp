//
// Created by per on 09.10.20.
//

#include "Algorithms.h"
#include "TwoNeighbour.h"
#include "PageRank.h"

#include <algorithm>
#include <queue>
#include <omp.h>

#include <HashSetSimulatorAdjacencyList.h>
#include <MallocAdjacencyLists.h>
#include <CSR.h>
#include <CSRMallocAdjacencyLists.h>
#include <BlockedSkipListAdjacencyLists.h>
#include <versioning/SnapshotTransaction.h>
#include <versioning/VersioningBlockedSkipListAdjacencyList.h>
#include <data_types.h>
#include "GAPBSAlgorithms.h"


vector<uint> Algorithms::bfs_batched_interface(Driver &driver, TopologyInterface &ds, vertex_id_t start_vertex) {
  size_t vertices_traversed = 0;

  uint maxDistance = numeric_limits<uint>::max();
  vector<uint> distances(ds.max_physical_vertex(), numeric_limits<uint>::max());
  distances[start_vertex] = 0;
  queue<vertex_id_t> work;
  work.push(start_vertex);

  // TODO fix start vertex distance to 0

  ContigiousBlockIterator &iter = driver.getIter(ds); // TODO move getIter to data structure instead of driver
  while (!work.empty()) {
    vertex_id_t v = work.front();
    work.pop();

    vertices_traversed++;

    ds.neighbourhood(v, iter);
    while (iter.has_next()) {
      auto &batch = iter.next();

      dst_t *end = batch.start + batch.size;
      dst_t *n = batch.start;
      while (n < end) {
        if (distances[*n] == maxDistance) {
          distances[*n] = distances[v] + 1;
          work.push(*n);
        }
        n++;
      }
    }
  }
  return distances;
}

vector<uint> Algorithms::bfs_single_edge_interface(Driver &driver, TopologyInterface &ds, vertex_id_t start_vertex) {
  size_t vertices_traversed = 0;

  uint maxDistance = numeric_limits<uint>::max();
  vector<uint> distances(ds.max_physical_vertex(), numeric_limits<uint>::max());
  distances[start_vertex] = 0;
  queue<vertex_id_t> work;
  work.push(start_vertex);

  EdgeIterator &iter = driver.getSingleEdgeIter(ds); // TODO move getIter to data structure instead of driver
  while (!work.empty()) {
    vertex_id_t v = work.front();
    work.pop();

    vertices_traversed++;

    ds.neighbourhood_p(v, iter);
    while (iter.has_next()) {
      dst_t n = iter.next();
      if (distances[n] == maxDistance) {
        distances[n] = distances[v] + 1;
        work.push(n);
      }
    }
  }

  return distances;
}


uint Algorithms::traversed_vertices(TopologyInterface &ds, vector<pair<vertex_id_t, uint>> &distances) {
  auto count = 0;
  for (auto d : distances) {
    if (d.second != numeric_limits<uint>::max()) {
      count++;
    }
  }
  return count;
}

vector<uint>
Algorithms::bfs_raw_neighbourhood(Driver &driver, TopologyInterface &ds, vertex_id_t start_vertex, bool aquire_locks) {
  if (typeid(ds) != typeid(SnapshotTransaction)) {
    if (aquire_locks) {
      throw NotImplemented();
    }
  }

  size_t vertices_traversed = 0;
  vector<uint> distances(ds.max_physical_vertex(), numeric_limits<uint>::max());
  distances[start_vertex] = 0;
  uint maxDistance = numeric_limits<uint>::max();

  queue<vertex_id_t> work;
  work.push(start_vertex);

  if (typeid(ds) == typeid(HashSetSimulatorAdjacencyList)) {
    uint emtpy = numeric_limits<uint>::max();

    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      vertices_traversed++;

      dst_t *ns = (dst_t *) ds.raw_neighbourhood(v);
      dst_t *end = ns + ns[0] + 1;
      ns++;

      while (ns < end) {
        if (*ns != emtpy) {
          dst_t n = *ns;
          if (distances[n] == maxDistance) {
            distances[n] = distances[v] + 1;
            work.push(n);
          }
        }
        ns++;
      }
    }
  } else if (typeid(ds) == typeid(MallocAdjacencyLists)) {
    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      vertices_traversed++;

      dst_t *ns = (dst_t *) ds.raw_neighbourhood(v);
      dst_t *end = ns + ns[0] + 1;
      ns++;

      while (ns < end) {
        dst_t n = *ns;
        if (distances[n] == maxDistance) {
          distances[n] = distances[v] + 1;
          work.push(n);
        }
        ns++;
      }
    }
  } else if (typeid(ds) == typeid(CSRMallocAdjacencyLists)) {
    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      vertices_traversed++;

      auto n = (dst_t *) ds.raw_neighbourhood(v);
      auto end = n + ds.neighbourhood_size(v);
      while (n < end) {
        if (distances[*n] == maxDistance) {
          distances[*n] = distances[v] + 1;
          work.push(*n);
        }
        n++;
      }
    }

  } else if (typeid(ds) == typeid(BlockedLinkedListAdjacencyLists)) {
    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      vertices_traversed++;

      long tagged_pointer = (long) ds.raw_neighbourhood(v);

      if (tagged_pointer < 0) {
        tagged_pointer *= -1;

        dst_t *ns = (dst_t *) tagged_pointer;
        ns++;
        dst_t *end = ns + ds.neighbourhood_size(v);

        while (ns < end) {
          dst_t n = *ns;
          if (distances[n] == maxDistance) {
            distances[n] = distances[v] + 1;
            work.push(n);
          }
          ns++;
        }
      } else {
        BlockHeader *block = (BlockHeader *) tagged_pointer;
        while (block != nullptr) {
          auto data = block->data;
          dst_t *end = block->data + block->size;

          while (data < end) {
            dst_t n = *data;
            if (distances[n] == maxDistance) {
              distances[n] = distances[v] + 1;
              work.push(n);
            }
            data++;
          }
          block = block->next;
        }
      }
    }
  } else if (typeid(ds) == typeid(SnapshotTransaction)) {
    auto transaction = dynamic_cast<SnapshotTransaction &>(ds);
    auto trans_timestamp = transaction.get_version();
    auto raw_ds = dynamic_cast<VersioningBlockedSkipListAdjacencyList *>(transaction.raw_ds());

    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      vertices_traversed++;
      raw_ds->aquire_vertex_lock_p(v);
      if (raw_ds->get_set_type(v, trans_timestamp)) {
        dst_t *ns = (dst_t *) raw_ds->raw_neighbourhood_version(v, trans_timestamp);
        uint64_t size = 0x00000000FFFFFFFF & (uint64_t) raw_ds->raw_neighbourhood_size_entry(v); // TODO create function which exposes the tuple interface to the outside
        dst_t *end = ns + size;

        while (ns < end) {
          dst_t n = *ns;
          bool deleted = false;
          if (is_versioned(n)) {
            n = make_unversioned(n);
            ns++;  //  Now points to the version.
            auto version = (version_t) *ns;
            if (trans_timestamp < timestamp(version)) {
              deleted = !is_deletion(version);
            } else {
              deleted = is_deletion(version);
            }
          }
          if (!deleted && distances[n] == maxDistance) {
            distances[n] = distances[v] + 1;
            work.push(n);
          }
          ns++;
        }
      } else {
        VSkipListHeader *block = (VSkipListHeader *) raw_ds->raw_neighbourhood_version(v, trans_timestamp);
        while (block != nullptr) {
          auto data = block->data;
          dst_t *end = block->data + block->size;

          while (data < end) {
            dst_t n = *data;
            bool deleted = false;
            if (is_versioned(n)) {
              n = make_unversioned(n);
              data++;  // Now points to the version.
              auto version = (version_t) *data;
              if (trans_timestamp < timestamp(version)) {
                deleted = !is_deletion(version);
              } else {
                deleted = is_deletion(version);
              }
            }
            if (!deleted && distances[n] == maxDistance) {
              distances[n] = distances[v] + 1;
              work.push(n);
            }
            data++;
          }
          block = block->next_levels[0];
        }
      }
      raw_ds->release_vertex_lock_p(v);
    }
  } else if (typeid(ds) == typeid(BlockedSkipListAdjacencyLists)) {
    auto block_size = dynamic_cast<BlockedSkipListAdjacencyLists &>(ds).get_block_size();
    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      vertices_traversed++;

      if (ds.neighbourhood_size(v) <= block_size) {
        // TODO why is there no problem if ns is a nullptr, could it be that neighbourhood size returns incorrect values here?
        dst_t *ns = (dst_t *) ds.raw_neighbourhood(v);
        dst_t *end = ns + ds.neighbourhood_size(v);

        while (ns < end) {
          dst_t n = *ns;
          if (distances[n] == maxDistance) {
            distances[n] = distances[v] + 1;
            work.push(n);
          }
          ns++;
        }
      } else {
        BlockHeader *block = (BlockHeader *) ds.raw_neighbourhood(v);
        while (block != nullptr) {
          auto data = block->data;
          dst_t *end = block->data + block->size;

          while (data < end) {
            dst_t n = *data;
            if (distances[n] == maxDistance) {
              distances[n] = distances[v] + 1;
              work.push(n);
            }
            data++;
          }
          block = block->next;
        }
      }
    }
  } else if (typeid(ds) == typeid(CSR)) {
    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      vertices_traversed++;

      auto n = (dst_t *) ds.raw_neighbourhood(v);  //&(csr.adjacency_lists[csr.adjacency_index[v]]);
      auto end = n + ds.neighbourhood_size(v); //&(csr.adjacency_lists[csr.adjacency_index[v + 1]]);
      while (n < end) {
        if (distances[*n] == maxDistance) {
          distances[*n] = distances[v] + 1;
          work.push(*n);
        }
        n++;
      }
    }
  } else {
    throw NotImplemented();
  }
  return distances;
}

vector<pair<vertex_id_t, uint>> Algorithms::bfs(Driver &driver, TopologyInterface &ds, vertex_id_t start_vertex, bool raw_neighbourhood,
                             bool aquire_locks, bool gapbs) {
  auto start = chrono::steady_clock::now();
  start_vertex = ds.physical_id(start_vertex);  // Logical to physical translation
  vector<uint> physical_result;
  vector<pair<vertex_id_t, uint>> logical_result;
  if (gapbs) {
    physical_result = GAPBSAlgorithms::bfs(ds, start_vertex, raw_neighbourhood);
  } else if (raw_neighbourhood) {
    physical_result = bfs_raw_neighbourhood(driver, ds, start_vertex, aquire_locks);
  } else if (typeid(ds) == typeid(HashSetSimulatorAdjacencyList)) {
    if (aquire_locks) {
      throw NotImplemented();
    }
    physical_result = bfs_single_edge_interface(driver, ds, start_vertex);
  } else if (typeid(ds) == typeid(SnapshotTransaction)) {
    physical_result = bfs_single_edge_interface(driver, ds, start_vertex);
  } else {
    if (aquire_locks) {
      throw NotImplemented();
    }
    physical_result = bfs_batched_interface(driver, ds, start_vertex);
  }

  auto end = chrono::steady_clock::now();
  size_t milliseconds = chrono::duration_cast<chrono::milliseconds>(end - start).count();

  cout << "BFS took: " << milliseconds << " milliseconds" << endl;

  // Translation to logical
  // TODO rephrase once we have vertex iterators.
  start = chrono::steady_clock::now();
  logical_result.resize(ds.max_physical_vertex());
  auto V = ds.max_physical_vertex();
#pragma omp parallel for
  for (uint v = 0; v <  V; v++) {
    if (ds.has_vertex_p(v)) {
        logical_result[v] = make_pair(ds.logical_id(v), physical_result[v]);
    } else {
      logical_result[v] = make_pair(v, numeric_limits<uint>::max());
    }
  }
  end = chrono::steady_clock::now();
  milliseconds = chrono::duration_cast<chrono::milliseconds>(end - start).count();

  cout << "Translating took: " << milliseconds << " milliseconds" << endl;

  return logical_result;
}

unordered_map<vertex_id_t, size_t> Algorithms::neighbourhood_2(Driver &driver,
                                                               TopologyInterface &ds,
                                                               const vector<vertex_id_t> &sources,
                                                               bool raw_neighbourhood) {
  if (raw_neighbourhood) {
    return TwoNeighbour::neighbourhood_2_raw_neighbourhood(driver, ds, sources);
  } else if (typeid(ds) == typeid(HashSetSimulatorAdjacencyList)) {
    throw NotImplemented();
  } else {
    return TwoNeighbour::neighbourhood_2_batched_interface(driver, ds, sources);
  }
}

vector<float> Algorithms::page_rank(Driver &driver, TopologyInterface &ds, bool run_on_raw_neighbourhood) {
  const int max_iters = 5;
  const float epsilon = 1e-4;

  if (run_on_raw_neighbourhood) {
    return PageRank::page_rank_raw_neighbourhood(driver, ds, max_iters, epsilon);
  } else if (typeid(ds) == typeid(HashSetSimulatorAdjacencyList)) {
    throw NotImplemented();
  } else {
    return PageRank::page_rank_batched_interface(driver, ds, max_iters, epsilon);
  }
}
