//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_VECTORADJACENCYLISTS_H
#define LIVE_GRAPH_TWO_VECTORADJACENCYLISTS_H


#include "AdjacencyLists.h"

class VectorAdjacencyLists : public AdjacencyLists {
public:
    explicit VectorAdjacencyLists(bool unordered) : unordered(unordered) {}

protected:
    unique_ptr<AdjacencyList> construct_adjacency_list(vector<dst_t>::const_iterator begin,
            vector<dst_t>::const_iterator end) override;
private:
    bool unordered;
};


#endif //LIVE_GRAPH_TWO_VECTORADJACENCYLISTS_H
