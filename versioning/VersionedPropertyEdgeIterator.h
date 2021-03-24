//
// Created by per on 24.03.21.
//

#ifndef LIVE_GRAPH_TWO_VERSIONEDPROPERTYEDGEITERATOR_H
#define LIVE_GRAPH_TWO_VERSIONEDPROPERTYEDGEITERATOR_H


#include <data_types.h>
#include "VersionedEdgeIterator.h"

class VersioningBlockedSkipListAdjacencyList;

class VersionedPropertyEdgeIterator : public VersionedEdgeIterator {
public:
    VersionedPropertyEdgeIterator(VersioningBlockedSkipListAdjacencyList& ds, size_t property_size);
    ~VersionedPropertyEdgeIterator() = default;

    // TODO make protected
    /**
     * Initializes the iterator to iterate over a adjacency set.
     *
     * Also, closes it if its open.
     * Also, opens the iterator.
     *
     * @param type
     * @param adjacency_set
     * @param set_size only set to a meaningful value if type is VSINGLE_BLOCK.
     * @param version
     */
    void initialize(vertex_id_t src, VAdjacencySetType type, void* adjacency_set, char* property_column, size_t block_size, uint64_t set_size, version_t version, bool is_versioned);

    tuple<dst_t, char*> next_with_properties();


private:
    const size_t property_size;

    VSkipListHeader* current_skip_list_header;

    size_t block_size = 0;

    char* property_column = nullptr;
    int current_property = 0;


};

typedef VersionedEdgeIterator sortledton_property_iterator;


#endif //LIVE_GRAPH_TWO_VERSIONEDPROPERTYEDGEITERATOR_H
