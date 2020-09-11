//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_VECTORBATCHEDEDGEITERATOR_H
#define LIVE_GRAPH_TWO_VECTORBATCHEDEDGEITERATOR_H

#include "BatchedEdgeIterator.h"
#include "ContiguousEdgeBatch.h"
#include "ContigiousBlockIterator.h"

class VectorBatchedEdgeIterator: public ContigiousBlockIterator {
public:
    VectorBatchedEdgeIterator() : batch(nullptr, 0) {};

    bool has_next() override { return hn;  }
    ContiguousEdgeBatch& next() override { hn = false; return batch; }

    void initialize(dst_t* start, size_t size) {
      hn = true;
      batch.start = start;
      batch.size = size;
    }

private:
    ContiguousEdgeBatch batch;
    bool hn = true;
};


#endif //LIVE_GRAPH_TWO_VECTORBATCHEDEDGEITERATOR_H
