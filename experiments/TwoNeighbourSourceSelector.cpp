//
// Created by per on 09.10.20.
//

#include "TwoNeighbourSourceSelector.h"

vector<vertex_id_t> TwoNeighbourSourceSelector::get_sources(uint number) {
  vector<vertex_id_t> out;

  for (int i = 0; i < number; i++) {
    vertex_id_t v = distribution(gen);
    if (graph.adjacency_index[v] != graph.adjacency_index[v+1]) { // Has neighbours
      out.push_back(v);
    }
  }

  return out;
}
