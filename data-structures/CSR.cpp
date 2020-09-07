//
// Created by per on 31.08.20.
//

#include <algorithm>
#include "CSR.h"

void CSR::bulkload(const SortedCSRDataSource &src) {
  adjacency_index = src.adjacency_index;
  adjacency_lists = src.adjacency_lists;
}

void CSR::neighbourhood(vertex_id_t src, BatchedEdgeIterator& iter) {
  auto& vbi = static_cast<VectorBatchedEdgeIterator&>(iter);
  vbi.hn = true;
  vbi.batch.start = &adjacency_lists[0] + adjacency_index[src];
  vbi.batch.size = adjacency_index[src + 1] - adjacency_index[src];
}

void CSR::intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  out.clear();

  auto a_size = adjacency_index[a+1] - adjacency_index[a];
  auto b_size = adjacency_index[b+1] - adjacency_index[b];

//  if (b_batch.size < a_batch.size) {
//    swap(a_batch, b_batch);
//  }

  dst_t* n = &adjacency_lists[adjacency_index[a]];
  dst_t* m = &adjacency_lists[adjacency_index[b]];

  dst_t* a_end = n + a_size;
  dst_t* b_end = m + b_size;

  if (b_size < a_size) {
    swap(n, m);
    swap(a_end, b_end);
    swap(a_size, b_size);
  }

  while (n < a_end) {
    m = upper_bound(m, b_end, *n);
    if (m == b_end) {
      break;
    }
    if (*n == *m) {
      out.push_back(*n);
    }
    while (*n < *m && n < a_end) {
      n++;
    }
  }

}
