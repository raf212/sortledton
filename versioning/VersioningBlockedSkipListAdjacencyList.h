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

#include "VertexIndex.h"
#include "EdgeBlock.h"

class MultipleVersionException : exception {

};

class VersioningBlockedSkipListAdjacencyList : public VersionedTopologyInterface {

public:
    VersioningBlockedSkipListAdjacencyList(size_t block_size, size_t property_size, TransactionManager& tm);
    ~VersioningBlockedSkipListAdjacencyList() override;

    vertex_id_t physical_id(vertex_id_t v) override;
    vertex_id_t logical_id(vertex_id_t v) override;

    size_t vertex_count_version(version_t version) override;
    size_t max_physical_vertex() override;
    size_t edge_count_version(version_t version) override;

    // TODO vertex versioning not yet supported
    bool has_vertex_version(vertex_id_t v, version_t version) override;
    bool has_vertex_version_p(vertex_id_t v, version_t version) override;

    // TODO versioning not yet supported
    bool insert_vertex_version(vertex_id_t v, version_t version) override;
    bool delete_vertex_version(vertex_id_t v, version_t version) override { throw NotImplemented(); };

    size_t neighbourhood_size_version_p(vertex_id_t src, version_t version) override;

    void neighbourhood_version(vertex_id_t src, EdgeIterator& iter, version_t version) override { throw NotImplemented(); };
    void neighbourhood_version_p(vertex_id_t src, EdgeIterator& iter, version_t version) override;
    void* raw_neighbourhood_version(vertex_id_t src, version_t version) override;
    VAdjacencySetType get_set_type(vertex_id_t v, version_t version);
    void* raw_neighbourhood_size_entry(vertex_id_t v);

    void intersect_neighbourhood_version(vertex_id_t a, vertex_id_t b, vector<dst_t>& out, version_t version) override { throw NotImplemented(); };
    void intersect_neighbourhood_version_p(vertex_id_t a, vertex_id_t b, vector<dst_t>& out, version_t version) override;

    bool has_edge_version_p(edge_t edge, version_t version) override;

    bool insert_edge_version(edge_t edge, version_t version) override;
    bool insert_edge_version(edge_t edge, version_t version, char* properties, size_t properties_size) override;
    bool delete_edge_version(edge_t edge, version_t version) override { throw NotImplemented(); };

    bool aquire_vertex_lock(vertex_id_t v) override;
    void release_vertex_lock(vertex_id_t v) override;
    void aquire_vertex_lock_p(vertex_id_t vertex_lock) override;
    void release_vertex_lock_p(vertex_id_t v) override;

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

    void rollback_vertex_insert(vertex_id_t v) override;
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
    VertexIndex adjacency_index;

    size_t block_size;
    size_t property_size;
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

    EdgeBlock new_single_edge_block(size_t min_capicity_in_edges);
    VSkipListHeader* new_skip_list_block();

    void insert_empty(edge_t edge, version_t version, char* properties);
    void insert_single_block(edge_t edge, version_t version, char* properties);
    void insert_skip_list(edge_t edge, version_t version, char* properties);

    bool size_is_versioned(vertex_id_t v);

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
    size_t assert_edge_block_consistency(EdgeBlock eb, vertex_id_t src, version_t version);

    dst_t get_min_from_skip_list_header(VSkipListHeader* header);

    /**
     * Assumes that the adjacency set is unversioned.
     * @param v
     */
    void free_adjacency_set(vertex_id_t v);

    size_t get_max_vertex();

};


#endif //LIVE_GRAPH_TWO_VERSIONINGBLOCKEDSKIPLISTADJACENCYLIST_H
