//
// Created by per on 13.04.21.
//

#include "VersionedBlockedEdgeIterator.h"
#include "VersioningBlockedSkipListAdjacencyList.h"

#include <utils/NotImplemented.h>

VersionedBlockedEdgeIterator::VersionedBlockedEdgeIterator(VersioningBlockedSkipListAdjacencyList* ds, vertex_id_t v,dst_t *block, size_t size, bool versioned)
        : ds(ds), src(v), block(block), current_size(size) {
  if (versioned) {
    throw NotImplemented();
  }
  open();
}

VersionedBlockedEdgeIterator::VersionedBlockedEdgeIterator(VersioningBlockedSkipListAdjacencyList* ds, vertex_id_t v, VSkipListHeader *block, bool versioned) : ds(ds), src(v) {
  if (versioned) {
    throw NotImplemented();
  }

  n_block = block->next_levels[0];
  this->block = block->data;
  current_size = block->size;
  open();
}

bool VersionedBlockedEdgeIterator::has_next_block() {
  if (first_block && block != nullptr) {
    first_block = false;
    return true;
  } else if (n_block != nullptr) {
    block = n_block->data;
    current_size = n_block->size;
    n_block = n_block->next_levels[0];
    return true;
  }
  return false;
}

pair<dst_t *, dst_t*> VersionedBlockedEdgeIterator::next_block() {
  return make_pair(block, block + current_size);
}

VersionedBlockedEdgeIterator::~VersionedBlockedEdgeIterator() {
  close();
}

void VersionedBlockedEdgeIterator::open() {
  opened = true;
  ds->aquire_vertex_lock_p(src);
}

void VersionedBlockedEdgeIterator::close() {
  ds->release_vertex_lock_p(src);
  opened = false;
}

bool VersionedBlockedEdgeIterator::is_open() {
  return opened;
}


