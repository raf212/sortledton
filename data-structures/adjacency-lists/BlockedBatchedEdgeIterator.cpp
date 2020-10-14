//
// Created by per on 09.09.20.
//

#include "BlockedBatchedEdgeIterator.h"

void BlockedBatchedEdgeIterator::initialize(BlockHeader *head) {
  prefetch(head);
  current_block = head;

  if (prefetch_ahead == 0) {
    last_prefetched = nullptr;
  } else {
    last_prefetched = current_block;
    for (auto i = 0; i < prefetch_ahead && last_prefetched != nullptr; i++) {
      last_prefetched = last_prefetched->next;
    }

    prefetch(last_prefetched);
  }
}

bool BlockedBatchedEdgeIterator::has_next() {
  return current_block != nullptr;
}

ContiguousEdgeBatch &BlockedBatchedEdgeIterator::next() {
  batch.start = current_block->data;
  batch.size = current_block->size;

  current_block = current_block->next;
  if (last_prefetched != nullptr) {
    prefetch(last_prefetched->next);
    last_prefetched = last_prefetched->next;
  }

  return batch;
}

void BlockedBatchedEdgeIterator::prefetch(BlockHeader *addr) {
  __builtin_prefetch((void*) addr, 0, 2);
}
