//
// Created by per on 31.08.20.
//

#include <data-src/SortedCSRDataSource.h>
#include <utils/NotImplemented.h>
#include "AdjacencyLists.h"

void AdjacencyLists::bulkload(const SortedCSRDataSource& src) {
  adjacency_index.reserve(src.adjacency_index.size());

  for (size_t i = 0; i < src.adjacency_index.size(); i++) {
    auto src_neighbourhood = src.adjacency_lists[src.adjacency_index[i]];
    adjacency_index[i] = construct_adjacency_list(src_neighbourhood);
  }
}

vertex_id_t AdjacencyLists::insert_vertex() {
  throw NotImplemented();
}

void AdjacencyLists::delete_vertex() {
  throw NotImplemented();
}

void AdjacencyLists::insert_edge(edge_t edge) {
  adjacency_index[edge.src]->insert_edge(edge.dst);
}

void AdjacencyLists::delete_edge(edge_t edge) {
  adjacency_index[edge.src]->delete_edge(edge.dst);
}

void AdjacencyLists::intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  adjacency_index[a]->intersect(*adjacency_index[b], out);
}

BatchedEdgeIterator &AdjacencyLists::neighbourhood(vertex_id_t src) {
  return adjacency_index[src]->iterator();
}

