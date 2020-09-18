//
// Created by per on 09.09.20.
//

#include <algorithm>
#include <cstring>

#include "BlockedLinkedListAdjacencyLists.h"
#include <data-structures/adjacency-lists/BlockedBatchedEdgeIterator.h>
#include <cassert>

void BlockedLinkedListAdjacencyLists::bulkload(const SortedCSRDataSource &src) {
  adjacency_index.reserve(src.vertex_count());

  for (int i = 0; i < src.vertex_count(); i++) {
    auto start = src.adjacency_lists.data() + src.adjacency_index[i];
    auto end = &src.adjacency_lists[0] + src.adjacency_index[i + 1];

    vector<dst_t> shuffled_src(start, end);
    if (unordered) {
      shuffle(shuffled_src.begin(), shuffled_src.end(), std::mt19937(std::random_device()()));
    }

    BlockHeader *head_block = write_to_blocks(shuffled_src.data(), shuffled_src.data() + shuffled_src.size());

    adjacency_index.push_back(head_block);
  }
}

BlockHeader *BlockedLinkedListAdjacencyLists::write_to_blocks(const dst_t *start, const dst_t *end) {
  auto size = end - start;
  if (size == 0) {
    return nullptr;
  } else {
    BlockHeader *first_block = nullptr;
    BlockHeader *last_block = nullptr;

    size_t block_fill = block_size * bulk_load_fill_rate;

    while (start < end) {
      BlockHeader *block = (BlockHeader *) pool.get_block();
      block->next = nullptr;

      if (first_block == nullptr) {
        first_block = block;
      } else {
        last_block->next = block;
      }
      last_block = block;

      block->size = block_fill < end - start ? block_fill : end - start;

      dst_t *block_data = GET_DATA(block);
      memcpy((void *) block_data, (void *) start, block->size * sizeof(dst_t));

      if (!unordered) {
        block->min = *block_data;
        block->max = *(block_data + block->size - 1);
      } else {
        block->min = -1;
        block->max - -1;
      }

      start += block->size;
    }

    // TODO debug code
    auto all_size = 0;
    auto i = first_block;

    while (i != nullptr) {
      dst_t* j = GET_DATA(i);

//      dst_t min = -1;
//      dst_t max = 0;
//      while (j < GET_DATA(i) + i->size) {
//        min = std::min(min, *j);
//        max = std::max(max, *j);
//        j++;
//      }

      assert(*GET_DATA(i) == i->min);
      assert(*(GET_DATA(i) + i->size - 1) == i->max);

      all_size += i->size;
      i = i->next;
    }

    return first_block;
  }
}

void BlockedLinkedListAdjacencyLists::neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) {
  static_cast<BlockedBatchedEdgeIterator &>(iter).initialize(adjacency_index[src]);
}

void BlockedLinkedListAdjacencyLists::insert_edge(edge_t edge) {
  if (unordered) {
    throw NotImplemented();
  }

  BlockHeader *adjacency_list = adjacency_index[edge.src];

  // Insert to empty list
  if (adjacency_list == nullptr) {
    BlockHeader* first_block = (BlockHeader*) pool.get_block();
    *(GET_DATA(first_block)) = edge.dst;

    first_block->size = 1;
    first_block->next = nullptr;
    first_block->max = edge.dst;
    first_block->min = edge.dst;

    adjacency_index[edge.src] = first_block;
  } else {
    auto i = adjacency_list;

    // Find block to insert into
    while (i->next != nullptr && i->max < edge.dst) {
      i = i->next;
    }

    // Handle a full block
    if (i->size == block_size) {
      auto data = GET_DATA(i);
      const auto split = block_size / 2;

      BlockHeader* new_block = (BlockHeader*) pool.get_block();

      new_block->size = split;
      i->size = split;

      new_block->next = i->next;
      i->next = new_block;

      new_block->max = i->max;
      new_block->min = *(data + split);
      i->max = *(data + split - 1);


      memcpy((void*) GET_DATA(new_block), (void*) (data + split), split * sizeof(dst_t));

      // Recursive call of max depth 1.
      insert_edge(edge);

    } else {
      auto data = GET_DATA(i) + i->size - 1;

      while (edge.dst < *data &&  GET_DATA(i) <= data) {
        *(data + 1) = *data;
        data--;
      }

      *(data + 1) = edge.dst;

      i->size++;
      i->min = std::min(edge.dst, i->min);
      i->max = std::max(edge.dst, i->max);
    }
  }
}
