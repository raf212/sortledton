//
// Created by per on 13.04.21.
//

#ifndef LIVE_GRAPH_TWO_VERSIONEDBLOCKEDEDGEITERATOR_H
#define LIVE_GRAPH_TWO_VERSIONEDBLOCKEDEDGEITERATOR_H

#include <optional>

#include <data_types.h>
#include "AdjacencySetTypes.h"

class VersioningBlockedSkipListAdjacencyList;

class VersionedBlockedEdgeIterator {
public:
    VersionedBlockedEdgeIterator(VersioningBlockedSkipListAdjacencyList* ds, vertex_id_t v, dst_t* block, size_t size, bool versioned);
    VersionedBlockedEdgeIterator(VersioningBlockedSkipListAdjacencyList* ds, vertex_id_t v, VSkipListHeader* block, bool versioned);

    VersionedBlockedEdgeIterator(VersionedBlockedEdgeIterator& other) = delete;
    VersionedBlockedEdgeIterator& operator=(const VersionedBlockedEdgeIterator&) = delete;

    ~VersionedBlockedEdgeIterator();

    bool has_next_block();
    pair<dst_t*, dst_t*> next_block();

    void open();
    void close();
    bool is_open();
private:
    VersioningBlockedSkipListAdjacencyList* ds = nullptr;
    vertex_id_t src = 0;

    VSkipListHeader* n_block = nullptr;
    dst_t* block = nullptr;
    size_t current_size = 0;

    bool first_block = true;
    bool opened = false;
};


#endif //LIVE_GRAPH_TWO_VERSIONEDBLOCKEDEDGEITERATOR_H
