//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_VECTORADJACENCYLIST_H
#define LIVE_GRAPH_TWO_VECTORADJACENCYLIST_H

#include <vector>
#include <memory>

#include "../../data_types.h"
#include "AdjacencyList.h"
#include "ContiguousEdgeBatch.h"
#include "VectorBatchedEdgeIterator.h"

using namespace std;

class VectorAdjacencyList : public AdjacencyList {
public:
    explicit VectorAdjacencyList(vector<dst_t> src) : neighbourhood(src) {
      // Reserve 10% empty space for insert experiment.
      neighbourhood.reserve(neighbourhood.size() + neighbourhood.size() * 0.1);
    };

    void initialize_iterator(BatchedEdgeIterator& iterator) override;
    void intersect(AdjacencyList& other, vector<dst_t>& out) override;

    void insert_edge(dst_t edge) override;
    void delete_edge(dst_t edge) override;

    bool has_neighbour(dst_t n) override;

private:
    vector<dst_t> neighbourhood;
};


#endif //LIVE_GRAPH_TWO_VECTORADJACENCYLIST_H
