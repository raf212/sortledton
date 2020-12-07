//
// Created by per on 31.08.20.
//

#include <algorithm>
#include <utils/utils.h>
#include <iostream>
#include <iomanip>
#include "CSR.h"

void CSR::bulkload(const SortedCSRDataSource &src) {
  adjacency_index = src.adjacency_index;
  adjacency_lists = src.adjacency_lists;
}

void CSR::neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) {
  auto &vbi = static_cast<VectorBatchedEdgeIterator &>(iter);
  vbi.initialize(&adjacency_lists[0] + adjacency_index[src], adjacency_index[src + 1] - adjacency_index[src]);
}

void CSR::intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  out.clear();

  dst_t *start_a = &adjacency_lists[0] + adjacency_index[a];
  dst_t *start_b = &adjacency_lists[0] + adjacency_index[b];

  dst_t *end_a = &adjacency_lists[0] + adjacency_index[a + 1];
  dst_t *end_b = &adjacency_lists[0] + adjacency_index[b + 1];

  long s_a = end_a - start_a;
  long s_b = end_b - start_b;

  if (s_b < s_a) {
    swap(start_a, start_b);
    swap(end_a, end_b);
    swap(s_a, s_b);
  }

  if (32 * s_a < s_b) {
    if (s_a == 0) {
      return;
    }

    auto out_iter = back_inserter(out);
    while (start_a < end_a) {
      long galloping_upper_bound = 1;
      while (galloping_upper_bound < s_b && start_b[galloping_upper_bound] < *start_a) {
        galloping_upper_bound *= 2;
      }
      if (binary_search(start_b + galloping_upper_bound / 2, start_b +  min(galloping_upper_bound + 1, s_b), *start_a)) {
        *out_iter = *start_a;
      }
      start_a++;
    }
  } else {
    set_intersection(start_a, end_a, start_b, end_b, back_inserter(out));
  }
}

bool CSR::has_edge(edge_t edge) {
  dst_t *last = &adjacency_lists[adjacency_index[edge.src + 1]];
  dst_t *first = &adjacency_lists[adjacency_index[edge.src]];
  return find(first, last, edge.dst) != last;
}

size_t CSR::neighbourhood_size(vertex_id_t src) {
  return adjacency_index[src + 1] - adjacency_index[src];
}

void *CSR::raw_neighbourhood(vertex_id_t src) {
  return &adjacency_lists[adjacency_index[src]];
}

void CSR::report_storage_size() {
  size_t edges = + sizeof(dst_t) * adjacency_lists.size();
  size_t vertices = sizeof(size_t) * adjacency_index.size();

  cout << setw(10) << "Vertices: " << right << setw(20) <<  vertices << endl;
  cout << setw(10) << "Edges: " << right << setw(20) <<  edges << endl;
  cout << endl;
  cout << setw(10) << "Total: " << right << setw(20) <<  edges + vertices << endl;
}
