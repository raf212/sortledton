//
// Created by per on 09.10.20.
//

#ifndef LIVE_GRAPH_TWO_ALGORITHMS_H
#define LIVE_GRAPH_TWO_ALGORITHMS_H

#include <memory>
#include <vector>

#include "data-structures/ToplogyInterface.h"
#include "Driver.h"


using namespace std;
class Algorithms {
public:
  static vector<uint> bfs(Driver& driver, TopologyInterface& ds, vertex_id_t start_vertex);

    static uint traversed_vertices(TopologyInterface& ds, vector<uint>& vector);

private:
    static vector<uint> bfs_batched_interface(Driver& driver, TopologyInterface& ds, vertex_id_t start_vertex);
    static vector<uint> bfs_single_edge_interface(Driver& driver, TopologyInterface& ds, vertex_id_t start_vertex);
};


#endif //LIVE_GRAPH_TWO_ALGORITHMS_H
