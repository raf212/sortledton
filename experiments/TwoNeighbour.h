//
// Created by per on 23.10.20.
//

#ifndef LIVE_GRAPH_TWO_TWONEIGHBOUR_H
#define LIVE_GRAPH_TWO_TWONEIGHBOUR_H

#include <unordered_map>
#include <data_types.h>
#include <data-structures/ToplogyInterface.h>
#include "Driver.h"

class TwoNeighbour {
public:
    static unordered_map<vertex_id_t, size_t> neighbourhood_2_batched_interface(Driver& driver, TopologyInterface &ds, const vector<vertex_id_t> &sources);
    static unordered_map<vertex_id_t, size_t> neighbourhood_2_raw_neighbourhood(Driver& driver, TopologyInterface &ds, const vector<vertex_id_t> &sources);
};


#endif //LIVE_GRAPH_TWO_TWONEIGHBOUR_H
