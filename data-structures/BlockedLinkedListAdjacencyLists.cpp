//
// Created by per on 09.09.20.
//

#include <algorithm>
#include <cstring>

#include "BlockedLinkedListAdjacencyLists.h"
#include <cassert>

void BlockedLinkedListAdjacencyLists::bulkload(const SortedCSRDataSource &src) {
  if (size_in_index) {
    adjacency_index.reserve(src.vertex_count() * 2);
  } else {
    adjacency_index.reserve(src.vertex_count());
  }

  for (uint i = 0; i < src.vertex_count(); i++) {
    auto start = src.adjacency_lists.data() + src.adjacency_index[i];
    auto end = &src.adjacency_lists[0] + src.adjacency_index[i + 1];

    vector<dst_t> shuffled_src(start, end);
    if (unordered) {
      shuffle(shuffled_src.begin(), shuffled_src.end(), std::mt19937(std::random_device()()));
    }

    BlockHeader *head_block = write_to_blocks(shuffled_src.data(), shuffled_src.data() + shuffled_src.size());

    if (head_block != nullptr) {
//      assert((long) head_block % 64 == 0);
//      assert((sizeof(*head_block) + block_size * 4) % 64 == 0);
    }

    adjacency_index.push_back(head_block);
    if (size_in_index) {
      adjacency_index.push_back((BlockHeader *) shuffled_src.size());
    }
  }
}

BlockHeader *BlockedLinkedListAdjacencyLists::write_to_blocks(const dst_t *start, const dst_t *end) {
  uint size = end - start;
  if (size == 0) {
    return nullptr;
  } else if (size < block_size) {  // Fits into one block
    uint nearest_block_size = round_up_power_of_two(size);
    dst_t* adjacency_list = (dst_t *) malloc((nearest_block_size + 1) * sizeof(dst_t));

    adjacency_list[0] = size;
    memcpy((void *) &adjacency_list[1], (void *) start, size * sizeof(dst_t));

    long tagged_pointer = (long) adjacency_list * -1;

    return (BlockHeader*) tagged_pointer;
  } else {
    BlockHeader *first_block = nullptr;
    BlockHeader *last_block = nullptr;

    while (start < end) {
      size_t data_size = end - start;

      uint chosen_pool = 0;
      for (uint i = 1; i < pools.size(); i++) {
        if (pool_sizes[i] <= data_size) {
          chosen_pool = i;
        }
      }
      if (chosen_pool + 1 < pools.size()) {
        chosen_pool++;   // Choose the first pool which can fit all data
      }
      size_t block_capacity = pool_sizes[chosen_pool] * bulk_load_fill_rate;

      BlockHeader *block = (BlockHeader *) malloc(pool_sizes[chosen_pool] * sizeof(dst_t) + sizeof(BlockHeader));
      block->data = (dst_t*) ((char*) block + sizeof(BlockHeader));
      block->next = nullptr;

      if (first_block == nullptr) {
        first_block = block;
      } else {
        last_block->next = block;
      }
      last_block = block;

      block->size = block_capacity < (uint) (end - start) ? block_capacity : end - start;

      dst_t *block_data = block->data;
      memcpy((void *) block_data, (void *) start, block->size * sizeof(dst_t));

      if (!unordered) {
        block->min = *block_data;
        block->max = *(block_data + block->size - 1);
      } else {
        block->min = -1;
        block->max = -1;
      }

      start += block->size;
    }

    // TODO debug code
    auto all_size = 0;
    auto list_length = 0;
    auto i = first_block;

    while (i != nullptr) {
//      dst_t* j = i->data;

//      dst_t min = -1;
//      dst_t max = 0;
//      while (j < GET_DATA(i) + i->size) {
//        min = std::min(min, *j);
//        max = std::max(max, *j);
//        j++;
//      }

      assert(*i->data == i->min);
      assert(*(i->data + i->size - 1) == i->max);

      if (i->size != block_size) {
        assert(i->next == nullptr );
      }

      list_length++;
      all_size += i->size;
      i = i->next;
    }

    if (size < block_size && 0u < size) {
      assert(list_length == 1);
    }

    return first_block;
  }
}

bool BlockedLinkedListAdjacencyLists::insert_edge(edge_t edge) {

    throw NotImplemented(); // Does not work for new 1 block sized lists

  if (pools.size() != 1) {
    throw NotImplemented("Cannot insert new edges because capacity is not saved yet.");
  }

  BlockHeader *adjacency_list = adjacency_index[edge.src];

  // Insert to empty list
  if (adjacency_list == nullptr) {
    BlockHeader* first_block = (BlockHeader*) pools[0].get_block();
    first_block->data = (dst_t*) ((char*) first_block + sizeof(BlockHeader));

    *first_block->data = edge.dst;

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
      auto data = i->data;
      const auto split = block_size / 2;

      BlockHeader* new_block = (BlockHeader*) pools[0].get_block();
      new_block->data = (dst_t*) ((char*) new_block + sizeof(BlockHeader));

      new_block->size = split;
      i->size = split;

      new_block->next = i->next;
      i->next = new_block;

      new_block->max = i->max;
      new_block->min = *(data + split);
      i->max = *(data + split - 1);


      memcpy((void*) new_block->data, (void*) (data + split), split * sizeof(dst_t));

      // Recursive call of max depth 1.
      insert_edge(edge);

    } else {
      auto data = i->data + i->size - 1;

      while (edge.dst < *data &&  i->data <= data) {
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

bool BlockedLinkedListAdjacencyLists::has_edge_p(edge_t e) {
  throw NotImplemented();
}

size_t BlockedLinkedListAdjacencyLists::neighbourhood_size_p(vertex_id_t src) {
  if (size_in_index) {
    return (size_t) adjacency_index[src * 2 + 1];
  } else {
    long tagged_pointer = (long) adjacency_index[src];
    if (tagged_pointer < 0) {
      return (size_t) ((dst_t*) (-1 * tagged_pointer))[0];
    } else {
      throw NotImplemented();
    }
  }
}

void *BlockedLinkedListAdjacencyLists::raw_neighbourhood(vertex_id_t src) {
  if (size_in_index) {
    return adjacency_index[src * 2];
  } else {
    return adjacency_index[src];
  }
}

size_t BlockedLinkedListAdjacencyLists::vertex_count() {
  if (size_in_index) {
    return adjacency_index.size() / 2;
  } else {
    return adjacency_index.size();
  }
}
