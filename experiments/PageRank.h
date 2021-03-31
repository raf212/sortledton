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
public:
    static vector<pair<vertex_id_t, double>> page_rank(Driver& driver, TopologyInterface& ds, int iterations, double damping_factor, bool use_raw_neighbourhood, bool use_gapbs);

private:
    static vector<double> page_rank_batched_interface(Driver& driver, TopologyInterface& ds, int iterations, double damping_factor);
    static vector<double> page_rank_raw_neighbourhood(Driver& driver, TopologyInterface& ds, int iterations, double damping_factor);
    static vector<double> page_rank_bs(Driver& driver, TopologyInterface& ds, int iterations, double damping_factor);
};


#endif //LIVE_GRAPH_TWO_PAGERANK_H
