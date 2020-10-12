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
  if (use_hash_index) {
    auto e = hash_index.find(src);
    if (e != hash_index.end()) {
      i.initialize(e->second + 1, *e->second);
    } else {
      i.initialize(nullptr, 0);
    }

  } else {
    i.initialize(adjacency_index[src] + 1, *adjacency_index[src]);
  }
}

void MallocAdjacencyLists::bulkload(const SortedCSRDataSource &src) {
  adjacency_index.reserve(src.vertex_count());
  hash_index.reserve(src.vertex_count());

  for (size_t i = 0; i < src.adjacency_index.size() - 1; i++) {
    auto begin = src.adjacency_lists.begin() + src.adjacency_index[i];
    auto end = src.adjacency_lists.begin() + src.adjacency_index[i + 1];

    vector<dst_t> shuffled_src(begin, end);
    if (unordered) {
      shuffle(shuffled_src.begin(), shuffled_src.end(), std::mt19937(std::random_device()()));
    }

    dst_t *adjacency_list = (dst_t *) pool.get_memory((shuffled_src.size() + 1) * sizeof(dst_t));

    adjacency_list[0] = shuffled_src.size();
    memcpy((void *) &adjacency_list[1], (void *) shuffled_src.data(), shuffled_src.size() * sizeof(dst_t));

    if (use_hash_index) {
      hash_index.insert({i, adjacency_list});
    } else {
      adjacency_index.push_back(adjacency_list);
    }
  }
}

void MallocAdjacencyLists::intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  if (unordered) {
    throw NotImplemented("Intersection of unordered list is not implemented.");
  }

  dst_t* a_start;
  dst_t* b_start;

  dst_t* a_end;
  dst_t* b_end;
  if (use_hash_index) {
    auto a_e = hash_index.find(a);
    auto b_e = hash_index.find(b);

    if (a_e == hash_index.end() || b_e == hash_index.end()) {
      out.resize(0);
      return;
    } else{
      a_start = a_e->second + 1;
      b_start = b_e->second + 1;

      a_end = a_start + (size_t) *a_e->second;
      b_end = b_start + (size_t) *b_e->second;
    }

  } else {
    a_start = adjacency_index[a] + 1;
    b_start = adjacency_index[b] + 1;

    a_end = a_start + (size_t) *adjacency_index[a];
    b_end = b_start + (size_t) *adjacency_index[b];
  }


  intersect_edge_block(a_start, a_end, b_start, b_end, out);
}

size_t MallocAdjacencyLists::vertex_count() {
  if (use_hash_index) {
    return hash_index.size();
  } else {
    return adjacency_index.size();
  }
}

MallocAdjacencyLists::~MallocAdjacencyLists() = default;
