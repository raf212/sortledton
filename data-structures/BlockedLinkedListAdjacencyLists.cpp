//
// Created by per on 09.09.20.
//

#include <algorithm>
#include <cstring>
#include <data-structures/adjacency-lists/BlockedBatchedEdgeIterator.h>
#include "BlockedLinkedListAdjacencyLists.h"

void BlockedLinkedListAdjacencyLists::bulkload(const SortedCSRDataSource &src) {
  adjacency_index.reserve(src.vertex_count());

  for (int i = 0; i < src.vertex_count(); i++) {
    auto start = src.adjacency_lists.data() + src.adjacency_index[i];
    auto end = &src.adjacency_lists[0] + src.adjacency_index[i + 1];

    vector<dst_t> shuffled_src(start, end);
//    if (unordered) {
//      shuffle(start, end, std::mt19937(std::random_device()()));
//    }  TODO make this compile

    BlockHeader* head_block = write_to_blocks(shuffled_src.data(), shuffled_src.data() + shuffled_src.size());

    adjacency_index.push_back(head_block);
  }
}

BlockHeader *BlockedLinkedListAdjacencyLists::write_to_blocks(const dst_t *start, const dst_t *end) {
  auto size = end - start;
  if (size == 0) {
    return nullptr;
  } else {
    BlockHeader* first_block = nullptr;
    BlockHeader* last_block = nullptr;

    while (start < end) {
      BlockHeader* block = (BlockHeader*) pool.get_block();
      block->next = nullptr;

      if (first_block == nullptr) {
        first_block = block;
      } else {
        last_block->next = block;
      }
      last_block = block;

      block->size = block_size < end - start ? block_size : end - start;

      dst_t* block_data = GET_DATA(block);
      memcpy((void*) block_data, start, block->size * sizeof(dst_t));

      start += block_size;
    }

    auto all_size = 0;
    auto i = first_block;
    while (i != nullptr) {
      all_size += i->size;
      i = i->next;
    }

    return first_block;
  }
}

void BlockedLinkedListAdjacencyLists::neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) {
  static_cast<BlockedBatchedEdgeIterator&>(iter).initialize(adjacency_index[src]);
}
