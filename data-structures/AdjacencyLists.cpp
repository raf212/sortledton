//
// Created by per on 31.08.20.
//

#include <data-src/SortedCSRDataSource.h>
#include <utils/NotImplemented.h>
#include "AdjacencyLists.h"

void AdjacencyLists::bulkload(const SortedCSRDataSource& src) {
  adjacency_index.reserve(src.adjacency_index.size() - 1);

  for (size_t i = 0; i < src.adjacency_index.size() - 1; i++) {
    auto begin = src.adjacency_lists.begin() + src.adjacency_index[i];
    auto end = src.adjacency_lists.begin() + src.adjacency_index[i+1];
    adjacency_index.push_back(construct_adjacency_list(begin, end));
  }
}

bool AdjacencyLists::insert_vertex(vertex_id_t v) {
  throw NotImplemented();
}

bool AdjacencyLists::delete_vertex(vertex_id_t v) {
  throw NotImplemented();
}

bool AdjacencyLists::insert_edge(edge_t edge) {
  adjacency_index[edge.src]->insert_edge(edge.dst);
}

bool AdjacencyLists::delete_edge(edge_t edge) {
  adjacency_index[edge.src]->delete_edge(edge.dst);
}

void AdjacencyLists::intersect_neighbourhood_p(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  adjacency_index[a]->intersect(*adjacency_index[b], out);
}

void AdjacencyLists::neighbourhood_p(vertex_id_t src, BatchedEdgeIterator& iter) {
  adjacency_index[src]->initialize_iterator(iter);
}

bool AdjacencyLists::has_edge_p(edge_t e) {
  return adjacency_index[e.src]->has_neighbour(e.dst);
}

