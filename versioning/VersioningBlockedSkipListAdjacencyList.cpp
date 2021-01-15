//
// Created by per on 23.12.20.
//

#include "VersioningBlockedSkipListAdjacencyList.h"

//
// Created by per on 28.09.20.
//

#include <cstring>
#include <cassert>
#include <data-structures/adjacency-lists/VectorBatchedEdgeIterator.h>
#include <iomanip>
#include "BlockedSkipListAdjacencyLists.h"
#include "adjacency-lists/BlockedBatchedEdgeIterator.h"
#include "SizeVersionChainEntry.h"
//#include "TransactionManager.h"
#define FIRST_VERSION 0
// TODO include instead

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


void VersioningBlockedSkipListAdjacencyList::bulkload(const SortedCSRDataSource &src) {
  adjacency_index.reserve(src.vertex_count() * 2);
  vector<mutex> m(src.vertex_count());
  vertex_mutices.swap(m);

  for (int i = 0; i < src.vertex_count(); i++) {
    const dst_t *start = src.adjacency_lists.data() + src.adjacency_index[i];
    const dst_t *end = &src.adjacency_lists[0] + src.adjacency_index[i + 1];

    void *head_block = write_to_blocks(start, end);

    adjacency_index.push_back(head_block);
    adjacency_index.push_back((void *) (end - start));
  }
}

void *VersioningBlockedSkipListAdjacencyList::write_to_blocks(const dst_t *start, const dst_t *end) {
  auto size = end - start;
  if (size == 0) {
    return nullptr;
  } else if (size <= block_size) {
    size_t block_size = round_up_power_of_two(size);
    dst_t *block = (dst_t *) malloc((block_size + 1) * sizeof(dst_t));  // TODO is it better to have blocks of sizes with power of twos.

    block[0] = size;
    memcpy((void *) (block + 1), (void *) start, size * sizeof(dst_t));

    return (void*) ((uint64_t) block | EDGE_SET_TYPE_MASK);
  } else {
    VSkipListHeader *first_block = nullptr;
    VSkipListHeader *last_block = nullptr;

    size_t block_fill = block_size * bulk_load_fill_rate;

    while (start < end) {
      VSkipListHeader *block = (VSkipListHeader *) malloc(memory_block_size());

      if (first_block == nullptr) {
        first_block = block;
      } else {
        last_block->next_levels[0] = block;
      }
      last_block = block;

      block->data = get_data_pointer(block);
      block->size = block_fill < end - start ? block_fill : end - start;

      dst_t *block_data = get_data_pointer(block);
      memcpy((void *) block_data, (void *) start, block->size * sizeof(dst_t));

      block->max = block_data[block->size - 1];

      start += block->size;
    }
    last_block->next_levels[0] = nullptr;

    auto i = first_block;
    vector<VSkipListHeader *> level_blocks(levels, first_block);
    while (i != nullptr) {
      auto height = get_height();
      for (int l = 1; l < levels; l++) {
        i->next_levels[l] = nullptr;
        if (i != first_block && l < height) {
          level_blocks[l]->next_levels[l] = i;
          level_blocks[l] = i;
        }
      }
      i = i->next_levels[0];
    }

    auto b = first_block;
    auto a_size = 0;
    while (b != nullptr) {
      for (auto i = 0; i < b->size; i ++) {
        a_size++;
      }
      b = b->next_levels[0];
    }
    assert(a_size == size);


    return first_block;
  }
}

dst_t *VersioningBlockedSkipListAdjacencyList::get_data_pointer(VSkipListHeader *header) const {
  return (dst_t *) ((char*) header + skip_list_header_size());
}

