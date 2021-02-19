//
// Created by per on 01.02.21.
//

#ifndef LIVE_GRAPH_TWO_VERSIONEDEDGEITERATOR_H
#define LIVE_GRAPH_TWO_VERSIONEDEDGEITERATOR_H


#include <adjacency-lists/EdgeIterator.h>
#include "VersioningBlockedSkipListAdjacencyList.h"

class VersionedEdgeIterator : public EdgeIterator {
public:
    explicit VersionedEdgeIterator(VersioningBlockedSkipListAdjacencyList& ds);
    ~VersionedEdgeIterator();
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
    void initialize(vertex_id_t src, VAdjacencySetType type, void* adjacency_set, uint64_t set_size, version_t version, bool is_versioned);

    bool has_next() override;
    dst_t next() override;

    void open() override;
    void close() override;
    bool is_open() override;

private:
    VersioningBlockedSkipListAdjacencyList& ds;  // The graph data structure this iterator belongs to.
    bool is_versioned = true;
    version_t  version = NO_TRANSACTION;  // The version to read by this iterator
    dst_t src = 0;  // The source of the adjacency list that is traversed.

    bool opened = false;

    VSkipListHeader* next_skip_list_block = nullptr; // Pointer to the next block up.
    dst_t* data = nullptr;  // Pointer to the next item up.
    dst_t* current_block_end = nullptr; // Pointer behind the end of the current block
    dst_t current_edge = 0; // Current item

    bool has_next_versioned();
    bool has_next_fast();

    bool move_to_next_edge_in_current_block();
};

typedef VersionedEdgeIterator sortledton_iterator;

#endif //LIVE_GRAPH_TWO_VERSIONEDEDGEITERATOR_H
