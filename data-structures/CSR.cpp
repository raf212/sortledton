//
// Created by per on 31.08.20.
//

#include <algorithm>
#include "CSR.h"

void CSR::bulkload(const SortedCSRDataSource &src) {
  adjacency_index = src.adjacency_index;
  adjacency_lists = src.adjacency_lists;

  iterators.reserve(adjacency_index.size());
  dst_t* base_address = adjacency_lists.data();
  for (size_t i = 0; i < src.adjacency_index.size() - 1; i++) {
    VectorBatchedEdgeIterator iter;
    size_t size = adjacency_lists[i + 1] - adjacency_lists[i];
    iter.batch = ContiguousEdgeBatch(base_address + adjacency_lists[i], size);
    iterators[i] = iter;
  }
}

BatchedEdgeIterator &CSR::neighbourhood(vertex_id_t src) {
  return iterators[src];
}

void CSR::intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  auto a_batch = iterators[a].batch;
  auto b_batch = iterators[b].batch;

  if (b_batch.size < a_batch.size) {
    swap(a_batch, b_batch);
  }

  dst_t* n = a_batch.start;
  dst_t* m = b_batch.start;

  dst_t* a_end = a_batch.start + a_batch.size;
  dst_t* b_end = b_batch.start + b_batch.size;
  while (n < a_end) {
    m = upper_bound(m, b_end, *n);
    if (*n == *m) {
      out.push_back(*n);
    }
    while (*n < *m && n < a_end) {
      n++;
    }
  }

}
