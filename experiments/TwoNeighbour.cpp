//
// Created by per on 23.10.20.
//

#include <data-structures/adjacency-lists/ContigiousBlockIterator.h>
#include <HashSetSimulatorAdjacencyList.h>
#include <MallocAdjacencyLists.h>
#include <CSR.h>
#include "TwoNeighbour.h"

unordered_map<vertex_id_t, size_t> TwoNeighbour::neighbourhood_2_batched_interface(Driver &driver,
                                                                                   TopologyInterface &ds,
                                                                                   const vector<vertex_id_t> &sources) {
  __attribute__((unused)) ContigiousBlockIterator &neighbour_neighbours = driver.getIter(ds);
  ContigiousBlockIterator &neighbours = driver.getIter(ds);
  ContigiousBlockIterator &neighbours_3 = driver.getIter(ds);

  unordered_map<vertex_id_t, size_t> neighbour_counts;
  unordered_set<dst_t> visited;
  visited.reserve(100000);
  for (const auto &s : sources) {
    visited.clear();

    size_t count = 0;
    ds.neighbourhood(s, neighbours);
    while (neighbours.has_next()) {
      auto &batch = neighbours.next();
      for (const auto &n : batch) {
        if (visited.find(n) == visited.end()) {
          visited.insert(n);
          count++;

          ds.neighbourhood(n, neighbours_3);
          while (neighbours_3.has_next()) {
            auto &batch2 = neighbours_3.next();

            for (const auto &nn : batch2) {
              if (visited.find(nn) == visited.end()) {
                visited.insert(nn);
                count++;
              }
            }
          }
        }
        neighbour_counts.insert(make_pair(s, count));
      }
    }
  }
  return neighbour_counts;
}

unordered_map<vertex_id_t, size_t>
TwoNeighbour::neighbourhood_2_raw_neighbourhood(Driver &driver, TopologyInterface &ds,
                                                const vector<vertex_id_t> &sources) {
  unordered_map<vertex_id_t, size_t> neighbour_counts;
  unordered_set<dst_t> visited;
  visited.reserve(100000);

  if (typeid(ds) == typeid(HashSetSimulatorAdjacencyList)) {
    auto &d = dynamic_cast<HashSetSimulatorAdjacencyList &>(ds);
    dst_t empty = numeric_limits<dst_t>::max();

    for (const auto &s : sources) {
      visited.clear();

      size_t count = 0;

      auto n = (dst_t *) d.raw_neighbourhood(s);
      auto n_end = n + 1 + *n;
      n++;

      while (n < n_end) {
        if (*n != empty) {
          if (visited.find(*n) == visited.end()) {
            visited.insert(*n);
            count++;

            auto n_n = (dst_t *) d.raw_neighbourhood(*n);
            auto n_n_end = n_n + *n_n + 1;
            n_n++;

            while (n_n < n_n_end) {
              if (*n_n != empty) {
                if (visited.find(*n_n) == visited.end()) {
                  visited.insert(*n_n);
                  count++;
                }
              }
              n_n++;
            }
          }
        }
        n++;
      }
      neighbour_counts.insert(make_pair(s, count));
    }
  } else if (typeid(ds) == typeid(MallocAdjacencyLists)) {

    for (const auto &s : sources) {
      visited.clear();

      size_t count = 0;

      auto n = (dst_t *) ds.raw_neighbourhood(s);
      auto n_end = n + 1 + *n;
      n++;

      while (n < n_end) {
        if (visited.find(*n) == visited.end()) {
          visited.insert(*n);
          count++;

          auto n_n = (dst_t *) ds.raw_neighbourhood(*n);
          auto n_n_end = n_n + *n_n + 1;
          n_n++;

          while (n_n < n_n_end) {
            if (visited.find(*n_n) == visited.end()) {
              visited.insert(*n_n);
              count++;
            }
            n_n++;
          }
        }
        n++;
      }
      neighbour_counts.insert(make_pair(s, count));
    }
  } else if (typeid(ds) == typeid(BlockedLinkedListAdjacencyLists)) {
    auto &d = dynamic_cast<BlockedLinkedListAdjacencyLists &>(ds);

    for (const auto &s : sources) {
      visited.clear();

      size_t count = 0;

      auto b = (BlockHeader *) d.raw_neighbourhood(s);
      while (b != nullptr) {
        auto n = b->data;
        auto n_end = n + b->size;

        while (n < n_end) {
          if (visited.find(*n) == visited.end()) {
            visited.insert(*n);
            count++;

            auto n_b = (BlockHeader *) d.raw_neighbourhood(*n);
            while (n_b != nullptr) {
              auto n_n = n_b->data;
              auto n_n_end = n_n + n_b->size;

              while (n_n < n_n_end) {
                if (visited.find(*n_n) == visited.end()) {
                  visited.insert(*n_n);
                  count++;
                }
                n_n++;
              }
              n_b = n_b->next;
            }
          }
          n++;
        }
        b = b->next;
      }
      neighbour_counts.insert(make_pair(s, count));
    }
  } else if (typeid(ds) == typeid(CSR)) {
    auto &csr = dynamic_cast<CSR &>(ds);
    for (const auto &s : sources) {
      visited.clear();

      size_t count = 0;

      for (size_t n_index = csr.adjacency_index[s]; n_index < csr.adjacency_index[s + 1]; n_index++) {
        auto n = csr.adjacency_lists[n_index];
        if (visited.find(n) == visited.end()) {
          visited.insert(n);
          count++;

          for (size_t n_n_index = csr.adjacency_index[n]; n_n_index < csr.adjacency_index[n + 1]; n_n_index++) {
            auto n_n = csr.adjacency_lists[n_n_index];
            if (visited.find(n_n) == visited.end()) {
              visited.insert(n_n);
              count++;
            }
          }
        }
      }
      neighbour_counts.insert(make_pair(s, count));
    }
  } else {
    throw NotImplemented();
  }
  return neighbour_counts;
}
