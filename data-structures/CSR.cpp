//
// Created by per on 31.08.20.
//

#include <algorithm>
#include <utils/utils.h>
#include "CSR.h"

void CSR::bulkload(const SortedCSRDataSource &src) {
  adjacency_index = src.adjacency_index;
  adjacency_lists = src.adjacency_lists;
}

void CSR::neighbourhood(vertex_id_t src, BatchedEdgeIterator& iter) {
  auto& vbi = static_cast<VectorBatchedEdgeIterator&>(iter);
  vbi.initialize(&adjacency_lists[0] + adjacency_index[src], adjacency_index[src + 1] - adjacency_index[src]);
}

void CSR::intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  auto start_a = &adjacency_lists[0] + adjacency_index[a];
  auto start_b = &adjacency_lists[0] + adjacency_index[b];

  auto end_a = &adjacency_lists[0] + adjacency_index[a+1];
  auto end_b = &adjacency_lists[0] + adjacency_index[b+1];

  intersect_edge_block(start_a, end_a, start_b, end_b, out);
}
