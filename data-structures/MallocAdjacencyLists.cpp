//
// Created by per on 07.09.20.
//

#include <random>
#include <algorithm>
#include <memory>
#include <iostream>
#include <cstring>
#include "MallocAdjacencyLists.h"

#include "adjacency-lists/VectorBatchedEdgeIterator.h"

void MallocAdjacencyLists::neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) {
  auto &i = static_cast<VectorBatchedEdgeIterator &>(iter);
  i.initialize(adjacency_index[src] + 1, *adjacency_index[src]);
}

void MallocAdjacencyLists::bulkload(const SortedCSRDataSource &src) {
  cout << "Unoredered " << unordered << endl;
  adjacency_index.reserve(src.adjacency_index.size() - 1);

  for (size_t i = 0; i < src.adjacency_index.size() - 1; i++) {
    auto begin = src.adjacency_lists.begin() + src.adjacency_index[i];
    auto end = src.adjacency_lists.begin() + src.adjacency_index[i + 1];

    vector<dst_t> shuffled_src(begin, end);
    if (unordered) {
      shuffle(shuffled_src.begin(), shuffled_src.end(), std::mt19937(std::random_device()()));
    }

    dst_t *adjacency_list = (dst_t *) malloc((shuffled_src.size() + 1) * sizeof(dst_t));

    adjacency_list[0] = shuffled_src.size();
    memcpy((void *) &adjacency_list[1], (void *) shuffled_src.data(), shuffled_src.size() * sizeof(dst_t));

    adjacency_index.push_back(adjacency_list);
  }
}

MallocAdjacencyLists::~MallocAdjacencyLists() {
  for (dst_t *al : adjacency_index) {
    free(al);
  }
}
