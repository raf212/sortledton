//
// Created by per on 09.10.20.
//

#ifndef LIVE_GRAPH_TWO_TWONEIGHBOURSOURCESELECTOR_H
#define LIVE_GRAPH_TWO_TWONEIGHBOURSOURCESELECTOR_H

#include "Driver.h"
#include "ToplogyInterface.h"

class TwoNeighbourSourceSelector {
public:
    TwoNeighbourSourceSelector(const SortedCSRDataSource &src) : graph(src), distribution(0, src.vertex_count() - 1) {
      gen = mt19937(42);
    };

    /**
     * Selects <number> determistically, random sources from ds. All sources have at least one neighbour.
     *
     * @param number the amount of sources to return
     * @return
     */
    vector<vertex_id_t> get_sources(uint number);
private:
    const SortedCSRDataSource& graph;

    mt19937 gen;
    uniform_int_distribution<vertex_id_t> distribution;
};


#endif //LIVE_GRAPH_TWO_TWONEIGHBOURSOURCESELECTOR_H
