//
// Created by per on 28.09.20.
//

#ifndef LIVE_GRAPH_TWO_BLOCKEDSKIPLISTADJACENCYLISTS_H
#define LIVE_GRAPH_TWO_BLOCKEDSKIPLISTADJACENCYLISTS_H

#include <atomic>
#include <utils/NotImplemented.h>
#include <mutex>
#include "ToplogyInterface.h"
#include "BlockedLinkedListAdjacencyLists.h"

struct SkipListHeader : BlockHeader {
    SkipListHeader* next_levels[];  // a fixed number of pointers for all levels but the first.
};

class BlockedSkipListAdjacencyLists : public TopologyInterface {
public:
    BlockedSkipListAdjacencyLists(size_t block_size, size_t levels, bool unordered,
            size_t max_edges, size_t max_vertices) :
    block_size(block_size), unordered(unordered), levels(levels) {
      level_distribution = binomial_distribution<int>(levels - 1, p);
    }

    size_t vertex_count() override { return adjacency_index.size(); };

    vertex_id_t insert_vertex() override { throw NotImplemented(); };

    void delete_vertex() override { throw NotImplemented(); };

    void insert_edge(edge_t edge) override;
    bool insert_safe(edge_t edge) override;

    void delete_edge(edge_t edge) override { throw NotImplemented(); };

    size_t neighbourhood_size(vertex_id_t src) override;

    void neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) override;
    void neighbourhood(vertex_id_t src, EdgeIterator &iter) override { throw NotImplemented(); };
    void* raw_neighbourhood(vertex_id_t src) override { return adjacency_index[src]; };

    void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) override;

    bool has_edge(edge_t edge) override;

    void bulkload(const SortedCSRDataSource &src) override;

private:
    vector<SkipListHeader *> adjacency_index;
    vector<size_t> neighbourhood_sizes;
    vector<mutex> vertex_mutices;

    bool unordered;
    size_t block_size;
    const float bulk_load_fill_rate = 0.9;

    size_t levels;
    const float p = 0.25;

    mt19937 level_generator = mt19937(42);
    binomial_distribution<int> level_distribution;


    SkipListHeader* write_to_blocks(const dst_t* start, const dst_t* end);

    size_t memory_block_size() {
      return block_size * sizeof(dst_t) + sizeof(BlockHeader) + levels * sizeof(SkipListHeader*);
    };

    size_t get_height() {
      return level_distribution(level_generator) + 1;
    };

    SkipListHeader* find_block(SkipListHeader *pHeader, dst_t element, vector<SkipListHeader*> &blocks);

    size_t skip_list_header_size() const {
      return levels * sizeof(SkipListHeader*) + sizeof(BlockHeader);
    };
};


#endif //LIVE_GRAPH_TWO_BLOCKEDSKIPLISTADJACENCYLISTS_H
