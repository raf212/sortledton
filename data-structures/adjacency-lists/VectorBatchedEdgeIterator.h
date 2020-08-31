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

    bool has_next() override { bool ret = hn; hn = false; return ret;  }
    ContiguousEdgeBatch& next() override { return batch; }

    ContiguousEdgeBatch batch;
private:
    bool hn = true;
};


#endif //LIVE_GRAPH_TWO_VECTORBATCHEDEDGEITERATOR_H
