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

void BlockedBatchedEdgeIterator::initialize(dst_t *data, size_t size) {
  current_block = &single_block_buffer;
  current_block->data = data;
  current_block->size = size;
  current_block->next = nullptr;
  current_block->min = -1;
  current_block->max = -1;
}
