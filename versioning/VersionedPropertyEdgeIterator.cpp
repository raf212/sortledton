//
// Created by per on 24.03.21.
//

#include "VersionedPropertyEdgeIterator.h"

VersionedPropertyEdgeIterator::VersionedPropertyEdgeIterator(VersioningBlockedSkipListAdjacencyList &ds, size_t property_size) :
                                                             VersionedEdgeIterator(ds), property_size(property_size) {}

void VersionedPropertyEdgeIterator::initialize(vertex_id_t src, VAdjacencySetType type, void *adjacency_set,
                                               char *property_column, size_t block_size, uint64_t set_size, version_t version,
                                               bool is_versioned) {
  VersionedEdgeIterator::initialize(src, type, adjacency_set, set_size, version, is_versioned);

  switch (type) {
    case VSINGLE_BLOCK: {
      this->property_column = property_column;
      current_property = 0;
      current_skip_list_header = nullptr;
      break;
    }
    case VSKIP_LIST: {
      current_skip_list_header = (VSkipListHeader*) adjacency_set;
      current_property = 0;
      this->property_column = current_skip_list_header->property_start(block_size, property_size);
      break;
    }
  }
  this->block_size = block_size;
}

tuple<dst_t, char *> VersionedPropertyEdgeIterator::next_with_properties() {
  if (current_skip_list_header == nullptr || current_property < current_skip_list_header->properties) {
    dst_t d = VersionedEdgeIterator::next();
    char* p = property_column + current_property * property_size;
    current_property += 1;
    return {d, p};
  } else {
    current_skip_list_header = current_skip_list_header->next_levels[0];
    current_property = 0;
    property_column = current_skip_list_header->property_start(block_size, property_size);
    return next_with_properties();
  }
}

