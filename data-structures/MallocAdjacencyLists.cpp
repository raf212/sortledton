//
// Created by per on 07.09.20.
//

#include <random>
#include <algorithm>
#include <memory>
#include <iostream>
#include <cstring>
#include <utils/utils.h>
#include "MallocAdjacencyLists.h"

#include "adjacency-lists/VectorBatchedEdgeIterator.h"

void MallocAdjacencyLists::neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) {
  auto &i = static_cast<VectorBatchedEdgeIterator &>(iter);
  i.initialize(adjacency_index[src] + 1, *adjacency_index[src]);
}

void MallocAdjacencyLists::bulkload(const SortedCSRDataSource &src) {
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

    // malloc seems to return contiguous addresses. Let's seperate them.
    seperators.push_back((dst_t*) malloc(5 * 64));  // Seperate each malloced region by five cachelines.
  }
}

MallocAdjacencyLists::~MallocAdjacencyLists() {
  for (dst_t *al : adjacency_index) {
    free(al);
  }
}

void MallocAdjacencyLists::intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  if (unordered) {
    throw NotImplemented("Intersection of unordered list is not implemented.");
  }
  auto a_start = adjacency_index[a] + 1;
  auto b_start = adjacency_index[b] + 1;

  auto a_end = a_start + (size_t) *adjacency_index[a];
  auto b_end = b_start + (size_t) *adjacency_index[b];

  intersect_edge_block(a_start, a_end, b_start, b_end, out);
}
