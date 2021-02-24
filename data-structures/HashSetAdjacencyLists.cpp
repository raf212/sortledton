//
// Created by per on 28.10.20.
//

#include "HashSetAdjacencyLists.h"

void HashSetAdjacencyLists::bulkload(const SortedCSRDataSource &src) {
  adjacency_index.reserve(src.vertex_count());

  for (vertex_id_t v = 0; v < src.vertex_count(); v++) {
    auto begin = src.adjacency_lists.begin() + src.adjacency_index[v];
    auto end = src.adjacency_lists.begin() + src.adjacency_index[v + 1];

    auto set = new robin_hood::unordered_flat_set<dst_t>(begin, end);

    adjacency_index.push_back(set);
  }
}

HashSetAdjacencyLists::~HashSetAdjacencyLists() {
  for (auto s : adjacency_index) {
    delete s;
  }
}

size_t HashSetAdjacencyLists::vertex_count() {
  return adjacency_index.size();
}

size_t HashSetAdjacencyLists::neighbourhood_size_p(vertex_id_t src) {
  return adjacency_index[src]->size();
}

void HashSetAdjacencyLists::intersect_neighbourhood_p(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  auto a_s = neighbourhood_size(a);
  auto b_s = neighbourhood_size(b);

  if (b_s < a_s) {
    swap(a, b);
    swap(a_s, b_s);
  }

  out.clear();
  out.reserve(a_s);

  auto a_n = (robin_hood::unordered_flat_set<dst_t>*) raw_neighbourhood(a);
  auto b_n = (robin_hood::unordered_flat_set<dst_t>*) raw_neighbourhood(b);

  for (auto d : *a_n) {
    if (b_n->find(d) != b_n->end()) {
      out.push_back(d);
    }
  }

}
