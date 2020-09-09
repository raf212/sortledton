//
// Created by per on 08.09.20.
//

#include <iostream>
#include <random>
#include <algorithm>
#include <cstring>
#include <data-structures/adjacency-lists/VectorBatchedEdgeIterator.h>
#include <utils/utils.h>
#include "CSRMallocAdjacencyLists.h"

void CSRMallocAdjacencyLists::bulkload(const SortedCSRDataSource &src) {
  cout << "Unoredered " << unordered << endl;
  size_t csr_size = 0;

  for (int i = 0; i < src.adjacency_index.size(); i++) {
    auto size = src.adjacency_index[i + 1] - src.adjacency_index[i];
    if (size < malloc_limit) {
      csr_size += size;
    }
  }

  csr = (dst_t*) malloc(csr_size * sizeof(dst_t));

  dst_t* csr_position = csr;
  for (size_t i = 0; i < src.adjacency_index.size() - 1; i++) {
    auto begin = src.adjacency_lists.begin() + src.adjacency_index[i];
    auto end = src.adjacency_lists.begin() + src.adjacency_index[i+1];

    vector<dst_t> shuffled_src(begin, end);
    if (unordered) {
      shuffle(shuffled_src.begin(), shuffled_src.end(), std::mt19937(std::random_device()()));
    }

    if (shuffled_src.size() < malloc_limit) {
      memcpy((void *) (csr_position), (void *) shuffled_src.data(), shuffled_src.size() * sizeof(dst_t));

      adjacency_index.push_back(csr_position);
      adjacency_index.push_back((dst_t*) shuffled_src.size());

      csr_position += shuffled_src.size();
    } else {
      dst_t *adjacency_list = (dst_t *) malloc((shuffled_src.size()) * sizeof(dst_t));

      memcpy((void *) &adjacency_list[0], (void *) shuffled_src.data(), shuffled_src.size() * sizeof(dst_t));

      adjacency_index.push_back(adjacency_list);
      adjacency_index.push_back((dst_t*) shuffled_src.size());
    }
  }
}

CSRMallocAdjacencyLists::~CSRMallocAdjacencyLists() {
//  for (const auto& al : adjacency_index) {
//    free(al);  // Dangerous some of these have not been allocated by malloc, let's see what happens.
//  }
  free(csr);
}

void CSRMallocAdjacencyLists::neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) {
  auto& i = static_cast<VectorBatchedEdgeIterator&>(iter);
  i.initialize(adjacency_index[2 * src], (size_t) adjacency_index[2 * src + 1]);
}

void CSRMallocAdjacencyLists::intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  if (unordered) {
    throw NotImplemented("Intersection of unordered list is not implemented.");
  }
  auto a_start = adjacency_index[2 * a];
  auto b_start = adjacency_index[2 * b];

  auto a_end = a_start + (size_t) adjacency_index[2* a + 1];
  auto b_end = b_start + (size_t) adjacency_index[2* b + 1];

  intersect_edge_block(a_start, a_end, b_start, b_end, out);
}
