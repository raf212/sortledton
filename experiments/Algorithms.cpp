//
// Created by per on 09.10.20.
//

#include "Algorithms.h"

#include <algorithm>
#include <queue>

#include <HashSetSimulatorAdjacencyList.h>
#include <MallocAdjacencyLists.h>
#include <CSR.h>

vector<uint> Algorithms::bfs_batched_interface(Driver &driver, TopologyInterface &ds, vertex_id_t start_vertex) {
  size_t vertices_traversed = 0;

  uint maxDistance = numeric_limits<uint>::max();
  vector<uint> distances(ds.vertex_count(), numeric_limits<uint>::max());
  queue<vertex_id_t> work;
  work.push(start_vertex);

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
  vector<uint> distances(ds.vertex_count(), numeric_limits<uint>::max());
  queue<vertex_id_t> work;
  work.push(start_vertex);

  EdgeIterator &iter = driver.getSingleEdgeIter(ds); // TODO move getIter to data structure instead of driver
  while (!work.empty()) {
    vertex_id_t v = work.front();
    work.pop();

    vertices_traversed++;

    ds.neighbourhood(v, iter);
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


uint Algorithms::traversed_vertices(TopologyInterface &ds, vector<uint> &distances) {
  return ds.vertex_count() - count(distances.begin(), distances.end(), numeric_limits<uint>::max());
}

vector<uint> Algorithms::bfs_raw_neighbourhood(Driver &driver, TopologyInterface &ds, vertex_id_t start_vertex) {
  size_t vertices_traversed = 0;
  vector<uint> distances(ds.vertex_count(), numeric_limits<uint>::max());
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
  } else if (typeid(ds) == typeid(BlockedLinkedListAdjacencyLists)) {
    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      vertices_traversed++;

      BlockHeader *block = (BlockHeader *) ds.raw_neighbourhood(v);
      while (block != nullptr) {
        auto data = block->data;
        dst_t *end = block->data + block->size;

        while (data < end) {
          if (distances[*data] == maxDistance) {
            distances[*data] = distances[v] + 1;
            work.push(*data);
          }
          data++;
        }
        block = block->next;
      }
    }
  } else if (typeid(ds) == typeid(CSR)) {
    CSR& csr = dynamic_cast<CSR&>(ds);
    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      vertices_traversed++;

      auto n = &(csr.adjacency_lists[csr.adjacency_index[v]]);
      auto end = &(csr.adjacency_lists[csr.adjacency_index[v + 1]]);
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

vector<uint> Algorithms::bfs(Driver &driver, TopologyInterface &ds, vertex_id_t start_vertex, bool raw_neighbourhood) {
  if (raw_neighbourhood) {
    return bfs_raw_neighbourhood(driver, ds, start_vertex);
  } else if (typeid(ds) == typeid(HashSetSimulatorAdjacencyList)) {
    return bfs_single_edge_interface(driver, ds, start_vertex);
  } else {
    return bfs_batched_interface(driver, ds, start_vertex);
  }
}
