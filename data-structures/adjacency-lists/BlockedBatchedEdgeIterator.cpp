//
// Created by per on 09.09.20.
//

#include "BlockedBatchedEdgeIterator.h"

void BlockedBatchedEdgeIterator::initialize(BlockHeader *head) {
  current_block = head;
  prefetch(current_block);
}

bool BlockedBatchedEdgeIterator::has_next() {
  return current_block != nullptr;
}

ContiguousEdgeBatch &BlockedBatchedEdgeIterator::next() {
  prefetch(current_block->next);
  batch.start = current_block->data;
  batch.size = current_block->size;

  current_block = current_block->next;

  return batch;
}

void BlockedBatchedEdgeIterator::prefetch(BlockHeader * block) {
  __builtin_prefetch((void*) block, 0, 3);

}
