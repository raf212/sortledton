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
    // TODO make protected
    /**
     *
     * @param type
     * @param adjacency_set
     * @param set_size only set to a meaningful value if type is VSINGLE_BLOCK.
     * @param version
     */
    void initialize(vertex_id_t src, VAdjacencySetType type, void* adjacency_set, uint64_t set_size, version_t version);

    bool has_next() override;
    dst_t next() override;

private:
    VersioningBlockedSkipListAdjacencyList& ds;
    version_t  version;
    dst_t src;

    VSkipListHeader* next_skip_list_block;
    dst_t* data;
    dst_t* current_block_end;
    dst_t current_edge;

    bool move_to_next_edge_in_current_block();
};


#endif //LIVE_GRAPH_TWO_VERSIONEDEDGEITERATOR_H
