//
// Created by per on 09.09.20.
//

#ifndef LIVE_GRAPH_TWO_BLOCKEDBATCHEDEDGEITERATOR_H
#define LIVE_GRAPH_TWO_BLOCKEDBATCHEDEDGEITERATOR_H


#include "BatchedEdgeIterator.h"

#include "../BlockedLinkedListAdjacencyLists.h"
#include "ContiguousEdgeBatch.h"
#include "ContigiousBlockIterator.h"

// TODO rename the iterator families
class BlockedBatchedEdgeIterator : public ContigiousBlockIterator {
public:
    BlockedBatchedEdgeIterator() : batch(nullptr, 0) {};
    void initialize(BlockHeader* head) {
      current_block = head;
    };

    bool has_next() override { return current_block != nullptr; };
    ContiguousEdgeBatch& next() override {
      batch.start = GET_DATA(current_block);
      batch.size = current_block->size;

      current_block = current_block->next;
      return batch;
    }

private:
    BlockHeader* current_block;
    ContiguousEdgeBatch batch;
};


#endif //LIVE_GRAPH_TWO_BLOCKEDBATCHEDEDGEITERATOR_H
