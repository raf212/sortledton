//
// Created by per on 23.12.20.
//

#ifndef LIVE_GRAPH_TWO_VERSIONINGBLOCKEDSKIPLISTADJACENCYLIST_H
#define LIVE_GRAPH_TWO_VERSIONINGBLOCKEDSKIPLISTADJACENCYLIST_H


#include <mutex>
#include <random>
#include <atomic>
#include <utils/NotImplemented.h>
#include <versioning/TransactionManager.h>
#include <versioning/SizeVersionChainEntry.h>
#include "VersionedTopologyInterface.h"

#define SKIP_LIST_LEVELS 6

// The mask indicating if a size entry in the index is versioned.
#define SIZE_VERSION_MASK (1L << 63)
// The 2nd bit of the adjacency set pointer in the index is used to indicate the VAdjacencySetType.
// Set means the edge set is of type VSINGLE_BLOCK, unset means it is of type VSKIP_LIST
#define EDGE_SET_TYPE_MASK (1L << 62)
#define LOCK_MASK (1L << 61)
// This mask is set on vertex index entries for unused vertices.
#define VERTEX_NOT_USED_MASK (1L << 60)

/**
 * The types of adjacency sets used.
 */
enum VAdjacencySetType {
    VSKIP_LIST,    // A blocked skip list defined in VSkipListHeader
    VSINGLE_BLOCK  // An array of edges prepended by the number of edges and versions in their.
};

struct VSkipListHeader {
    VSkipListHeader* before;  // TODO remove
    dst_t* data;
    uint16_t size;  // Number of destinations stored in this block.
    dst_t max;
    VSkipListHeader* next_levels[SKIP_LIST_LEVELS];  // a fixed number of pointers for all levels.
};

class MultipleVersionException : exception {

};



class VersioningBlockedSkipListAdjacencyList : public VersionedTopologyInterface {

public:
    VersioningBlockedSkipListAdjacencyList(size_t block_size, TransactionManager& tm);
    ~VersioningBlockedSkipListAdjacencyList() override;

    void reserve_vertices(size_t max_vertices);

    size_t vertex_count_version(version_t version) override;

    // TODO vertex versioning not yet supported
    bool has_vertex_version(vertex_id_t v, version_t version) override;

    // TODO versioning not yet supported
    void insert_vertex_version(vertex_id_t v, version_t version) override;
    void delete_vertex_version(vertex_id_t v, version_t version) override { throw NotImplemented(); };

    size_t edge_count_version(version_t version) override;
    void insert_edge_version(edge_t edge, version_t version) override;
    void delete_edge_version(edge_t edge, version_t version) override { throw NotImplemented(); };

    size_t neighbourhood_size_version(vertex_id_t src, version_t version) override;

    void neighbourhood_version(vertex_id_t src, EdgeIterator& iter, version_t version) override;
    void* raw_neighbourhood_version(vertex_id_t src, version_t version) override;
    VAdjacencySetType get_set_type(vertex_id_t v, version_t version);
    void* raw_neighbourhood_size_entry(vertex_id_t v);

    void intersect_neighbourhood_version(vertex_id_t a, vertex_id_t b, vector<dst_t>& out, version_t version) override;

    bool has_edge_version(edge_t edge, version_t version) override;

    void aquire_vertex_lock(vertex_id_t vertex_lock) override;
    void release_vertex_lock(vertex_id_t v) override;

    void report_storage_size() override;

    /**
     * Bulkload data from a CSR. Does not write any versions. Only to be used with an empty data structure.
     */
    void bulkload(const SortedCSRDataSource &src);

    size_t get_block_size();

    void gc_all() override;
    void gc_vertex(vertex_id_t v) override;

    // TODO make protected
    bool traverse_version_chain(edge_t edge, version_t required_version, version_t inline_version);

    thread_local static int gced_edges;
    thread_local static int gc_merges;
    thread_local static int gc_to_single_block;
protected:
    bool gc_block(vertex_id_t v);
    bool gc_skip_list(vertex_id_t v);

