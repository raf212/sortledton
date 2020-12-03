//
// Created by per on 09.10.20.
//

#include "Algorithms.h"
#include "TwoNeighbour.h"
#include "PageRank.h"

#include <algorithm>
#include <queue>

#include <HashSetSimulatorAdjacencyList.h>
#include <MallocAdjacencyLists.h>
#include <CSR.h>
#include <CSRMallocAdjacencyLists.h>
#include <BlockedSkipListAdjacencyLists.h>

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
  } else if (typeid(ds) == typeid(CSRMallocAdjacencyLists)) {
    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      vertices_traversed++;

      auto n =  (dst_t*) ds.raw_neighbourhood(v);
      auto end = n + ds.neighbourhood_size(v);
      while (n < end) {
        if (distances[*n] == maxDistance) {
          distances[*n] = distances[v] + 1;
          work.push(*n);
        }
        n++;
      }
    }

  }
  else if (typeid(ds) == typeid(BlockedLinkedListAdjacencyLists)) {
    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      vertices_traversed++;

      long tagged_pointer = (long) ds.raw_neighbourhood(v);

      if (tagged_pointer < 0) {
        tagged_pointer *= -1;

        dst_t *ns = (dst_t *) tagged_pointer ;
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
        BlockHeader* block = (BlockHeader*) tagged_pointer;
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
  } else if (typeid(ds) == typeid(BlockedSkipListAdjacencyLists)) {
    auto block_size = dynamic_cast<BlockedSkipListAdjacencyLists&>(ds).get_block_size();
    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      vertices_traversed++;

      if (ds.neighbourhood_size(v) <= block_size) {
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
        BlockHeader* block = (BlockHeader*) ds.raw_neighbourhood(v);
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
    CSR& csr = dynamic_cast<CSR&>(ds);
    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      vertices_traversed++;

      auto n = (dst_t*) ds.raw_neighbourhood(v);  //&(csr.adjacency_lists[csr.adjacency_index[v]]);
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

vector<uint> Algorithms::bfs(Driver &driver, TopologyInterface &ds, vertex_id_t start_vertex, bool raw_neighbourhood) {
  if (raw_neighbourhood) {
    return bfs_raw_neighbourhood(driver, ds, start_vertex);
  } else if (typeid(ds) == typeid(HashSetSimulatorAdjacencyList)) {
    return bfs_single_edge_interface(driver, ds, start_vertex);
  } else {
    return bfs_batched_interface(driver, ds, start_vertex);
  }
}

unordered_map<vertex_id_t, size_t> Algorithms::neighbourhood_2(Driver& driver,
        TopologyInterface &ds, const vector<vertex_id_t> &sources, bool raw_neighbourhood) {
  if (raw_neighbourhood) {
    return TwoNeighbour::neighbourhood_2_raw_neighbourhood(driver, ds, sources);
  } else if (typeid(ds) == typeid(HashSetSimulatorAdjacencyList)) {
    throw NotImplemented();
  } else {
    return TwoNeighbour::neighbourhood_2_batched_interface(driver, ds, sources);
  }
}

vector<float> Algorithms::page_rank(Driver& driver, TopologyInterface &ds, bool run_on_raw_neighbourhood) {
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
