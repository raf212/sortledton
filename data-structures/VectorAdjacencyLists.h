//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_VECTORADJACENCYLISTS_H
#define LIVE_GRAPH_TWO_VECTORADJACENCYLISTS_H


#include "AdjacencyLists.h"

class VectorAdjacencyLists : public AdjacencyLists {
protected:
    unique_ptr<AdjacencyList> construct_adjacency_list(vector<dst_t>& src) override;
};


#endif //LIVE_GRAPH_TWO_VECTORADJACENCYLISTS_H
