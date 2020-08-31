//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_ADJACENCYLIST_H
#define LIVE_GRAPH_TWO_ADJACENCYLIST_H

#include "../../data_types.h"
#include "BatchedEdgeIterator.h"

class AdjacencyList {
public:
    virtual void insert_edge(dst_t edge) = 0;
    virtual void delete_edge(dst_t edge) = 0;

    virtual BatchedEdgeIterator& iterator() = 0;
    virtual void intersect(AdjacencyList& other, vector<dst_t>& out) = 0;
};


#endif //LIVE_GRAPH_TWO_ADJACENCYLIST_H
