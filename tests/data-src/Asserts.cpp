//
// Created by per on 08.10.20.
//

#include "Asserts.h"

#include <gmock/gmock-matchers.h>

void testing::Asserts::assert_csr(SortedCSRDataSource &src, vector<pair<dst_t, dst_t>> &edges) {

  vector<pair<dst_t, dst_t>> actual_edges;
  actual_edges.reserve(edges.size());

  for (vertex_id_t v = 0; v < src.vertex_count(); v++) {
    for (size_t o = src.adjacency_index[v]; o < src.adjacency_index[v+1]; o++) {
      actual_edges.emplace_back(v, src.adjacency_lists[o]);
    }
  }

  EXPECT_THAT(actual_edges, ContainerEq(edges));
}
