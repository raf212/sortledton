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

    static unordered_map<vertex_id_t, size_t> neighbourhood_2(Driver& driver, TopologyInterface &ds, const vector<vertex_id_t> &sources, bool raw_neighbourhood);

    static vector<uint> bfs(Driver& driver, TopologyInterface& ds, vertex_id_t start_vertex, bool run_on_raw_neighbourhood,
                            bool aquire_locks);
    static vector<uint> bfs(Driver& driver, TopologyInterface& ds, vertex_id_t start_vertex) { return bfs(driver, ds, start_vertex, false, false); };
    static vector<float> page_rank(Driver& driver, TopologyInterface& ds, bool run_on_raw_neighbourhood);

    static uint traversed_vertices(TopologyInterface& ds, vector<uint>& vector);

private:
    static vector<uint> bfs_batched_interface(Driver& driver, TopologyInterface& ds, vertex_id_t start_vertex);
    static vector<uint> bfs_single_edge_interface(Driver& driver, TopologyInterface& ds, vertex_id_t start_vertex);
    static vector<uint> bfs_raw_neighbourhood(Driver& driver, TopologyInterface& ds, vertex_id_t start_vertex,
                                              bool aquire_locks);
};


#endif //LIVE_GRAPH_TWO_ALGORITHMS_H
