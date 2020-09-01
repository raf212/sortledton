//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_BATCHEDEDGEITERATOR_H
#define LIVE_GRAPH_TWO_BATCHEDEDGEITERATOR_H

#include "EdgeBatch.h"

class BatchedEdgeIterator {
public:
    virtual bool has_next() = 0;
    virtual EdgeBatch& next() = 0;
};


#endif //LIVE_GRAPH_TWO_BATCHEDEDGEITERATOR_H
