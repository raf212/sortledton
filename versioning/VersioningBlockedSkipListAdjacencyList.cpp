//
// Created by per on 23.12.20.
//

#include "VersioningBlockedSkipListAdjacencyList.h"

//
// Created by per on 28.09.20.
//

#include <cstring>
#include <cassert>
#include "BlockedSkipListAdjacencyLists.h"
#include "SizeVersionChainEntry.h"
#include "VersionedEdgeIterator.h"
#include "EdgeBlock.h"

#define CACHELINE_SIZE 64
#define PAGE_SIZE 4096

#define COLLECT_VERSIONS_ON_INSERT 1

#define ASSERT_CONSISTENCY  1

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

// TODO not rewritten to handle versions.
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

//thread_local mt19937 VersioningBlockedSkipListAdjacencyList::level_generator = mt19937((uint) time(NULL));
thread_local mt19937 VersioningBlockedSkipListAdjacencyList::level_generator = mt19937((uint) 42);

thread_local int VersioningBlockedSkipListAdjacencyList::gced_edges = 0;
thread_local int VersioningBlockedSkipListAdjacencyList::gc_merges = 0;
thread_local int VersioningBlockedSkipListAdjacencyList::gc_to_single_block = 0;


void VersioningBlockedSkipListAdjacencyList::bulkload(const SortedCSRDataSource &src) {
  throw NotImplemented();
//  assert(adjacency_index.get_high_water_mark() == 0);  // Should only be called on an empty data structure
//
//  adjacency_index.reserve(src.vertex_count() * 2);
//  vector<mutex> m(src.vertex_count());
//  vertex_mutices.swap(m);
//  vector<atomic_flag> m1(src.vertex_count());
//  vertex_cas_locks.swap(m1);
//
//  for (int i = 0; i < src.vertex_count(); i++) {
//    const dst_t *start = src.adjacency_lists.data() + src.adjacency_index[i];
//    const dst_t *end = &src.adjacency_lists[0] + src.adjacency_index[i + 1];
//
//    void *head_block = write_to_blocks(start, end);
//
//#if defined(DEBUG) && ASSERT_CONSISTENCY
//    assert_adjacency_list_consistency(i, FIRST_VERSION);
//#endif
//
//    adjacency_index.push_back(head_block);
//    adjacency_index.push_back((void *) (end - start));
//  }
//  vertex_count.store(src.vertex_count());
//  max_vertex.store(src.vertex_count());
}

void *VersioningBlockedSkipListAdjacencyList::write_to_blocks(const dst_t *start, const dst_t *end) {
  uint size = end - start;
  if (size == 0) {
    return nullptr;
  } else if (size <= block_size) {
    size_t block_size = max(MIN_BLOCK_SIZE, round_up_power_of_two(size));
    dst_t *block = (dst_t *) malloc((block_size) * sizeof(dst_t));
    memcpy((void *) block, (void *) start, size * sizeof(dst_t));
    return (void *) ((uint64_t) block | EDGE_SET_TYPE_MASK);
  } else {
    VSkipListHeader *first_block = nullptr;
    VSkipListHeader *last_block = nullptr;

    size_t block_fill = block_size * bulk_load_fill_rate;

    while (start < end) {
      VSkipListHeader *block = (VSkipListHeader *) malloc(memory_block_size());

      if (first_block == nullptr) {
        first_block = block;
        block->before = nullptr;
      } else {
        last_block->next_levels[0] = block;
        block->before = last_block;
      }
      last_block = block;

      block->data = get_data_pointer(block);
      block->size = block_fill < (uint) (end - start) ? block_fill : end - start;

      dst_t *block_data = get_data_pointer(block);
      memcpy((void *) block_data, (void *) start, block->size * sizeof(dst_t));

      block->max = block_data[block->size - 1];

      start += block->size;
    }
    last_block->next_levels[0] = nullptr;

    auto i = first_block;
    vector<VSkipListHeader *> level_blocks(SKIP_LIST_LEVELS, first_block);
    while (i != nullptr) {
      auto height = get_height();
      for (uint l = 1; l < SKIP_LIST_LEVELS; l++) {
        i->next_levels[l] = nullptr;
        if (i != first_block && l < height) {
          level_blocks[l]->next_levels[l] = i;
          level_blocks[l] = i;
        }
      }
      i = i->next_levels[0];
    }

    auto b = first_block;
    uint a_size = 0;
    while (b != nullptr) {
      for (auto i = 0; i < b->size; i++) {
        a_size++;
      }
      b = b->next_levels[0];
    }
    assert(a_size == size);


    return first_block;
  }
}

dst_t *VersioningBlockedSkipListAdjacencyList::get_data_pointer(VSkipListHeader *header) const {
  return (dst_t *) ((char *) header + skip_list_header_size());
}

bool VersioningBlockedSkipListAdjacencyList::insert_edge_version(edge_t edge, version_t version, char *properties,
                                                                 size_t properties_size) {
  assert((properties != nullptr && properties_size != 0) || (properties == nullptr && properties_size == 0));
  assert(properties_size == property_size && "We allow only properties of the same size for all edges.");

  void *adjacency_list = raw_neighbourhood_version(edge.src, version);
  __builtin_prefetch((void *) ((uint64_t) adjacency_list & ~EDGE_SET_TYPE_MASK));
  __builtin_prefetch((void *) ((uint64_t) ((dst_t *) adjacency_list + 1) & ~SIZE_VERSION_MASK));

  // Insert to empty list
  if (unlikely(adjacency_list == nullptr)) {
    insert_empty(edge, version, properties);
    return true;
  } else {
    switch (get_set_type(edge.src, version)) {
      case SINGLE_BLOCK: {
        insert_single_block(edge, version, properties);
        return true;
      }
      case SKIP_LIST: {
        insert_skip_list(edge, version, properties);
        return true;
      }
      default: {
        throw NotImplemented();
      }
    }
  }
}

