//
// Created by per on 09.10.20.
//

#include "TwoNeighbourSourceSelector.h"

vector<vertex_id_t> TwoNeighbourSourceSelector::get_sources(uint number) {
  vector<vertex_id_t> out;
  auto ran = bind(distribution, gen);

  for (int i = 0; i < number; i++) {
    vertex_id_t v = ran();
    if (graph.adjacency_index[v] != graph.adjacency_index[v+1]) { // Has neighbours
      out.push_back(v);
    }
  }

  return vector<vertex_id_t>();
}
