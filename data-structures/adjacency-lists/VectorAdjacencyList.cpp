//
// Created by per on 31.08.20.
//

#include "VectorAdjacencyList.h"

VectorBatchedEdgeIterator& VectorAdjacencyList::iterator() {
  iter.batch.start = neighbourhood.data();
  iter.batch.size = neighbourhood.size();

  return iter;
}

void VectorAdjacencyList::insert_edge(dst_t edge) {
  neighbourhood.push_back(edge);
}

void VectorAdjacencyList::delete_edge(dst_t edge) {
  int i = 0;
  while (i < neighbourhood.size()) {
    if (neighbourhood[i] == edge) {
      break;
    }
    i++;
  }
  while (i < neighbourhood.size()) {
    neighbourhood[i] = neighbourhood[i + 1];
    i++;
  }
}

void VectorAdjacencyList::intersect(AdjacencyList &other, vector<dst_t>& out) {
  if (typeid(other) == typeid(this)) {
    VectorAdjacencyList &a = dynamic_cast<VectorAdjacencyList&>(other);
    VectorAdjacencyList &b = *this;
    if (b.neighbourhood.size() < a.neighbourhood.size()) {
      swap(a, b);
    }
    out.reserve(a.neighbourhood.size());
    for (const dst_t n : a.neighbourhood) {
      for (const dst_t m : b.neighbourhood) {
        if (n == m) {
          out.push_back(n);
        }
      }
    }
  } else {
    throw invalid_argument("Other needs to be of type VectorAdjacencyList");
  }
}
