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

struct BlockHeader {
    size_t size;  // TODO make smaller
    dst_t min;    // TODO remove, this is not needed because the the first entry is the smallest.
    dst_t max;    // Needed because this would need one additional memory load for each block. Keep it.
    dst_t* data;  // TODO remove, only needed for convenience or for variable length headers. I could check if it's cheaper to have variable length headers indeed.
                  // It should be.
    BlockHeader* next;  // TODO remove first level from skip list then.
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
     * @param adjust_size if true, there will be one block pool for each size up to <block_size> and bulkloading will choose the best fit. That's the smallest that fits all vertices or the largest size.
     */
    BlockedLinkedListAdjacencyLists(size_t block_size, bool unordered, size_t max_edges, size_t max_vertices, bool adjust_size, bool size_in_index):
    block_size(block_size), unordered(unordered), size_in_index(size_in_index) {
      if (block_size % 2 != 0) {
        throw ConfigurationError("We rely on the block to be an even number.");
      }

      if (adjust_size) {
        if (bulk_load_fill_rate != 1.0) {
          throw ConfigurationError("Cannot use adjusting blocksizes with other fill rates than 1.0.");
        }

        uint i = 5;
        while ((1<<i) < block_size) {
          uint bs = 1 << i;
          pools.emplace_back(1000, bs * sizeof(dst_t) + sizeof(BlockHeader), 500, true, true);
          pool_sizes.push_back(bs);
          i++;
        }
        pools.emplace_back(1000, block_size * sizeof(dst_t) + sizeof(BlockHeader), 500, true, true);
        pool_sizes.push_back(block_size);
      } else {
        pools.emplace_back(max_edges / block_size + 1, block_size * sizeof(dst_t) + sizeof(BlockHeader), 500, true, true);
        pool_sizes.push_back(block_size);
      }
    };

    size_t vertex_count() override;

    bool insert_vertex(vertex_id_t v) override { throw NotImplemented(); };
    bool delete_vertex(vertex_id_t v) override { throw NotImplemented(); };

    size_t edge_count() override { throw NotImplemented(); };
    bool insert_edge(edge_t edge) override;
    bool insert_safe(edge_t edge) override { throw NotImplemented(); };

    bool delete_edge(edge_t edge) override { throw NotImplemented(); };

    size_t neighbourhood_size_p(vertex_id_t src) override;

    void neighbourhood_p(vertex_id_t src, BatchedEdgeIterator &iter) override;
    void neighbourhood_p(vertex_id_t src, EdgeIterator &iter) override { throw NotImplemented(); };
    void* raw_neighbourhood(vertex_id_t src) override;

    void intersect_neighbourhood_p(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) override {
      throw NotImplemented();
    };

    bool has_edge_p(edge_t e) override;

    bool has_vertex_p(vertex_id_t v) override { throw NotImplemented(); };

    void bulkload(const SortedCSRDataSource &src) override;

    void report_storage_size() override { throw NotImplemented(); };

private:
    vector<BlockHeader *> adjacency_index;

    bool unordered;
    bool size_in_index;
    size_t block_size;
    const float bulk_load_fill_rate = 1.0;

    vector<BlockMemoryPool> pools;
    vector<uint> pool_sizes;

    BlockHeader* write_to_blocks(const dst_t* start, const dst_t* end);
};


#endif //LIVE_GRAPH_TWO_BLOCKEDLINKEDLISTADJACENCYLISTS_H