void VersioningBlockedSkipListAdjacencyList::insert_edge_version(edge_t edge, version_t version) {
   void *adjacency_list = raw_neighbourhood_version(edge.src, version);

  // Insert to empty list
  if (adjacency_list == nullptr) {
    return insert_empty(edge, version);
  } else {
    switch (get_set_type(edge.src, version)) {
      case SINGLE_BLOCK: {
        return insert_single_block(edge, version);
      }
      case SKIP_LIST: {
        return insert_skip_list(edge, version);
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
VSkipListHeader *
VersioningBlockedSkipListAdjacencyList::find_block(VSkipListHeader *pHeader, dst_t element,
                                                   vector<VSkipListHeader *> &blocks) {
  for (int l = levels - 1; 0 <= l; l--) {
    while (pHeader->next_levels[l] != nullptr && pHeader->next_levels[l]->max < element) {
      pHeader = pHeader->next_levels[l];
    }
    if (l == 0 && pHeader->next_levels[0] != nullptr && pHeader->max < element) {
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
VSkipListHeader *
VersioningBlockedSkipListAdjacencyList::find_block1(VSkipListHeader *pHeader, dst_t element) {
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

bool VersioningBlockedSkipListAdjacencyList::has_edge_version(edge_t edge, version_t version) {
  dst_t *pos;
  dst_t *end;
  switch (get_set_type(edge.src, version)) {
    case SKIP_LIST: {
      auto block = find_block1((VSkipListHeader *) raw_neighbourhood_version(edge.src, version), edge.dst);
      end = get_data_pointer(block) + block->size;

      pos = find_upper_bound(get_data_pointer(block), end, edge.dst);
      break;
    }
    case SINGLE_BLOCK: {
      auto start = (dst_t *) raw_neighbourhood_version(edge.src, version);
      auto size = start[0];
      start++;
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
  for (; start < end; start++) {
    auto s = *start;
    if (value <= make_unversioned(s)) {
      return start;
    }
    if (is_versioned(s)) {
      start++;  // Skip inline version record.
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


void VersioningBlockedSkipListAdjacencyList::intersect_neighbourhood_version(vertex_id_t a, vertex_id_t b,
                                                                             vector<dst_t> &out, version_t version) {
  throw NotImplemented();
//  auto s_a = neighbourhood_size(a);
//  auto s_b = neighbourhood_size(b);
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

size_t VersioningBlockedSkipListAdjacencyList::neighbourhood_size_version(vertex_id_t src, version_t version) {
  if (!size_is_versioned(src)) {
    return (uint64_t) adjacency_index[2 * src + 1];
  } else {
    auto chain = (SizeVersionChainEntry *) ((uint64_t) adjacency_index[2 * src + 1] & ~SIZE_VERSION_MASK);
    return chain->traverse(version)->current_size;
  }
}

bool VersioningBlockedSkipListAdjacencyList::size_is_versioned(vertex_id_t v) {
  return (uint64_t) adjacency_index[2 * v + 1] & SIZE_VERSION_MASK;
}

VersioningBlockedSkipListAdjacencyList::VersioningBlockedSkipListAdjacencyList(size_t block_size, size_t levels)
  : block_size (block_size), levels(levels) {
  if (round_up_power_of_two(block_size) != block_size) {
    throw ConfigurationError("Block size needs to be a power of two.");
  }
  level_distribution = binomial_distribution<int>(levels - 1, p);
}

size_t VersioningBlockedSkipListAdjacencyList::vertex_count_version(version_t version) {
  // TODO vertex versions not supported yet.
  return adjacency_index.size() / 2;
}

void *VersioningBlockedSkipListAdjacencyList::raw_neighbourhood_version(vertex_id_t src, version_t version) {
  // TODO vertex versions not supported yet.
  return (void*) ((uint64_t) adjacency_index[2 * src] & ~EDGE_SET_TYPE_MASK);
}

size_t VersioningBlockedSkipListAdjacencyList::memory_block_size() {
  return block_size * sizeof(dst_t) + skip_list_header_size();
}

size_t VersioningBlockedSkipListAdjacencyList::get_height() {
  return level_distribution(level_generator) + 1;
}

size_t VersioningBlockedSkipListAdjacencyList::skip_list_header_size() const {
  return levels * sizeof(VSkipListHeader *) + sizeof(VSkipListHeader);
}

VAdjacencySetType VersioningBlockedSkipListAdjacencyList::get_set_type(vertex_id_t v, version_t version) {
  if ((uint64_t) adjacency_index[v * 2] & EDGE_SET_TYPE_MASK) {
    return VSINGLE_BLOCK;
  } else {
    return VSKIP_LIST;
  }
}

void VersioningBlockedSkipListAdjacencyList::insert_empty(edge_t edge, version_t version) {
  auto block = (dst_t *) malloc(5 * sizeof(dst_t));
  block[0] = 2;  // Fill size of the block, these are destinations plus the amount of versions in the block.
  block[1] = make_versioned(edge.dst);
  block[2] = inline_version(false, false, version);

  adjacency_index[2 * edge.src] = (void*) ((uint64_t) block | EDGE_SET_TYPE_MASK);

  update_adjacency_size(edge.src, false, version);
}

version_t VersioningBlockedSkipListAdjacencyList::inline_version(bool deletion, bool more_versions, version_t version) {
  if (more_versions) {
    version |= MORE_VERSION_MASK;
  }
  if (deletion) {
    version |= DELETION_MASK;
  }
  return version;
}

void VersioningBlockedSkipListAdjacencyList::insert_single_block(edge_t edge, version_t version) {
  auto block = (dst_t *) raw_neighbourhood_version(edge.src, version);
  auto size = block[0];
  auto block_capacity = round_up_power_of_two(size);

  if (size < block_capacity - 1) {  // If block is not too full; -1 for enough space to insert new edge and version, insert into block by shifting
    insert_by_shift(block + 1, block + 1 + size, edge.dst, version);
    block[0] = size + 2;
    update_adjacency_size(edge.src, false, version);
  } else {  // else resize block or add skip list
    if (block_capacity == block_size) {    // Block should be split into 2 skip list blocks, we do this in two steps, convert to SkipListHeader and then by recursion split into two.
      VSkipListHeader *new_block = (VSkipListHeader *) malloc(memory_block_size());
      new_block->data = get_data_pointer(new_block);
      new_block->size = size;

      auto data = block + 1;
      memcpy((void *) get_data_pointer(new_block), (void *) data, size * sizeof(dst_t));

      if (is_versioned(data[size - 2])) {
        new_block->max = make_unversioned(data[size - 2]);
      } else {
        new_block->max = data[size - 1];
      }

      for (int l = 0; l < levels; l++) {
        new_block->next_levels[l] = nullptr;
      }

      adjacency_index[edge.src * 2] = new_block;

      free(block);

      return insert_skip_list(edge,
                              version); // recursive call of depth 2, inefficient could be done with one time less copying.
    } else { // Block full: we double size and copy.
      dst_t *new_block = (dst_t *) malloc((block_capacity * 2  + 1) * sizeof(dst_t));

      new_block[0] = size + 2;  // old size plus new edge and version

      auto old_data = block + 1;
      auto new_data = new_block + 1;
      auto pos_to_insert = find_upper_bound(old_data, old_data + size, edge.dst) - old_data;
      memcpy((void *) new_data, (void *) old_data, sizeof(dst_t) * pos_to_insert);

      new_data[pos_to_insert] = make_versioned(edge.dst);
      new_data[pos_to_insert + 1] = inline_version(false, false, version);

      memcpy((void *) (new_data + pos_to_insert + 2), (void *) (old_data + pos_to_insert),
             sizeof(dst_t) * (size - pos_to_insert));

      free(block);
      adjacency_index[edge.src * 2] = (void*) ((uint64_t) new_block | EDGE_SET_TYPE_MASK);
      update_adjacency_size(edge.src, false, version);
    }
  }
}

void VersioningBlockedSkipListAdjacencyList::update_adjacency_size(vertex_id_t v, bool deletion, version_t version) {
  auto s = (uint64_t) adjacency_index[2 * v + 1];

  auto update = 1;
  if (deletion) {
    update = -1;
  }

  if (size_is_versioned(v)) {
    auto chain = (SizeVersionChainEntry *) (s & ~SIZE_VERSION_MASK);
    chain = new SizeVersionChainEntry(version, chain->current_size + update, deletion, chain);
    adjacency_index[2 * v + 1] = (void *) ((uint64_t) chain | SIZE_VERSION_MASK);
  } else {
    auto chain = new SizeVersionChainEntry(version, s + update, deletion,
                                           new SizeVersionChainEntry(FIRST_VERSION, s, false, nullptr));
    adjacency_index[2 * v + 1] = (void *) ((uint64_t) chain | SIZE_VERSION_MASK);
  }
}

void VersioningBlockedSkipListAdjacencyList::insert_skip_list(edge_t edge, version_t version) {
  VSkipListHeader *adjacency_list = (VSkipListHeader *) raw_neighbourhood_version(edge.src, version);

  vector<VSkipListHeader *> blocks_per_level(levels);
  find_block(adjacency_list, edge.dst, blocks_per_level);

  auto i = blocks_per_level[0];

  // Handle a full block
  if (block_size <= i->size + 1 ) {
    auto data = get_data_pointer(i);
    auto split = block_size / 2;

    if (is_versioned(data[split - 1])) { // Keep the versioned edge together with its version.
      split -= 1;
    }

    auto *new_block = (VSkipListHeader *) malloc(memory_block_size());
    new_block->data = get_data_pointer(new_block);


    memcpy((void *) get_data_pointer(new_block), (void *) (data + split), (i->size - split) * sizeof(dst_t));

    new_block->size = i->size - split;
    i->size = split;

    new_block->next_levels[0] = i->next_levels[0];
    i->next_levels[0] = new_block;

    new_block->max = i->max;
    if (is_versioned(data[split - 2])) {
      i->max = make_unversioned(data[split - 2]);
    } else {
      i->max = data[split - 1];
    }

    auto height = get_height();
    for (int l = 1; l < levels; l++) {
      if (l < height) {
        new_block->next_levels[l] = blocks_per_level[l]->next_levels[l];
        blocks_per_level[l]->next_levels[l] = new_block;
        blocks_per_level[l] = new_block;
      } else {
        new_block->next_levels[l] = nullptr;
      }
    }

    // Recursive call of max depth 1.
    insert_skip_list(edge, version);
  } else {
    auto data = get_data_pointer(i);
    insert_by_shift(data, data + i->size, edge.dst, version);

    i->size += 2;
    i->max = std::max(edge.dst, i->max);

    update_adjacency_size(edge.src, false, version);
  }
}

void VersioningBlockedSkipListAdjacencyList::insert_by_shift(dst_t* start, dst_t* end, dst_t dst, version_t version) {
  auto pos_to_insert = find_upper_bound(start, end, dst);

  for (auto i = end - 1; pos_to_insert <= i; i--) {
    *(i + 2) = *i;
  }

  *pos_to_insert = make_versioned(dst);
  *(pos_to_insert + 1) = inline_version(false, false, version);
}

size_t VersioningBlockedSkipListAdjacencyList::get_block_size() {
  return block_size;
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
//        edges_multi_block_header += sizeof(BlockHeader) + levels * sizeof(SkipListHeader *);
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

void VersioningBlockedSkipListAdjacencyList::aquire_vertex_lock(vertex_id_t vertex_lock) {
  vertex_mutices[vertex_lock].lock();
}

void VersioningBlockedSkipListAdjacencyList::release_vertex_lock(vertex_id_t v) {
  vertex_mutices[v].unlock();
}
