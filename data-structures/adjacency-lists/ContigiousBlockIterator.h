//
// Created by per on 10.09.20.
//

#ifndef LIVE_GRAPH_TWO_CONTIGIOUSBLOCKITERATOR_H
#define LIVE_GRAPH_TWO_CONTIGIOUSBLOCKITERATOR_H


#include "BatchedEdgeIterator.h"
#include "ContiguousEdgeBatch.h"

// TODO correct spelling mistake
class ContigiousBlockIterator : public BatchedEdgeIterator {
public:
  virtual ContiguousEdgeBatch& next() override = 0;
};


#endif //LIVE_GRAPH_TWO_CONTIGIOUSBLOCKITERATOR_H