bool VersioningBlockedSkipListAdjacencyList::insert_edge_version(edge_t edge, version_t version) {
  return insert_edge_version(edge, version, nullptr, 0);
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
VSkipListHeader *
VersioningBlockedSkipListAdjacencyList::find_block(VSkipListHeader *pHeader, dst_t element,
                                                   VSkipListHeader *blocks[SKIP_LIST_LEVELS]) {
  for (int l = SKIP_LIST_LEVELS - 1; 0 <= l; l--) {
    while (pHeader->next_levels[l] != nullptr && pHeader->next_levels[l]->max < element &&
           pHeader->next_levels[l]->next_levels[0] != nullptr) {
      // The last block is special case it can be the one to insert but is not a lower bound
      pHeader = pHeader->next_levels[l];
    }
    blocks[l] = pHeader;
  }
  return blocks[0]->next_levels[0] != nullptr && blocks[0]->max < element ? blocks[0]->next_levels[0] : blocks[0];
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
VSkipListHeader *
VersioningBlockedSkipListAdjacencyList::find_block1(VSkipListHeader *pHeader, dst_t element) {
  VSkipListHeader *blocks[SKIP_LIST_LEVELS];
  return find_block(pHeader, element, blocks);
//  for (int l = SKIP_LIST_LEVELS - 1; 0 <= l; l--) {
//    while (pHeader->next_levels[l] != nullptr && get_min_from_skip_list_header(pHeader->next_levels[l]) <= element) {
//      pHeader = pHeader->next_levels[l];
//    }
//  }
//  return pHeader;
}

bool VersioningBlockedSkipListAdjacencyList::has_edge_version_p(edge_t edge, version_t version) {
  dst_t *pos;
  dst_t *end;
  switch (get_set_type(edge.src, version)) {
    case SKIP_LIST: {
      VSkipListHeader *head = (VSkipListHeader *) raw_neighbourhood_version(edge.src, version);
      if (head != nullptr) {
        auto block = find_block1(head, edge.dst);
        end = get_data_pointer(block) + block->size;

        pos = find_upper_bound(get_data_pointer(block), end, edge.dst);
      } else {
        return false;
      }
      break;
    }
    case SINGLE_BLOCK: {
      auto start = (dst_t *) raw_neighbourhood_version(edge.src, version);
      auto [size, _, _1] = adjacency_index.get_block_size(edge.src);
      end = start + size;
      pos = find_upper_bound(start, end, edge.dst);
      break;
    }
  }
  if (pos == end) {
    return false;
  } else if (!is_versioned(*pos)) {
    return *pos == edge.dst;
  } else {
    return make_unversioned(*pos) == edge.dst && traverse_version_chain(edge, version, *(pos + 1));
  }
}

/**
 * Finds the upper bound for value in a sorted array.
 *
 * Ignores versions.
 *
 * @param start
 * @param end
 * @param value
 * @return a pointer to the position of the upper bound or end.
 */
dst_t *VersioningBlockedSkipListAdjacencyList::find_upper_bound(dst_t *start, dst_t *end, dst_t value) {
  auto l = 0;
  auto r = end - start;
  while ((r - l) > 16) {  // Incorrect if not ended before r-l > 4 because there could be an endless loop.
    auto m = l + (r - l) / 2;

    auto v = 0 < m && is_versioned(start[m - 1]) ? make_unversioned(start[m - 1]) : make_unversioned(start[m]);
    if (value > v) {
      l = m + 1;
    } else if (v == value) {
      if (0 < m && is_versioned(start[m - 1])) {
        return start + m - 1;
      } else {
        return start + m;
      }
    } else {
      r = m;
    }
  }
  dst_t *ptr = start + l;
  if (ptr != start && is_versioned(*(ptr - 1))) { // Do not start on a version record.
    ptr -= 1;
  }
  for (; ptr < end; ptr++) {
    auto v = *ptr;
    if (value <= make_unversioned(v)) {
      return ptr;
    }
    if (is_versioned(v)) {
      ptr++;  // Skip inline version record.
    }
  }
  return end;
}

/**
 * Returns if an edge exists in version required_version.
 *
 * @param edge
 * @param required_version
 * @param inline_version the inline version record for this edge.
 * @return
 */
bool VersioningBlockedSkipListAdjacencyList::traverse_version_chain(edge_t edge, version_t required_version,
                                                                    version_t inline_version) {
  if (timestamp(inline_version) <= required_version) { // We want the newest version
    if (is_deletion(inline_version)) { // Latest change was a deletion.
      return false;
    } else {
      return true;
    }
  } else if (!more_versions_existing(inline_version)) {  // We want an old version but there is only one version.
    if (is_deletion(inline_version)) { // The latest change is a deletion, hence the edge existed before
      return true;
    } else {
      return false;
    }
  } else {  // We want an old version and there are multiple versions.
    throw MultipleVersionException();  // TODO multiple versions not yet supported
  }
}


void VersioningBlockedSkipListAdjacencyList::intersect_neighbourhood_version_p(vertex_id_t a, vertex_id_t b,
                                                                               vector<dst_t> &out, version_t version) {
  throw NotImplemented();
//  auto s_a = neighbourhood_size_p(a);
//  auto s_b = neighbourhood_size_p(b);
//
//  out.clear();
//
//  if (s_a == 0 || s_b == 0) {
//    return;
//  }
//
//  if (s_b < s_a) {
//    swap(s_a, s_b);
//    swap(a, b);
//  }
//
//  if (get_set_type(a) == SINGLE_BLOCK && get_set_type(b) == SINGLE_BLOCK) {
//    call_single_single++;
//    auto out_iterator = back_inserter(out);
//    auto start_a = (dst_t *) raw_neighbourhood(a);
//    auto end_a = start_a + neighbourhood_size(a);
//    auto start_b = (dst_t *) raw_neighbourhood(b);
//    auto end_b = start_b + neighbourhood_size(b);
//
//    intersect(start_a, end_a, start_b, end_b, out_iterator)
//  } else if (get_set_type(a) == SINGLE_BLOCK) {
//    call_single++;
//    auto out_iterator = back_inserter(out);
//
//    auto start_a = (dst_t *) raw_neighbourhood(a);
//    auto end_a = start_a + neighbourhood_size(a);
//
//    SkipListHeader *ns_b = (SkipListHeader *) raw_neighbourhood(b);
//
//    if (32 * s_a < s_b) {
//      while (start_a < end_a) {
//        auto b_block = find_block1(ns_b, *start_a);
//        if (b_block == nullptr) {
//          return;
//        }
//
//        auto start_b = b_block->data;
//        auto end_b = start_b + b_block->size;
//
//        if (binary_search(start_b, end_b, *start_a)) {
//          *out_iterator = *start_a;
//        }
//        start_a++;
//      }
//    } else {
//      while (start_a < end_a && ns_b != nullptr) {
//        auto start_b = ns_b->data;
//        auto end_b = start_b + ns_b->size;
//
//        intersect(start_a, end_a, start_b, end_b, out_iterator)
//
//        ns_b = (SkipListHeader *) ns_b->next;
//      }
//    }
//  } else {
//    call_skip++;
//    auto out_iterator = back_inserter(out);
//
//    SkipListHeader *ns_a = (SkipListHeader *) raw_neighbourhood(a);
//    SkipListHeader *ns_b = (SkipListHeader *) raw_neighbourhood(b);
//
//    if (32 * s_a < s_b) {
//      while (ns_a != nullptr) {
//        auto start_a = ns_a->data;
//        auto end_a = ns_a->data + ns_a->size;
//
//        while (start_a < end_a) {
//          auto b_block = find_block1(ns_b, *start_a);
//          if (b_block == nullptr) {
//            return;
//          }
//          auto start_b = b_block->data;
//          auto end_b = start_b + b_block->size;
//
//          if (binary_search(start_b, end_b, *start_a)) {
//            *out_iterator = *start_a;
//          }
//          start_a++;
//        }
//
//        ns_a = (SkipListHeader *) ns_a->next;
//      }
//    } else {
//      auto start_a = ns_a->data;
//      auto end_a = ns_a->data + ns_a->size;
//      auto start_b = ns_b->data;
//      auto end_b = ns_b->data + ns_b->size;
//      while (ns_a != nullptr && ns_b != nullptr) {
//        intersect(start_a, end_a, start_b, end_b, out_iterator)
//
//        if (start_a == end_a) {
//          ns_a = (SkipListHeader *) ns_a->next;
//          if (ns_a != nullptr) {
//            start_a = ns_a->data;
//            end_a = ns_a->data + ns_a->size;
//          }
//        } else {
//          ns_b = (SkipListHeader *) ns_b->next;
//          if (ns_b != nullptr) {
//            start_b = ns_b->data;
//            end_b = ns_b->data + ns_b->size;
//          }
//        }
//      }
//    }
//  }

}


// TODO cleanup index usage
size_t VersioningBlockedSkipListAdjacencyList::neighbourhood_size_version_p(vertex_id_t src, version_t version) {
  switch (get_set_type(src, version)) {
    case VSKIP_LIST: {
      if (!size_is_versioned(src)) {
        return adjacency_index[src].size;
      } else {
        auto chain = (SizeVersionChainEntry *) ((uint64_t) adjacency_index[src].size & ~SIZE_VERSION_MASK);
        return chain->traverse(version, 0)->current_size;
      }
      break;
    }
    case VSINGLE_BLOCK: {
      auto [size, _, versioned] = adjacency_index.get_block_size(src);
      // If the size is not versioned, this means the correct size is stored in the index
      if (!versioned) {
        return size;
      } else { // If the size is versioned the index stores the current count of destinations and versions in the block,
        // we need to iterate over the block to filter out versions.
        auto block = (dst_t *) raw_neighbourhood_version(src, version);
        auto end = block + size;
        auto count = 0;
        for (auto start = block; start < end; start++) {
          if (!is_versioned(*start)) {
            count++;
          } else {
            auto v = (version_t) *(start + 1);
            auto t = timestamp(v);
            if (t <= version) {
              if (!is_deletion(v)) {
                count++;
              }
            } else {
              if (is_deletion(v)) {
                count++;
              }
            }
            start++; // Do not count the version record.
          }
        }
        return count;
      }
      break;
    }
    default: {
      throw NotImplemented();
    }
  }
}

bool VersioningBlockedSkipListAdjacencyList::size_is_versioned(vertex_id_t v) {
  return adjacency_index.size_is_versioned(v);
}

VersioningBlockedSkipListAdjacencyList::VersioningBlockedSkipListAdjacencyList(size_t block_size, size_t property_size,
                                                                               TransactionManager &tm)
        : tm(tm), block_size(block_size), property_size(property_size) {
  if (round_up_power_of_two(block_size) != block_size) {
    throw ConfigurationError("Block size needs to be a power of two.");
  }
}

size_t VersioningBlockedSkipListAdjacencyList::vertex_count_version(version_t version) {
  return adjacency_index.get_vertex_count(version);
}

void *VersioningBlockedSkipListAdjacencyList::raw_neighbourhood_version(vertex_id_t src, version_t version) {
  return adjacency_index.raw_neighbourhood_version(src, version);
}

size_t VersioningBlockedSkipListAdjacencyList::memory_block_size() {
  return block_size * sizeof(dst_t) + skip_list_header_size();
}

size_t VersioningBlockedSkipListAdjacencyList::get_height() {
  return binomial_distribution<int>(SKIP_LIST_LEVELS - 1, p)(level_generator) + 1;
}

// TODO remove function
size_t VersioningBlockedSkipListAdjacencyList::skip_list_header_size() const {
  return sizeof(VSkipListHeader);
}

VAdjacencySetType VersioningBlockedSkipListAdjacencyList::get_set_type(vertex_id_t v, version_t version) {
  return adjacency_index.get_adjacency_set_type(v, version);  // TODO remove
}

void VersioningBlockedSkipListAdjacencyList::insert_empty(edge_t edge, version_t version, char *properties) {
  EdgeBlock eb = new_single_edge_block(MIN_BLOCK_SIZE);
  eb.insert_edge(edge.dst, version, properties);
  adjacency_index.store_single_block(edge.src, eb.get_single_block_pointer(), 2, 1, true);
}

void VersioningBlockedSkipListAdjacencyList::insert_single_block(edge_t edge, version_t version, char *properties) {
  auto [size, property_count, is_versioned] = adjacency_index.get_block_size(edge.src);
  auto eb = EdgeBlock::from_single_block((dst_t *) raw_neighbourhood_version(edge.src, version), size, property_count, property_size);

  auto block_capacity = eb.get_block_capacity();

  bool versions_remaining = is_versioned;
#if COLLECT_VERSIONS_ON_INSERT
  if (is_versioned) {
    versions_remaining = eb.gc(tm.getMinActiveVersion());
  }
#endif

  if (eb.has_space_to_insert_edge()) {
    eb.insert_edge(edge.dst, version, properties);
    adjacency_index.set_block_size(edge.src, eb.get_edges_and_versions(), eb.get_property_count(), true);

#if defined(DEBUG) && ASSERT_CONSISTENCY
    assert_adjacency_list_consistency(edge.src, FIRST_VERSION);
#endif
  } else {  // else resize block or add skip list
    if (block_capacity == block_size) {
      // Block should be split into 2 skip list blocks, we do this in two steps, convert to SkipListHeader and then by recursion split into two.
      VSkipListHeader *new_block = new_skip_list_block();
      auto new_eb = EdgeBlock::from_vskip_list_header(new_block, block_size, property_size);
      eb.copy_into(new_eb);
      new_eb.update_skip_list_header(new_block);

      adjacency_index.set_block_size(edge.src, eb.get_edges_and_versions(), eb.get_property_count(), versions_remaining);
      adjacency_index[edge.src].size = ((uint64_t) construct_version_chain_from_block(edge.src, version) |
                                        SIZE_VERSION_MASK);

      adjacency_index[edge.src].adjacency_set = (uint64_t) new_block;

      free(eb.get_single_block_pointer());

      // recursive call of depth 2, inefficient could be done with one time less copying.
      return insert_skip_list(edge, version, properties);
    } else { // Block full: we double size and copy.
      auto new_eb = new_single_edge_block(block_capacity * 2);
      eb.copy_into(new_eb);
      new_eb.insert_edge(edge.dst, version, properties);

      free(eb.get_single_block_pointer());
      adjacency_index.store_single_block(edge.src, new_eb.get_single_block_pointer(), new_eb.get_edges_and_versions(), new_eb.get_property_count(), true);
#if defined(DEBUG) && ASSERT_CONSISTENCY
      assert_adjacency_list_consistency(edge.src, FIRST_VERSION);
#endif
    }
  }
}

void VersioningBlockedSkipListAdjacencyList::update_adjacency_size(vertex_id_t v, bool deletion, version_t version) {
  assert(get_set_type(v, FIRST_VERSION) == VSKIP_LIST);
  // TODO needs unit testing.
  auto s = adjacency_index[v].size;

  auto update = 1;
  if (deletion) {
    update = -1;
  }

  if (size_is_versioned(v)) {
    auto chain = (SizeVersionChainEntry *) (s & ~SIZE_VERSION_MASK);
    auto min_version_to_keep = tm.getMinActiveVersion();

    auto reuse = gc_adjacency_size(chain, min_version_to_keep);

    // After this loop chain stores the element following our new version.
    // After this loop chain_end stores the element previous to our new version - they can be the same in case of a singleton list.
    auto chain_end = chain;
    while (version < chain->version) {
      chain_end = chain;
      chain = chain->next;
    }

    if (chain->version == version) {
      chain->current_size += update;
    } else {  // We add a new entry between chain_end and chain.
      if (reuse != nullptr) {
        reuse->version = version;
        reuse->current_size = chain->current_size + update;
        reuse->next = chain;
      } else {
        reuse = new SizeVersionChainEntry(version, chain->current_size + update, chain);
      }
      if (chain_end != chain) {
        chain_end->next = reuse;
      } else {
        adjacency_index[v].size = ((uint64_t) reuse | SIZE_VERSION_MASK);
      }
    }
  } else {
    auto chain = new SizeVersionChainEntry(version, s + update,
                                           new SizeVersionChainEntry(FIRST_VERSION, s, nullptr));
    adjacency_index[v].size = ((uint64_t) chain | SIZE_VERSION_MASK);
  }
}

void VersioningBlockedSkipListAdjacencyList::insert_skip_list(edge_t edge, version_t version, char* properties) {
  VSkipListHeader *adjacency_list = (VSkipListHeader *) raw_neighbourhood_version(edge.src, version);

  VSkipListHeader *blocks_per_level[SKIP_LIST_LEVELS];
  auto block = find_block(adjacency_list, edge.dst, blocks_per_level);

  auto eb = EdgeBlock::from_vskip_list_header(block, block_size, property_size);

#if COLLECT_VERSIONS_ON_INSERT
  eb.gc(tm.getMinActiveVersion());
#if defined(DEBUG) && ASSERT_CONSISTENCY
  eb.assert_block_consistency(FIRST_VERSION);
#endif
#endif

  // Handle a full block
  if (!eb.has_space_to_insert_edge()) {
    auto *new_block = new_skip_list_block();
    auto new_edge_block = EdgeBlock::from_vskip_list_header(new_block, block_size, property_size);

    eb.split_into(new_edge_block);
    eb.update_skip_list_header(block);
    new_edge_block.update_skip_list_header(new_block);

    // Insert new block into the skip list at level 0.
    new_block->next_levels[0] = block->next_levels[0];
    new_block->before = block;
    if (new_block->next_levels[0] != nullptr) {
      new_block->next_levels[0]->before = new_block;
    }
    block->next_levels[0] = new_block;

    // Update skip list on all levels but 0
    auto height = get_height();
    for (uint l = 1; l < SKIP_LIST_LEVELS; l++) {
      if (l < height) {
        if (blocks_per_level[l]->next_levels[l] != block) {
          new_block->next_levels[l] = blocks_per_level[l]->next_levels[l];
          blocks_per_level[l]->next_levels[l] = new_block;
        } else {
          new_block->next_levels[l] = block->next_levels[l];
          block->next_levels[l] = new_block;
        }
        blocks_per_level[l] = new_block;
      } else {
        new_block->next_levels[l] = nullptr;
      }
    }

#if defined(DEBUG) && ASSERT_CONSISTENCY
    assert_adjacency_list_consistency(edge.src, FIRST_VERSION);
#endif
    // Recursive call of max depth 1.
    insert_skip_list(edge, version, properties);
  } else {
    eb.insert_edge(edge.dst, version, properties);
    eb.update_skip_list_header(block);
    update_adjacency_size(edge.src, false, version);
#if defined(DEBUG) && ASSERT_CONSISTENCY
    assert_adjacency_list_consistency(edge.src, FIRST_VERSION);
#endif
  }
}

void VersioningBlockedSkipListAdjacencyList::report_storage_size() {
  throw NotImplemented();
//  size_t vertices = sizeof(SkipListHeader *) * adjacency_index.size();
//
//  // All numbers in bytes
//  size_t edges_single_block = 0;           // Edges in single block actual storage needs.
//  size_t edges_single_block_strictly = 0;  // Edges in single block minus storage overhead for having blocks with the sizes of power of twos only.
//  size_t edges_multi_block = 0;            // Edges in multi blocks but not the header.
//  size_t edges_multi_block_header = 0;     // Only the header of multi blocks.
//  size_t edges_multi_block_strictly = 0;   // Strictly needed storage for edges in multi blocks, so without the storage overhead of using a fixed size.
//
//  for (auto v = 0; v < vertex_count(); v++) {
//    if (get_set_type(v) == SINGLE_BLOCK) {
//      edges_single_block_strictly += neighbourhood_size(v) * sizeof(dst_t);
//      edges_single_block += round_up_power_of_two(neighbourhood_size(v)) * sizeof(dst_t);
//    } else {
//      SkipListHeader *ns = (SkipListHeader *) raw_neighbourhood(v);
//
//      while (ns != nullptr) {
//        edges_multi_block += block_size * sizeof(dst_t);
//        edges_multi_block_header += sizeof(BlockHeader) + LEVELS * sizeof(SkipListHeader *);
//        edges_multi_block_strictly += ns->size * sizeof(dst_t);
//        ns = (SkipListHeader *) ns->next;
//      }
//    }
//  }
//  // Total size of all edges
//  size_t edges = edges_single_block + edges_multi_block + edges_multi_block_header;
//
//  cout << "All metrics in MB" << endl;
//  cout << setw(30) << "Vertices: " << right << setw(20) << vertices / 1000000 << endl;
//  cout << endl;
//
//  cout << setw(30) << "Single block: " << right << setw(20) << edges_single_block / 1000000 << endl;
//  cout << setw(30) << "Single block overhead: " << right << setw(20)
//       << (edges_single_block - edges_single_block_strictly) / 1000000 << endl;
//  cout << setw(30) << "Multi block header: " << right << setw(20) << edges_multi_block_header / 1000000 << endl;
//  cout << setw(30) << "Edge multi block: " << right << setw(20) << edges_multi_block / 1000000 << endl;
//  cout << setw(30) << "Multi block overhead: " << right << setw(20)
//       << (edges_multi_block - edges_multi_block_strictly) / 1000000 << endl;
//
//  cout << setw(30) << "Edges: " << right << setw(20) << edges / 1000000 << endl;
//  cout << endl;
//  cout << setw(30) << "Total: " << right << setw(20) << (edges + vertices) / 1000000 << endl;
}

void VersioningBlockedSkipListAdjacencyList::aquire_vertex_lock_p(vertex_id_t v) {
  adjacency_index.aquire_vertex_lock_p(v);
}

void VersioningBlockedSkipListAdjacencyList::release_vertex_lock_p(vertex_id_t v) {
  adjacency_index.release_vertex_lock_p(v);
}

SizeVersionChainEntry *
VersioningBlockedSkipListAdjacencyList::gc_adjacency_size(SizeVersionChainEntry *start, version_t collect_after) {
  while (start->next != nullptr && collect_after < start->next->version) {
    start = start->next;
  }
  // Now start points to the last version to keep. This could be the FIRST_VERSION.
  if (start->version == FIRST_VERSION) { // If its the FIRST_VERSION we can free nothing.
    return nullptr;
  } else {  // This becomes the new FIRST_VERSION
    start->version = FIRST_VERSION;
    auto temp = start->next;
    start->next = nullptr;  // Terminate list.

    SizeVersionChainEntry *to_free = nullptr;
    while (temp != nullptr) {
      if (to_free != nullptr) {
        free(to_free);
      }
      to_free = temp;
      temp = temp->next;
    }
    return to_free;
  }
}

SizeVersionChainEntry *
VersioningBlockedSkipListAdjacencyList::construct_version_chain_from_block(vertex_id_t v, version_t version) {
  auto block = (dst_t *) raw_neighbourhood_version(v, version);
  auto [size, _, _1] = adjacency_index.get_block_size(v);
  auto min_version = tm.getMinActiveVersion();

  vector<version_t> versions_to_construct;
  for (auto i = block; i < block + size; i++) {
    if (is_versioned(*i)) {
      auto v = (version_t) *(i + 1);
      auto t = timestamp(v);
      if (min_version < t) {
        versions_to_construct.push_back(t);
      }
    }
  }

  sort(versions_to_construct.begin(), versions_to_construct.end());

  auto chain = new SizeVersionChainEntry(FIRST_VERSION, neighbourhood_size_version_p(v, min_version), nullptr);
  for (uint i = 0; i < versions_to_construct.size(); i++) {
    chain = new SizeVersionChainEntry(versions_to_construct[i],
                                      neighbourhood_size_version_p(v, versions_to_construct[i]),
                                      chain);
  }
  return chain;
}

void *VersioningBlockedSkipListAdjacencyList::raw_neighbourhood_size_entry(vertex_id_t v) {
  return adjacency_index.raw_neighbourhood_size_entry(v);
}

void VersioningBlockedSkipListAdjacencyList::gc_all() {
  // TODO needs max vertex not vertex count
  auto vertices = get_max_vertex();
  for (vertex_id_t v = 0; v < vertices; v++) {
    if (has_vertex_version_p(v, FIRST_VERSION)) {
      gc_vertex(v);
    }
  }
}

void VersioningBlockedSkipListAdjacencyList::gc_vertex(vertex_id_t v) {
  aquire_vertex_lock_p(v);
  switch (get_set_type(v, FIRST_VERSION)) {
    case VSINGLE_BLOCK: {
      gc_block(v);
      break;
    }
    case VSKIP_LIST: {
      gc_skip_list(v);
      break;
    }
  }
#if defined(DEBUG) && ASSERT_CONSISTENCY
  assert_adjacency_list_consistency(v, tm.getMinActiveVersion());
#endif
  if (get_set_type(v, FIRST_VERSION) == VSKIP_LIST && size_is_versioned(v)) {
    auto start = (SizeVersionChainEntry *) ((uint64_t) raw_neighbourhood_size_entry(v) & ~SIZE_VERSION_MASK);
    SizeVersionChainEntry *to_free = gc_adjacency_size(start, tm.getMinActiveVersion());
    if (to_free != nullptr) {
      free(to_free);
    }
    if (start->version == FIRST_VERSION) {
      adjacency_index[v].size = start->current_size;
      free(start);
    }
  }
  release_vertex_lock_p(v);
}

bool VersioningBlockedSkipListAdjacencyList::gc_block(vertex_id_t v) {
  assert(get_set_type(v, FIRST_VERSION) == VSINGLE_BLOCK);
  auto [size, property_count, is_versioned] = adjacency_index.get_block_size(v);
  bool version_remaining = false; // If a version remains after shifting.
  if (is_versioned) {
    EdgeBlock eb = EdgeBlock::from_single_block((dst_t *) raw_neighbourhood_version(v, FIRST_VERSION), size, property_count, property_size);
    auto min_version = tm.getMinActiveVersion();
    version_remaining = eb.gc(min_version);
    adjacency_index.set_block_size(v, eb.get_edges_and_versions(), eb.get_property_count(), version_remaining);
  }
  return !version_remaining;
}

bool VersioningBlockedSkipListAdjacencyList::gc_skip_list(vertex_id_t v) {
  auto min_version = tm.getMinActiveVersion();
  bool version_remaining = false;
  if (size_is_versioned(v)) {
    VSkipListHeader *before = nullptr;
    auto i = (VSkipListHeader *) raw_neighbourhood_version(v, FIRST_VERSION);
    VSkipListHeader *blocks[SKIP_LIST_LEVELS];
    for (int j = 0; j < SKIP_LIST_LEVELS; j++) {
      blocks[j] = i;
    }
    while (i != nullptr) {
      for (auto l = 0; l < SKIP_LIST_LEVELS; l++) {
        if (i->next_levels[l] != nullptr) {
          blocks[l] = i;
        }
      }
      auto after = i->next_levels[0];
      version_remaining |= gc_skip_list_block(&i, before, after, min_version, blocks, 0);
      if (i != nullptr) {
        before = i;
        i = i->next_levels[0];
      } else {
        i = after;
      }
    }
    auto skip_list = (VSkipListHeader *) raw_neighbourhood_version(v, FIRST_VERSION);
    if (skip_list->next_levels[0] == nullptr) {  //  Single block skip list
      skip_list_to_single_block(v, version_remaining);
    }
  }
  return !version_remaining;
}

bool VersioningBlockedSkipListAdjacencyList::gc_skip_list_block(VSkipListHeader **to_clean, VSkipListHeader *before,
                                                                VSkipListHeader *after, version_t min_version,
                                                                VSkipListHeader *blocks[SKIP_LIST_LEVELS],
                                                                int leave_space) {
  auto eb = EdgeBlock::from_vskip_list_header(*to_clean, block_size, property_size);
  bool versions_remaining = eb.gc(min_version);
  eb.update_skip_list_header(*to_clean);

  // TODO reimplement list merging

//  if (before != nullptr && new_size + leave_space < block_size / 2) {
//    if (new_size + before->size + leave_space <= block_size) {
//      merge_skip_list_blocks(*to_clean, before, blocks);
//      *to_clean = nullptr;
//    } else {
//      /**
//       * Moves elements from the block before the cleaned block into this block to keep it more than half full.
//       *
//       * It moves as many element as needed to make the cleaned block half full again.
//       * It moves data from the end of the block before to the beginning of this block.
//       */
//      auto move_elements = block_size / 2 - new_size;
//      auto before_data = get_data_pointer(before);
//
//      if (is_versioned(before_data[before->size - move_elements - 1])) {
//        move_elements += 1;
//      }
//
//      for (int i = new_size - 1; 0 <= i; i--) {
//        data[i + move_elements] = data[i];
//      }
//      memcpy(data, before_data + before->size - move_elements, sizeof(dst_t) * move_elements);
//
//
//      // TODO need to move properties
//
//      before->size = before->size - move_elements;
//      (*to_clean)->size = (uint16_t) (new_size + move_elements);
//
//      if (is_versioned(before_data[before->size - 2])) {
//        before->max = make_unversioned(before_data[before->size - 2]);
//      } else {
//        before->max = before_data[before->size - 1];
//      }
//#if defined(DEBUG) && ASSERT_CONSISTENCY
//      assert_block_consistency(get_data_pointer(*to_clean), get_data_pointer(*to_clean) + (*to_clean)->size,
//                               FIRST_VERSION);
//      assert_block_consistency(get_data_pointer(before), get_data_pointer(before) + before->size, FIRST_VERSION);
//#endif
//    }
//  }
  return versions_remaining;
}

void VersioningBlockedSkipListAdjacencyList::merge_skip_list_blocks(VSkipListHeader *from, VSkipListHeader *to,
                                                                    VSkipListHeader **blocks) {
  assert(from->size + to->size <= block_size);
  assert(to->max < get_min_from_skip_list_header(from));
  // TODO need to have property support here
  memcpy(get_data_pointer(to) + to->size, get_data_pointer(from), sizeof(dst_t) * from->size);
  to->size += from->size;
  to->max = from->max;
  for (auto l = 0; l < SKIP_LIST_LEVELS; l++) {
    if (to->next_levels[l] == from) {
      to->next_levels[l] = from->next_levels[l];
    }
    if (blocks[l]->next_levels[l] == from) {
      blocks[l]->next_levels[l] = from->next_levels[l];
    }
  }
  if (from->next_levels[0] != nullptr) {
    from->next_levels[0]->before = to;
  }
  free(from);
//  cout << "merge" << endl;
//  cout << gc_merges << endl;
  gc_merges += 1;
}

void VersioningBlockedSkipListAdjacencyList::skip_list_to_single_block(vertex_id_t v, bool contains_versions) {
  assert(get_set_type(v, FIRST_VERSION) == VSKIP_LIST);

  auto skip_list_block = (VSkipListHeader *) raw_neighbourhood_version(v, FIRST_VERSION);
  assert(skip_list_block->next_levels[0] == nullptr);

  auto size = (uint64_t) skip_list_block->size;
  if (size < block_size / 2) {
    auto single_block_size = round_up_power_of_two(size);
    auto single_block = (dst_t *) malloc(single_block_size * sizeof(dst_t));

    EdgeBlock e_b(get_data_pointer(skip_list_block), block_size, skip_list_block->size, skip_list_block->properties, property_size);
    EdgeBlock new_e_b(single_block, single_block_size, 0, 0, property_size);

    e_b.copy_into(new_e_b);

    free(skip_list_block);

    adjacency_index[v].adjacency_set = ((uint64_t) single_block | EDGE_SET_TYPE_MASK);
    adjacency_index.set_block_size(v, size, skip_list_block->properties, contains_versions);
    gc_to_single_block += 1;
  }
}

void VersioningBlockedSkipListAdjacencyList::assert_adjacency_list_consistency(vertex_id_t v, version_t version) {
  switch (get_set_type(v, version)) {
    case VSKIP_LIST: {
      auto start = (VSkipListHeader *) raw_neighbourhood_version(v, version);

      auto i = start;
      VSkipListHeader *before = nullptr;
      VSkipListHeader *blocks[SKIP_LIST_LEVELS];
      for (int j = 0; j < SKIP_LIST_LEVELS; j++) {
        blocks[j] = start;
      }

      while (i != nullptr) {
        assert(i->size <= block_size);
        // If not the last block, the last block could contain less than b_size / 2 elements after bulkloading.
        // && if not the first block because the first block might have less than block_size / 2 elemetns because I only move elements forwards in GC
        if (i->next_levels[0] != nullptr && i != start) {
          // TODO fix that the fact that the last block is less than half full after bulkloading.
//          assert( block_size / 2 - 3 <= i->size);  // TODO there's a bug such that some blocks are slightly smaller than block_size / 2
        }

        auto eb = EdgeBlock::from_vskip_list_header(i, block_size, property_size);
        auto versions = eb.assert_block_consistency(version);

        assert(i->max == eb.get_max_edge());
        assert(i->properties == i->size - versions);

        for (auto l = 0; l < SKIP_LIST_LEVELS; l++) {
          if (i->next_levels[l] != nullptr) {
            if (i != start) {
              assert(blocks[l]->next_levels[l] == i);
            }
            blocks[l] = i;
            auto min_after = get_min_from_skip_list_header(i->next_levels[l]);
            assert(i->max < min_after);
          }
        }
        assert(before == i->before);
        before = i;
        i = i->next_levels[0];
      }
      break;
    }
    case VSINGLE_BLOCK: {
      auto [size, pc, is_versioned] = adjacency_index.get_block_size(v);
      auto eb = EdgeBlock::from_single_block((dst_t *) raw_neighbourhood_version(v, version), size, pc, property_size);
      auto versions = eb.assert_block_consistency(version);

      assert((is_versioned && 0 < versions) || (!is_versioned && versions == 0));
      assert(pc == size - versions);
      break;
    }
  }

}

dst_t VersioningBlockedSkipListAdjacencyList::get_min_from_skip_list_header(VSkipListHeader *header) {
  return make_unversioned(get_data_pointer(header)[0]);
}

void VersioningBlockedSkipListAdjacencyList::neighbourhood_version_p(vertex_id_t src, EdgeIterator &iter,
                                                                     version_t version) {
  bool is_versioned = size_is_versioned(src);
  auto set_type = get_set_type(src, version);
  dynamic_cast<VersionedEdgeIterator &>(iter).initialize(src, set_type,
                                                         raw_neighbourhood_version(src, version),
                                                         set_type == VSINGLE_BLOCK ? get<0>(adjacency_index.get_block_size(src)) : 0, version, is_versioned);
}

VersioningBlockedSkipListAdjacencyList::~VersioningBlockedSkipListAdjacencyList() {
  gc_all();  // Make the data structure completely unversioned.

#if defined(DEBUG) && ASSERT_CONSISTENCY
  for (vertex_id_t i = 0; i < get_max_vertex(); i++) {
    switch (get_set_type(i, FIRST_VERSION)) {
      case VSINGLE_BLOCK: {
        auto[size, pc, _] = adjacency_index.get_block_size(i);
        assert(size == pc);  // Every version has been cleaned up. There should be as many edges as properties.
        break;
      }
      case VSKIP_LIST: {
        auto sl = (VSkipListHeader*) raw_neighbourhood_version(i, FIRST_VERSION);
        while (sl != nullptr) {
          assert(sl->size == sl->properties);
          sl = sl->next_levels[0];
        }
        break;
        }
    }
  }
#endif

  for (auto v = 0u; v < get_max_vertex(); v++) {
    free_adjacency_set(v);
  }
}

void VersioningBlockedSkipListAdjacencyList::free_adjacency_set(vertex_id_t v) {
  assert(!size_is_versioned(v));

  switch (get_set_type(v, FIRST_VERSION)) {
    case VSKIP_LIST: {
      auto skip_list_header = (VSkipListHeader *) raw_neighbourhood_version(v, FIRST_VERSION);

      while (skip_list_header != nullptr) {
        auto next = skip_list_header->next_levels[0];
        free(skip_list_header);
        skip_list_header = next;
      }
      adjacency_index[v].adjacency_set = (uint64_t) nullptr;
      adjacency_index[v].size = 0;
      break;
    }
    case VSINGLE_BLOCK: {
      auto block = (dst_t *) raw_neighbourhood_version(v, FIRST_VERSION);
      free(block);
      adjacency_index.set_block_size(v, 0, 0, false);
      adjacency_index[v].adjacency_set = (uint64_t) nullptr;
      break;
    };
  }

}

bool VersioningBlockedSkipListAdjacencyList::insert_vertex_version(vertex_id_t v, version_t version) {
  return adjacency_index.insert_vertex(v, version);
}

bool VersioningBlockedSkipListAdjacencyList::has_vertex_version_p(vertex_id_t v, version_t version) {
  return adjacency_index.has_vertex_version_p(v, version);
}

size_t VersioningBlockedSkipListAdjacencyList::get_max_vertex() {
  return adjacency_index.get_high_water_mark();
}

size_t VersioningBlockedSkipListAdjacencyList::edge_count_version(version_t version) {
  size_t sum = 0;
  for (size_t v = 0, sz = get_max_vertex(); v < sz; v++) {
    aquire_vertex_lock_p(v);
    sum += neighbourhood_size_version_p(v, version);
    release_vertex_lock_p(v);
  }
  return sum;
}

bool VersioningBlockedSkipListAdjacencyList::aquire_vertex_lock(vertex_id_t v) {
  return adjacency_index.aquire_vertex_lock(v);
}

void VersioningBlockedSkipListAdjacencyList::release_vertex_lock(vertex_id_t v) {
  adjacency_index.release_vertex_lock(v);
}

vertex_id_t VersioningBlockedSkipListAdjacencyList::physical_id(vertex_id_t v) {
  return adjacency_index.physical_id(v).value();
}

vertex_id_t VersioningBlockedSkipListAdjacencyList::logical_id(vertex_id_t v) {
  return adjacency_index.logical_id(v);
}

void VersioningBlockedSkipListAdjacencyList::rollback_vertex_insert(vertex_id_t v) {
  adjacency_index.rollback_vertex_insert(v);
}

bool VersioningBlockedSkipListAdjacencyList::has_vertex_version(vertex_id_t v, version_t version) {
  return adjacency_index.has_vertex(v);
}

size_t VersioningBlockedSkipListAdjacencyList::max_physical_vertex() {
  return adjacency_index.get_high_water_mark();
}

EdgeBlock VersioningBlockedSkipListAdjacencyList::new_single_edge_block(size_t capacity) {
  assert(capacity <= block_size);
  assert(MIN_BLOCK_SIZE <= capacity);

  auto block = (dst_t*) aligned_alloc(CACHELINE_SIZE, capacity * sizeof(dst_t));

  return EdgeBlock(block, capacity, 0, 0, property_size);
}

VSkipListHeader *VersioningBlockedSkipListAdjacencyList::new_skip_list_block() {
  auto h = (VSkipListHeader*) aligned_alloc(PAGE_SIZE, memory_block_size());
  h->data = get_data_pointer(h);
  h->before = nullptr;

  for (auto i = 0u; i < SKIP_LIST_LEVELS; i++) {
    h->next_levels[i] = nullptr;
  }

  h->size = 0;
  h->properties = 0;
  h->max = 0;
  return h;
}
