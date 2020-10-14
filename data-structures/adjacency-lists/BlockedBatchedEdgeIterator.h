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
    BlockedBatchedEdgeIterator(uint prefetch) : prefetch_ahead(prefetch), batch(nullptr, 0) {};

    void initialize(BlockHeader* head);

    bool has_next() override;

    ContiguousEdgeBatch& next() override;

private:
    BlockHeader* current_block;
    ContiguousEdgeBatch batch;

    const uint prefetch_ahead;

    BlockHeader* last_prefetched;

    inline void prefetch(BlockHeader* addr);
};


#endif //LIVE_GRAPH_TWO_BLOCKEDBATCHEDEDGEITERATOR_H
