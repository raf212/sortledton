//
// Created by per on 09.09.20.
//

#ifndef LIVE_GRAPH_TWO_BLOCKEDLINKEDLISTADJACENCYLISTS_H
#define LIVE_GRAPH_TWO_BLOCKEDLINKEDLISTADJACENCYLISTS_H


#include <utils/NotImplemented.h>
#include <data-structures/memory_pools/BlockMemoryPool.h>
#include <experiments/Configuration.h>
#include <iostream>
#include "ToplogyInterface.h"

#define GET_DATA(blockHeader_p) (dst_t*) (((char*) blockHeader_p) + sizeof(BlockHeader))

struct BlockHeader {
    size_t size;
    dst_t min;
    dst_t max;
    BlockHeader* next;
};

/**
 * Optimization ideas:
 *   * use a second small block size for low degree vertices
 *   * while inserting move edges to both sides
 *   *
 */
class BlockedLinkedListAdjacencyLists : public TopologyInterface {
public:
    /**
     *
     * @param block_size the size of a block as the amount of dst_t that should be hold in a block.
     * @param unordered should the elements be loaded in order or unordered
     * @param max_edges the size of the underlying pool as the amount of dst_t that should be hold in total during this execution.
     */
    BlockedLinkedListAdjacencyLists(size_t block_size, bool unordered, size_t max_edges, size_t max_vertices) :
    block_size(block_size), unordered(unordered),
    pool(max_edges / block_size + 1,
            block_size * sizeof(dst_t) + sizeof(BlockHeader),
            500, true) {
      if (block_size % 2 != 0) {
        throw ConfigurationError("We rely on the block to be an even number.");
      }
    };

    size_t vertex_count() override { return adjacency_index.size(); };

    vertex_id_t insert_vertex() override { throw NotImplemented(); };

    void delete_vertex() override { throw NotImplemented(); };

    void insert_edge(edge_t edge) override;

    void delete_edge(edge_t edge) override { throw NotImplemented(); };

    void neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) override;

    void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) override {
      throw NotImplemented();
    };

    void bulkload(const SortedCSRDataSource &src) override;

private:
    vector<BlockHeader *> adjacency_index;

    bool unordered;
    size_t block_size;
    const float bulk_load_fill_rate = 0.9;

    BlockMemoryPool pool;

    BlockHeader* write_to_blocks(const dst_t* start, const dst_t* end);

};


#endif //LIVE_GRAPH_TWO_BLOCKEDLINKEDLISTADJACENCYLISTS_H
