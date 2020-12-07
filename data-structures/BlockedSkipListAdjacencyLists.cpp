//
// Created by per on 28.09.20.
//

#include <cstring>
#include <cassert>
#include <data-structures/adjacency-lists/VectorBatchedEdgeIterator.h>
#include "BlockedSkipListAdjacencyLists.h"
#include "adjacency-lists/BlockedBatchedEdgeIterator.h"

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#define intersect(start_a, end_a, start_b, end_b, out) while (start_a < end_a && start_b < end_b) { \
  const dst_t a = *start_a;                                                                        \
  const dst_t b = *start_b; \
  if (a == b) {\
    *out_iterator = *start_a;\
    start_a++;\
    start_b++;\
  } else if (a < b) {\
    start_a++;\
  } else {\
    start_b++;\
  }\
}


void BlockedSkipListAdjacencyLists::neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) {
  switch (get_set_type(src)) {
    case SKIP_LIST:
      return static_cast<BlockedBatchedEdgeIterator &>(iter).initialize((SkipListHeader *) adjacency_index[2 * src]);
    case SINGLE_BLOCK:
      return static_cast<BlockedBatchedEdgeIterator &>(iter).initialize((dst_t *) adjacency_index[2 * src],
                                                                        (size_t) adjacency_index[2 * src + 1]);
  }
}

void BlockedSkipListAdjacencyLists::bulkload(const SortedCSRDataSource &src) {
  adjacency_index.reserve(src.vertex_count() * 2);
  vector<mutex> m(src.vertex_count());
  vertex_mutices.swap(m);

  for (int i = 0; i < src.vertex_count(); i++) {
    auto start = src.adjacency_lists.data() + src.adjacency_index[i];
    auto end = &src.adjacency_lists[0] + src.adjacency_index[i + 1];

    vector<dst_t> shuffled_src(start, end);
    if (unordered) {
      shuffle(shuffled_src.begin(), shuffled_src.end(), std::mt19937(std::random_device()()));
    }

    void *head_block = write_to_blocks(shuffled_src.data(), shuffled_src.data() + shuffled_src.size());

    adjacency_index.push_back(head_block);
    adjacency_index.push_back((void *) shuffled_src.size());
  }
}

