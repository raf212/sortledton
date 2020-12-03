//
// Created by per on 03.12.20.
//

#ifndef LIVE_GRAPH_TWO_PAGERANK_H
#define LIVE_GRAPH_TWO_PAGERANK_H

#include <vector>

#include "Driver.h"
#include <data-structures/ToplogyInterface.h>
#include <data-structures/adjacency-lists/BlockedBatchedEdgeIterator.h>


class PageRank {
    static constexpr float damping_factor = 0.85;

public:
    static vector<float> page_rank_batched_interface(Driver& driver, TopologyInterface& ds, int max_iters, double epsilon = 0);

};


#endif //LIVE_GRAPH_TWO_PAGERANK_H
