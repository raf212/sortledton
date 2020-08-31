//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_VECTORBATCHEDEDGEITERATOR_H
#define LIVE_GRAPH_TWO_VECTORBATCHEDEDGEITERATOR_H

#include "BatchedEdgeIterator.h"
#include "ContiguousEdgeBatch.h"

class VectorBatchedEdgeIterator: public BatchedEdgeIterator {
public:
    VectorBatchedEdgeIterator() : batch(nullptr, 0) {};

    bool has_next() override { return false; }
    ContiguousEdgeBatch& next() override { return batch; }

    ContiguousEdgeBatch batch;
};


#endif //LIVE_GRAPH_TWO_VECTORBATCHEDEDGEITERATOR_H