    /**
     * Removes all versions older than min_version from to_clean.
     *
     * Moves versions to before if possible, otherwise moves versions to after.
     *
     * This does collapse skip lists into a single single blocked skip list, it does not collapse it to a smaller adjacency
     * list of type VSingleBlock. This is the responsibility of the caller.
     *
     * @param to_clean the block to remove old versions from, this is an out parameter, it is either the same as for input or a nullptr if to_clean has been removed from the list
     * @param before the block before to_clean, can be a nullptr, is stable after this function
     * @param after the block after to_clean, can be a nullptr, is stable after this function
     * @param min_version minimal version to keep
     * @param blocks all blocks from the skip list that point to from that is one per level of from. This function
     * guarantues not too touch any of these elements if they do not point to from.
     * @param leave_space when pulling elements from the block before or merging blocks, keep leave_space free places to allow for inserts or deletions which run afterwards.
     * @return true if there are still versioned edges in edges to to_clean. Although, they might have been moved to before or after.
     */
    bool gc_skip_list_block(VSkipListHeader **to_clean, VSkipListHeader *before,
            VSkipListHeader *after, version_t min_version, VSkipListHeader* blocks[SKIP_LIST_LEVELS],
            int leave_space);

private:
    TransactionManager& tm;
    vector<void *> adjacency_index;
    vector<mutex> vertex_mutices;
    vector<atomic_flag> vertex_cas_locks;


    atomic<uint> calls_to_add_edge { 0 };
    atomic<uint> vertex_count { 0 };

    size_t block_size;
    const float bulk_load_fill_rate = 1.0;

    // Skiplist constant, likelyhood for being x level high is p^x. 0.25 is a typical value from prior work.
    const float p = 0.25;
    static thread_local mt19937 level_generator;

    void* write_to_blocks(const dst_t* start, const dst_t* end);

    size_t memory_block_size();

    size_t get_height();

    VSkipListHeader* find_block(VSkipListHeader *pHeader, dst_t element, VSkipListHeader* blocks[SKIP_LIST_LEVELS]);
    VSkipListHeader* find_block1(VSkipListHeader *pHeader, dst_t element);

    size_t skip_list_header_size() const;
    dst_t* get_data_pointer(VSkipListHeader* header) const;

    dst_t* find_upper_bound(dst_t* start, dst_t* end, dst_t value);

    void insert_empty(edge_t edge, version_t version);
    void insert_single_block(edge_t edge, version_t version);
    void insert_skip_list(edge_t edge, version_t version);
    void insert_by_shift(dst_t* start, dst_t* end, dst_t dst, version_t version);

    bool size_is_versioned(vertex_id_t v);
    version_t inline_version(bool deletion, bool more_versions, version_t version);

    void update_adjacency_size(vertex_id_t v, bool deletion, version_t version);
    SizeVersionChainEntry* construct_version_chain_from_block(vertex_id_t v, version_t version);

    /**
     * Garbage collects unnecessary versions from a adjacency size version chain. These are all version which are
     * smaller than collect_after.
     *
     * @param start the start of the version chain.
     * @param collect_after timestamp of the minimal version to keep
     * @return nullptr or ptr to a garbage collected version which has not been freed.
     */
    SizeVersionChainEntry* gc_adjacency_size(SizeVersionChainEntry* start, version_t collect_after);

    /**
     * Removes all version below min_version from this block.
     *
     * @param start pointer to the start of the block
     * @param end  pointer past the end of the block
     * @param min_version minimal version to keep
     * @param out_size the size of the block after this function
     * @return if any versioned edge remain in the block after this function.
     */
    bool gc_by_shift(dst_t* start, const dst_t* end, version_t min_version, uint64_t& out_size);

    /**
     * Merges to skip list blocks into one. Frees the other.
     *
     * Assumes that to->size + from->size <= block_size.
     * Assumes to --> from relationship on the first skip list level, in other words, expects that the to block
     * is the predecessor of the from block.
     *
     * @param from all elements are moved to "to", "from" is freed.
     * @param to combines the elements of both blocks
     * @param blocks all blocks from the skip list that point to from that is one per level of from. This function
     * guarantues not too touch any of these elements if they do not point to from.
     */
    void merge_skip_list_blocks(VSkipListHeader* from, VSkipListHeader* to, VSkipListHeader* blocks[SKIP_LIST_LEVELS]);

    /**
     * Converts a SkipList adjacency list with only one block back into a single block.
     *
     * Does nothing if the SkipList is still half full.
     *
     * frees SkipList block if it is converted.
     *
     * @param v vertex id for which to convert the adjacency set.
     * @param contains_versions if the block still contains any versions.
     */
    void skip_list_to_single_block(vertex_id_t v, bool contains_versions);

    void assert_adjacency_list_consistency(vertex_id_t v, version_t min_version);
    void assert_block_consistency(dst_t* start, dst_t* end, version_t min_version);

    dst_t get_min_from_skip_list_header(VSkipListHeader* header);

    /**
     * Assumes that the adjacency set is unversioned.
     * @param v
     */
    void free_adjacency_set(vertex_id_t v);

    size_t get_max_vertex();

};


#endif //LIVE_GRAPH_TWO_VERSIONINGBLOCKEDSKIPLISTADJACENCYLIST_H
