//
// Created by per on 08.09.20.
//

#include <random>
#include <algorithm>
#include <cstring>
#include <data-structures/adjacency-lists/VectorBatchedEdgeIterator.h>
#include "CSRMallocAdjacencyLists.h"

void CSRMallocAdjacencyLists::bulkload(const SortedCSRDataSource &src) {
  size_t csr_size = 0;

  for (int i = 0; i < src.adjacency_index.size(); i++) {
    auto size = src.adjacency_index[i + 1] - src.adjacency_index[i];
    if (size < malloc_limit) {
      csr_size += size + 1;
    }
  }

  csr = (dst_t*) malloc(csr_size * sizeof(dst_t));

  dst_t* csr_position = csr;
  for (size_t i = 0; i < src.adjacency_index.size() - 1; i++) {
    auto begin = src.adjacency_lists.begin() + src.adjacency_index[i];
    auto end = src.adjacency_lists.begin() + src.adjacency_index[i+1];

    vector<dst_t> shuffled_src(begin, end);
    shuffle(shuffled_src.begin(), shuffled_src.end(), std::mt19937(std::random_device()()));

    if (shuffled_src.size() < malloc_limit) {
      *csr_position = shuffled_src.size();
      memcpy((void *) (csr_position + 1), (void *) shuffled_src.data(), shuffled_src.size() * sizeof(dst_t));

      adjacency_index.push_back(csr_position);

      csr_position += shuffled_src.size() + 1;
    } else {
      dst_t *adjacency_list = (dst_t *) malloc((shuffled_src.size() + 1) * sizeof(dst_t));

      adjacency_list[0] = shuffled_src.size();
      memcpy((void *) &adjacency_list[1], (void *) shuffled_src.data(), shuffled_src.size() * sizeof(dst_t));

      adjacency_index.push_back(adjacency_list);
    }
  }
}

CSRMallocAdjacencyLists::~CSRMallocAdjacencyLists() {
  for (const auto& al : adjacency_index) {
    free(al);  // Dangerous some of these have not been allocated by malloc, let's see what happens.
  }
  free(csr);
}

void CSRMallocAdjacencyLists::neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) {
  auto& i = static_cast<VectorBatchedEdgeIterator&>(iter);
  i.initialize(adjacency_index[src] + 1, *adjacency_index[src]);
}
