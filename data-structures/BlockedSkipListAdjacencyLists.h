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

enum AdjacencySetType {
    SKIP_LIST,
    SINGLE_BLOCK
};

class BlockedSkipListAdjacencyLists : public TopologyInterface {
public:
    size_t call_single_single = 0;
    size_t call_single = 0;
    size_t call_skip = 0;

    BlockedSkipListAdjacencyLists(size_t block_size, size_t levels, bool unordered,
            size_t max_edges, size_t max_vertices);

    size_t vertex_count() override;

    vertex_id_t insert_vertex() override { throw NotImplemented(); };
    void delete_vertex() override { throw NotImplemented(); };

    void insert_edge(edge_t edge) override;
    bool insert_safe(edge_t edge) override;
    void delete_edge(edge_t edge) override { throw NotImplemented(); };

    size_t neighbourhood_size(vertex_id_t src) override;

    void neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) override;
    void neighbourhood(vertex_id_t src, EdgeIterator &iter) override { throw NotImplemented(); };
    void* raw_neighbourhood(vertex_id_t src) override;

    void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) override;

    bool has_edge(edge_t edge) override;

    void bulkload(const SortedCSRDataSource &src) override;

    size_t get_block_size();

private:
    vector<void *> adjacency_index;
    vector<mutex> vertex_mutices;

    bool unordered;
    size_t block_size;
    const float bulk_load_fill_rate = 1.0;

    size_t levels;
    const float p = 0.25;

    mt19937 level_generator = mt19937(42);
    binomial_distribution<int> level_distribution;

    void* write_to_blocks(const dst_t* start, const dst_t* end);

    size_t memory_block_size();

    size_t get_height();

    SkipListHeader* find_block(SkipListHeader *pHeader, dst_t element, vector<SkipListHeader*> &blocks);
    SkipListHeader* find_block1(SkipListHeader *pHeader, dst_t element);
    SkipListHeader combine_levels(const vector<SkipListHeader*>& forward_pointers);

    size_t skip_list_header_size() const;

    AdjacencySetType get_set_type(vertex_id_t v);

    SkipListHeader skip_list_header_for_single_block(vertex_id_t v);

    void insert_empty(edge_t edge);
    void insert_single_block(edge_t edge);
    void insert_skip_list(edge_t edge);
};


#endif //LIVE_GRAPH_TWO_BLOCKEDSKIPLISTADJACENCYLISTS_H
