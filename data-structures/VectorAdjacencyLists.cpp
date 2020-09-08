//
// Created by per on 31.08.20.
//

#include <algorithm>
#include <random>
#include <data-structures/adjacency-lists/VectorAdjacencyList.h>
#include "VectorAdjacencyLists.h"

unique_ptr<AdjacencyList> VectorAdjacencyLists::construct_adjacency_list(vector<dst_t>::const_iterator begin,
        vector<dst_t>::const_iterator end) {
  vector<dst_t> shuffled_src (begin, end);
  if (unordered) {
    shuffle(shuffled_src.begin(), shuffled_src.end(), std::mt19937(std::random_device()()));
  }

  return make_unique<VectorAdjacencyList>(shuffled_src);
}
