//
// Created by per on 01.02.21.
//

#include <cassert>

#include "VersionedEdgeIterator.h"

VersionedEdgeIterator::VersionedEdgeIterator(VersioningBlockedSkipListAdjacencyList &ds) : ds(ds) {

}

bool VersionedEdgeIterator::has_next() {
  assert(opened && "Iterator has not been opened");
  if (data == nullptr) {
    close();
    return false;
  }
  bool ret = move_to_next_edge_in_current_block();
  while (!ret && next_skip_list_block != nullptr) {
    data = next_skip_list_block->data;
    current_block_end = data + next_skip_list_block->size;
    next_skip_list_block = next_skip_list_block->next_levels[0];
    ret = move_to_next_edge_in_current_block();
  }
  if (!ret) {
    ds.release_vertex_lock(src);
  }
  return ret;
}

bool VersionedEdgeIterator::move_to_next_edge_in_current_block() {
  while (data < current_block_end) {
    if (is_versioned(*data)) {
      bool exists = ds.traverse_version_chain({src, make_unversioned(*data)}, version, *(data + 1));
      if (exists) {
        current_edge = make_unversioned(*data);
        data += 2;
        return true;
      } else {
        data += 2;
      }
    } else {
      current_edge = *data;
      data += 1;
      return true;
    }
  }
  return false;
}

dst_t VersionedEdgeIterator::next() {
  assert(opened && "Iterator has not been opened");
  return current_edge;
}

void
VersionedEdgeIterator::initialize(vertex_id_t src, VAdjacencySetType type, void *adjacency_set, uint64_t set_size, version_t version) {
  if (opened) {
    close();
  }

  this->src = src;
  open();

  this->version = version;
  if (adjacency_set == nullptr) {
    data = nullptr;
    current_block_end = nullptr;
    next_skip_list_block = nullptr;
  } else {
    switch (type) {
      case VSINGLE_BLOCK: {
        next_skip_list_block = nullptr;
        data = (dst_t*) adjacency_set;
        current_block_end = data + set_size;
        break;
      }
      case VSKIP_LIST: {
        auto block = (VSkipListHeader*) adjacency_set;
        next_skip_list_block = block->next_levels[0];
        data = block->data;
        current_block_end = block->data + block->size;
        break;
      }
    }
  }
}

void VersionedEdgeIterator::open() {
  assert(!is_open());
  ds.aquire_vertex_lock(src);
  opened = true;
}

void VersionedEdgeIterator::close() {
  if (is_open()) {
    ds.release_vertex_lock(src);
    opened = false;
  }
}

bool VersionedEdgeIterator::is_open() {
  return opened;
}

VersionedEdgeIterator::~VersionedEdgeIterator() {
  // TODO problems here sometimes.

//  if (opened) {
//    ds.release_vertex_lock(src);
//    opened = false;
//  }
}
