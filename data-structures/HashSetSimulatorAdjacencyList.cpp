//
// Created by per on 14.10.20.
//

#include "HashSetSimulatorAdjacencyList.h"

#include <random>
#include <algorithm>
#include <memory>
#include <iostream>
#include <cstring>
#include <utils/utils.h>

#include "adjacency-lists/FilteredVectorIterator.h"

HashSetSimulatorAdjacencyList::HashSetSimulatorAdjacencyList(float fill_rate) : fill_rate(fill_rate) {

}



void HashSetSimulatorAdjacencyList::neighbourhood_p(vertex_id_t src, EdgeIterator &iter) {
  auto &i = static_cast<FilteredVectorIterator &>(iter);
  i.initialize(adjacency_index[src] + 1, *adjacency_index[src]);
}

void HashSetSimulatorAdjacencyList::bulkload(const SortedCSRDataSource &src) {
  adjacency_index.reserve(src.vertex_count());

  mt19937 engine(42);
  bernoulli_distribution dist(fill_rate);
  for (size_t i = 0; i < src.adjacency_index.size() - 1; i++) {
    auto begin = src.adjacency_lists.begin() + src.adjacency_index[i];
    auto end = src.adjacency_lists.begin() + src.adjacency_index[i + 1];

    size_t elements = end - begin;
    size_t actual_size = elements / fill_rate;

    dst_t *adjacency_list = (dst_t *) pool.get_memory((actual_size + 1) * sizeof(dst_t)); // + 1 for the size

    adjacency_list[0] = actual_size;

    uint unfilled = 0;
    uint unfilled_limit = actual_size - elements;
    auto j = begin;
    dst_t* write_to = &adjacency_list[1];
    while (j < end) {
      bool used = dist(engine);
      if (!used && unfilled < unfilled_limit) {
        *write_to = numeric_limits<dst_t>::max();
        unfilled++;
      } else {
        *write_to = *j;
        j++;

      }
      write_to++;
    }

    while (unfilled < unfilled_limit) {
      *write_to = numeric_limits<dst_t>::max();
      unfilled++;
      write_to++;
    }

    adjacency_index.push_back(adjacency_list);
  }
}

void HashSetSimulatorAdjacencyList::intersect_neighbourhood_p(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  throw NotImplemented();
}

size_t HashSetSimulatorAdjacencyList::vertex_count() {
    return adjacency_index.size();
}
