//
// Created by per on 23.12.20.
//

#ifndef LIVE_GRAPH_TWO_VERSIONINGBLOCKEDSKIPLISTADJACENCYLIST_H
#define LIVE_GRAPH_TWO_VERSIONINGBLOCKEDSKIPLISTADJACENCYLIST_H


#include <mutex>
#include <random>
#include <utils/NotImplemented.h>
#include "VersionedTopologyInterface.h"

// TODO use compile time constant everywhere.
#define LEVELS 6

// The mask indicating if a size entry in the index is versioned.
#define SIZE_VERSION_MASK (1L << 63)
// The 2nd bit of the adjacency set pointer in the index is used to indicate the VAdjacencySetType.
// Set means the edge set is of type VSINGLE_BLOCK, unset means it is of type VSKIP_LIST
#define EDGE_SET_TYPE_MASK (1L << 62)

/**
 * The types of adjacency sets used.
 */
enum VAdjacencySetType {
    VSKIP_LIST,    // A blocked skip list defined in VSkipListHeader
    VSINGLE_BLOCK  // An array of edges prepended by the number of edges and versions in their.
};

struct VSkipListHeader {
    dst_t* data;
    uint16_t size;  // Number of destinations stored in this block.
    dst_t max;
    VSkipListHeader* next_levels[];  // a fixed number of pointers for all levels.
};

class MultipleVersionException : exception {

};

class VersioningBlockedSkipListAdjacencyList : public VersionedTopologyInterface {

public:
    VersioningBlockedSkipListAdjacencyList(size_t block_size, size_t levels);

    size_t vertex_count_version(version_t version) override;

    void insert_vertex_version(vertex_id_t v, version_t version) override { throw NotImplemented(); };
    void delete_vertex_version(vertex_id_t v, version_t version) override { throw NotImplemented(); };

    void insert_edge_version(edge_t edge, version_t version) override;
    void delete_edge_version(edge_t edge, version_t version) override { throw NotImplemented(); };

    size_t neighbourhood_size_version(vertex_id_t src, version_t version) override;

    void* raw_neighbourhood_version(vertex_id_t src, version_t version) override;
    VAdjacencySetType get_set_type(vertex_id_t v, version_t version);
    void intersect_neighbourhood_version(vertex_id_t a, vertex_id_t b, vector<dst_t>& out, version_t version) override;

    bool has_edge_version(edge_t edge, version_t version) override;

    void aquire_vertex_lock(vertex_id_t vertex_lock) override;
    void release_vertex_lock(vertex_id_t v) override;

    void report_storage_size() override;

    void bulkload(const SortedCSRDataSource &src);

    size_t get_block_size();

private:
    vector<void *> adjacency_index;
    vector<mutex> vertex_mutices;

    size_t block_size;
    const float bulk_load_fill_rate = 1.0;

    size_t levels;
    const float p = 0.25;

    mt19937 level_generator = mt19937(42);
    binomial_distribution<int> level_distribution;

    void* write_to_blocks(const dst_t* start, const dst_t* end);

    size_t memory_block_size();

    size_t get_height();

    VSkipListHeader* find_block(VSkipListHeader *pHeader, dst_t element, VSkipListHeader* blocks[LEVELS]);
    VSkipListHeader* find_block1(VSkipListHeader *pHeader, dst_t element);

    size_t skip_list_header_size() const;
    dst_t* get_data_pointer(VSkipListHeader* header) const;

    dst_t* find_upper_bound(dst_t* start, dst_t* end, dst_t value);
    bool traverse_version_chain(edge_t edge, version_t required_version, version_t inline_version);


    void insert_empty(edge_t edge, version_t version);
    void insert_single_block(edge_t edge, version_t version);
    void insert_skip_list(edge_t edge, version_t version);
    void insert_by_shift(dst_t* start, dst_t* end, dst_t dst, version_t version);

    bool size_is_versioned(vertex_id_t v);
    version_t inline_version(bool deletion, bool more_versions, version_t version);

    void update_adjacency_size(vertex_id_t v, bool deletion, version_t version);
};


#endif //LIVE_GRAPH_TWO_VERSIONINGBLOCKEDSKIPLISTADJACENCYLIST_H
