//
// Created by per on 28.09.20.
//

#include <cstring>
#include <cassert>
#include "BlockedSkipListAdjacencyLists.h"
#include "adjacency-lists/BlockedBatchedEdgeIterator.h"

void BlockedSkipListAdjacencyLists::neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) {
  static_cast<BlockedBatchedEdgeIterator &>(iter).initialize(adjacency_index[src]);
}

void BlockedSkipListAdjacencyLists::bulkload(const SortedCSRDataSource &src) {
  adjacency_index.reserve(src.vertex_count());
  vector<mutex> m(src.vertex_count());
  vertex_mutices.swap(m);

  for (int i = 0; i < src.vertex_count(); i++) {
    auto start = src.adjacency_lists.data() + src.adjacency_index[i];
    auto end = &src.adjacency_lists[0] + src.adjacency_index[i + 1];

    vector<dst_t> shuffled_src(start, end);
    if (unordered) {
      shuffle(shuffled_src.begin(), shuffled_src.end(), std::mt19937(std::random_device()()));
    }

    SkipListHeader *head_block = write_to_blocks(shuffled_src.data(), shuffled_src.data() + shuffled_src.size());

    adjacency_index.push_back(head_block);
  }
}

SkipListHeader *BlockedSkipListAdjacencyLists::write_to_blocks(const dst_t *start, const dst_t *end) {
  auto size = end - start;
  if (size == 0) {
    return nullptr;
  } else {
    SkipListHeader *first_block = nullptr;
    SkipListHeader *last_block = nullptr;

    size_t block_fill = block_size * bulk_load_fill_rate;

    while (start < end) {
      SkipListHeader *block = (SkipListHeader *) malloc(memory_block_size());
      block->data = (dst_t *) ((char *) block + skip_list_header_size());

      block->next = nullptr;

      if (first_block == nullptr) {
        first_block = block;
      } else {
        last_block->next = block;
      }
      last_block = block;

      block->size = block_fill < end - start ? block_fill : end - start;

      dst_t *block_data = block->data;
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
      dst_t *j = i->data;

//      dst_t min = -1;
//      dst_t max = 0;
//      while (j < GET_DATA(i) + i->size) {
//        min = std::min(min, *j);
//        max = std::max(max, *j);
//        j++;
//      }

      assert(*i->data == i->min);
      assert(*(i->data + i->size - 1) == i->max);

      all_size += i->size;
      i = (SkipListHeader *) i->next;
    }

    i = first_block;
    vector<SkipListHeader *> level_blocks(levels, first_block);
    while (i != nullptr) {
      auto height = get_height();
      for (int l = 0; l < levels; l++) {
        i->next_levels[l] = nullptr;
        if (i != first_block && l < height) {
          level_blocks[l]->next_levels[l] = i;
          level_blocks[l] = i;
        }
      }
      i = (SkipListHeader *) i->next;
    }

    return first_block;
  }
}

void BlockedSkipListAdjacencyLists::insert_edge(edge_t edge) {
  if (unordered) {
    throw NotImplemented();
  }

  SkipListHeader *adjacency_list = adjacency_index[edge.src];

  // Insert to empty list
  if (adjacency_list == nullptr) {
    SkipListHeader *first_block = (SkipListHeader *) malloc(memory_block_size());
    first_block->data = (dst_t *) ((char *) first_block + skip_list_header_size());
    *(first_block->data) = edge.dst;

    first_block->size = 1;
    first_block->next = nullptr;
    first_block->max = edge.dst;
    first_block->min = edge.dst;

    for (int l = 0; l < levels; l++) {
      first_block->next_levels[l] = nullptr;
    }

    adjacency_index[edge.src] = first_block;
  } else {
    vector<SkipListHeader *> blocks_per_level(levels);
    find_block(adjacency_list, edge.dst, blocks_per_level);

    auto i = blocks_per_level[0];

    // Handle a full block
    if (i->size == block_size) {
      auto data = i->data;
      const auto split = block_size / 2;

      SkipListHeader *new_block = (SkipListHeader *) malloc(memory_block_size());
      new_block->data = (dst_t *) ((char *) new_block + skip_list_header_size());

      new_block->size = split;
      i->size = split;

      new_block->next = i->next;
      i->next = new_block;

      new_block->max = i->max;
      new_block->min = *(data + split);
      i->max = *(data + split - 1);


      memcpy((void *) new_block->data, (void *) (data + split), split * sizeof(dst_t));

      auto height = get_height();
      for (int l = 0; l < levels; l++) {
        if (l < height) {
          new_block->next_levels[l] = blocks_per_level[l]->next_levels[l];
          blocks_per_level[l]->next_levels[l] = new_block;
          blocks_per_level[l] = new_block;
        } else {
          new_block->next_levels[l] = nullptr;
        }
      }

      // Recursive call of max depth 1.
      insert_edge(edge);
    } else {
      auto data = i->data + i->size - 1;

      while (edge.dst < *data && i->data <= data) {
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

/**
 * Finds the block that contains the upper bound of element.
 *
 * Returns blocks for all levels in the out parameter blocks.
 *
 * The block for level 0 is the block containing the upper bound while all other
 * blocks are lower bounds.
 *
 * @param pHeader
 * @param element
 * @param blocks
 */
SkipListHeader *
BlockedSkipListAdjacencyLists::find_block(SkipListHeader *pHeader, dst_t element, vector<SkipListHeader *> &blocks) {
  for (int l = levels - 1; 0 <= l; l--) {
    while (pHeader->next_levels[l] != nullptr && pHeader->next_levels[l]->max < element) {
      pHeader = pHeader->next_levels[l];
    }
    if (l == 0 && pHeader->next != nullptr && pHeader->max < element) {
      blocks[0] = pHeader->next_levels[0];
    } else {
      blocks[l] = pHeader;
    }
  }
  return blocks[0];
}

bool BlockedSkipListAdjacencyLists::has_edge(edge_t edge) {
  vector<SkipListHeader *> v(levels);
  auto block = find_block(adjacency_index[edge.src], edge.dst, v);

  auto last = block->data + block->size;
  return find(block->data, last, edge.dst) != last;
}

bool BlockedSkipListAdjacencyLists::insert_safe(edge_t edge) {
  vertex_mutices[edge.src].lock();
  insert_edge(edge);
  vertex_mutices[edge.src].unlock();
}