void *BlockedSkipListAdjacencyLists::write_to_blocks(const dst_t *start, const dst_t *end) {
  auto size = end - start;
  if (size == 0) {
    return nullptr;
  } else if (size <= block_size) {
    size_t block_size = round_up_power_of_two(size);
    dst_t *block = (dst_t *) malloc(block_size * sizeof(dst_t));

    memcpy((void *) block, (void *) start, size * sizeof(dst_t));

    return block;
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

  void *adjacency_list = adjacency_index[2 * edge.src];

  // Insert to empty list
  if (adjacency_list == nullptr) {
    return insert_empty(edge);
  } else {
    switch (get_set_type(edge.src)) {
      case SINGLE_BLOCK: {
        return insert_single_block(edge);
      }
      case SKIP_LIST: {
        return insert_skip_list(edge);
      }
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
 * @param blocks vector with one entry for each level
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

/**
 * Finds the block which contains element if element is in the list.
 *
 * Does not keep track of the "path" of elements leading there as this is not necessary for intersections.
 *
 * @param pHeader
 * @param element
 * @return the block potentially containing element or nullptr if element is bigger than all elements in the list.
 */
SkipListHeader *
BlockedSkipListAdjacencyLists::find_block1(SkipListHeader *pHeader, dst_t element) {
  for (int l = levels - 1; 0 <= l; l--) {
    while (pHeader->next_levels[l] != nullptr && pHeader->next_levels[l]->max < element) {
      pHeader = pHeader->next_levels[l];
    }
  }
  if (pHeader->max < element) {
    return pHeader->next_levels[0];
  } else {
    return pHeader;
  }
}

bool BlockedSkipListAdjacencyLists::has_edge(edge_t edge) {
  switch (get_set_type(edge.src)) {
    case SKIP_LIST: {
      vector<SkipListHeader *> v(levels);
      auto block = find_block((SkipListHeader *) adjacency_index[2 * edge.src], edge.dst, v);

      auto last = block->data + block->size;
      return find(block->data, last, edge.dst) != last;
    }
    case SINGLE_BLOCK: {
      auto start = (dst_t *) adjacency_index[2 * edge.src];
      auto end = (dst_t *) adjacency_index[2 * edge.src] + (size_t) adjacency_index[2 * edge.src + 1];
      return find(start, end, edge.dst) != end;
    }
  }

}

bool BlockedSkipListAdjacencyLists::insert_safe(edge_t edge) {
  vertex_mutices[edge.src].lock();
  insert_edge(edge);
  vertex_mutices[edge.src].unlock();
  return true;
}

void BlockedSkipListAdjacencyLists::intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  auto s_a = neighbourhood_size(a);
  auto s_b = neighbourhood_size(b);

  out.clear();

  if (s_a == 0 || s_b == 0) {
    return;
  }

  if (s_b < s_a) {
    swap(s_a, s_b);
    swap(a, b);
  }

  if (get_set_type(a) == SINGLE_BLOCK && get_set_type(b) == SINGLE_BLOCK) {
    call_single_single++;
    auto out_iterator = back_inserter(out);
    auto start_a = (dst_t *) raw_neighbourhood(a);
    auto end_a = start_a + neighbourhood_size(a);
    auto start_b = (dst_t *) raw_neighbourhood(b);
    auto end_b = start_b + neighbourhood_size(b);

    intersect(start_a, end_a, start_b, end_b, out_iterator)
  } else if (get_set_type(a) == SINGLE_BLOCK) {
    call_single++;
    auto out_iterator = back_inserter(out);

    auto start_a = (dst_t *) raw_neighbourhood(a);
    auto end_a = start_a + neighbourhood_size(a);

    SkipListHeader *ns_b = (SkipListHeader *) raw_neighbourhood(b);

    if (32 * s_a < s_b) {
      while (start_a < end_a) {
        auto b_block = find_block1(ns_b, *start_a);
        if (b_block == nullptr) {
          return;
        }

        auto start_b = b_block->data;
        auto end_b = start_b + b_block->size;

        if (binary_search(start_b, end_b, *start_a)) {
          *out_iterator = *start_a;
        }
        start_a++;
      }
    } else {
      while (start_a < end_a && ns_b != nullptr) {
        auto start_b = ns_b->data;
        auto end_b = start_b + ns_b->size;

        intersect(start_a, end_a, start_b, end_b, out_iterator)

        ns_b = (SkipListHeader *) ns_b->next;
      }
    }
  } else {
    call_skip++;
    auto out_iterator = back_inserter(out);

    SkipListHeader *ns_a = (SkipListHeader *) raw_neighbourhood(a);
    SkipListHeader *ns_b = (SkipListHeader *) raw_neighbourhood(b);

    if (32 * s_a < s_b) {
      while (ns_a != nullptr) {
        auto start_a = ns_a->data;
        auto end_a = ns_a->data + ns_a->size;

        while (start_a < end_a) {
          auto b_block = find_block1(ns_b, *start_a);
          if (b_block == nullptr) {
            return;
          }
          auto start_b = b_block->data;
          auto end_b = start_b + b_block->size;

          if (binary_search(start_b, end_b, *start_a)) {
            *out_iterator = *start_a;
          }
          start_a++;
        }

        ns_a = (SkipListHeader *) ns_a->next;
      }
    } else {
      auto start_a = ns_a->data;
      auto end_a = ns_a->data + ns_a->size;
      auto start_b = ns_b->data;
      auto end_b = ns_b->data + ns_b->size;
      while (ns_a != nullptr && ns_b != nullptr) {
        intersect(start_a, end_a, start_b, end_b, out_iterator)

        if (start_a == end_a) {
          ns_a = (SkipListHeader *) ns_a->next;
          if (ns_a != nullptr) {
            start_a = ns_a->data;
            end_a = ns_a->data + ns_a->size;
          }
        } else {
          ns_b = (SkipListHeader *) ns_b->next;
          if (ns_b != nullptr) {
            start_b = ns_b->data;
            end_b = ns_b->data + ns_b->size;
          }
        }
      }
    }
  }

}

size_t BlockedSkipListAdjacencyLists::neighbourhood_size(vertex_id_t src) {
  return (size_t) adjacency_index[2 * src + 1];
}

BlockedSkipListAdjacencyLists::BlockedSkipListAdjacencyLists(size_t block_size, size_t levels, bool unordered,
                                                             size_t max_edges, size_t max_vertices) :
        block_size(block_size), unordered(unordered), levels(levels) {
  if (round_up_power_of_two(block_size) != block_size) {
    throw ConfigurationError("Block size needs to be a power of two.");
  }
  level_distribution = binomial_distribution<int>(levels - 1, p);
}

size_t BlockedSkipListAdjacencyLists::vertex_count() {
  return adjacency_index.size() / 2;
}

void *BlockedSkipListAdjacencyLists::raw_neighbourhood(vertex_id_t src) {
  return adjacency_index[2 * src];
}

size_t BlockedSkipListAdjacencyLists::memory_block_size() {
  return block_size * sizeof(dst_t) + sizeof(BlockHeader) + levels * sizeof(SkipListHeader *);
}

size_t BlockedSkipListAdjacencyLists::get_height() {
  return level_distribution(level_generator) + 1;
}

size_t BlockedSkipListAdjacencyLists::skip_list_header_size() const {
  return levels * sizeof(SkipListHeader *) + sizeof(BlockHeader);
}

AdjacencySetType BlockedSkipListAdjacencyLists::get_set_type(vertex_id_t v) {
  if (neighbourhood_size(v) <= block_size) {
    return SINGLE_BLOCK;
  } else {
    return SKIP_LIST;
  }
}

void BlockedSkipListAdjacencyLists::insert_empty(edge_t edge) {
  auto block = (dst_t *) malloc(2 * sizeof(dst_t));
  block[0] = edge.dst;

  adjacency_index[2 * edge.src] = block;
  adjacency_index[2 * edge.src + 1] = (void *) 1;
}

void BlockedSkipListAdjacencyLists::insert_single_block(edge_t edge) {
  auto size = neighbourhood_size(edge.src);
  auto block_capacity = round_up_power_of_two(size);
  auto block = (dst_t *) raw_neighbourhood(edge.src);

  if (size == block_capacity) {  // Block full
    if (size ==
        block_size) {    // Block should be split into 2 skip list blocks, we do this in two steps, convert to SkipListHeader and then by recursion split into two.
      SkipListHeader *new_block = (SkipListHeader *) malloc(memory_block_size());
      new_block->data = (dst_t *) ((char *) new_block + skip_list_header_size());
      new_block->size = size;

      memcpy((void *) new_block->data, (void *) block, size * sizeof(dst_t));

      new_block->min = block[0];
      new_block->max = block[size - 1];

      new_block->next = nullptr;
      for (int l = 0; l < levels; l++) {
        new_block->next_levels[l] = nullptr;
      }

      adjacency_index[edge.src * 2] = new_block;

      free(block);

      return insert_skip_list(edge); // recursive call of depth 2, inefficient could be done with one time less copying.
    } else { // Block full: we double size and copy.
      auto block = (dst_t *) raw_neighbourhood(edge.src);
      dst_t *new_block = (dst_t *) malloc(size * 2 * sizeof(dst_t));

      auto *old_data = block;
      auto *new_data = new_block;
      while (*old_data < edge.dst && old_data < block + size) {
        *new_data = *old_data;
        old_data++;
        new_data++;
      }

      *new_data = edge.dst;
      new_data++;

      while (old_data < block + size) {
        *new_data = *old_data;
        old_data++;
        new_data++;
      }

      free(block);
      adjacency_index[edge.src * 2] = new_block;
      adjacency_index[edge.src * 2 + 1] = (void *) (size + 1);
    }
  } else {  // Insert into block by shifting
    auto data = block + size - 1;

    while (edge.dst < *data && block <= data) {
      *(data + 1) = *data;
      data--;
    }

    *(data + 1) = edge.dst;

    adjacency_index[2 * edge.src + 1] = (void *) ((size_t) adjacency_index[2 * edge.src + 1] + 1);
  }
}

void BlockedSkipListAdjacencyLists::insert_skip_list(edge_t edge) {
  SkipListHeader *adjacency_list = (SkipListHeader *) raw_neighbourhood(edge.src);

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
    insert_skip_list(edge);
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

    adjacency_index[2 * edge.src + 1] = (void *) ((size_t) adjacency_index[2 * edge.src + 1] + 1);
  }
}

size_t BlockedSkipListAdjacencyLists::get_block_size() {
  return block_size;
}


