//
// Created by per on 23.02.21.
//

#ifndef LIVE_GRAPH_TWO_VERTEXINDEX_H
#define LIVE_GRAPH_TWO_VERTEXINDEX_H


#include <cstdint>
#include <data_types.h>
#include <atomic>
#include <optional>

#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_queue.h>
#include <mutex>

#include "SizeVersionChainEntry.h"

using namespace std;

// The mask indicating if a size entry in the index is versioned.
#define SIZE_VERSION_MASK (1L << 63)
// The 2nd bit of the adjacency set pointer in the index is used to indicate the VAdjacencySetType.
// Set means the edge set is of type VSINGLE_BLOCK, unset means it is of type VSKIP_LIST
#define EDGE_SET_TYPE_MASK (1L << 62)
#define LOCK_MASK (1L << 61)
// This mask is set on vertex index entries for unused vertices.
#define VERTEX_NOT_USED_MASK (1L << 60)

#define SKIP_LIST_LEVELS 6

#define INITIAL_VECTOR_SIZE 1000000

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

struct VertexVersionChainEntry;

//struct VertexEntry {
//
//public:
//    uint64_t get_block_capacity();
//
//    VAdjacencySetType get_adjacency_set_type();
//    VSkipListHeader* get_skip_list_header();
//    dst_t* get_block();
//
//    uint64_t get_size(version_t version);
//    VertexVersionChainEntry* get_vertex_versions();
//    SizeVersionChainEntry* get_size_versions();
//private:
//    void* adjacency_set;  // Holds a pointer to the adjacency set or the version chain entry for this vector
//    uint64_t size;        // Holds the size of the adjacency set of this vector or the capacity of the vector represesnting the adjacency set or the version chain entry for the size.
//
//    bool is_size_versioned();
//};
//
//struct VertexVersionChainEntry {
//    VertexVersionChainEntry* next;
//    version_t version;
//    VertexEntry e;
//};

typedef tbb::concurrent_hash_map<logical_vertex_id_t, vertex_id_t> l_t_p_table;

class VertexIndex {
public:
    VertexIndex() =default;
    VertexIndex(const VertexIndex&) =delete;
    VertexIndex& operator=(const VertexIndex&) =delete;

    vertex_id_t insert_vertex(logical_vertex_id_t id, version_t version);
    void remove_vertex(logical_vertex_id_t id, version_t version);

    optional<vertex_id_t> physical_id(logical_vertex_id_t v) const;
    logical_vertex_id_t logical_id(vertex_id_t v) const;

    void* const & operator[](size_t index) const;
    void*& operator[](size_t index);

    size_t get_vertex_count(version_t version);
    size_t get_high_water_mark();

private:
    atomic_uint high_water_mark { 0u };  // The next physical vertex id, not yet in use.
    atomic_uint vertex_count { 0u };

    mutex growing_vector_mutex;

    tbb::concurrent_vector<void*> index { INITIAL_VECTOR_SIZE};

    l_t_p_table logical_to_physical;
    tbb::concurrent_vector<logical_vertex_id_t> physical_to_logical { INITIAL_VECTOR_SIZE };

    tbb::concurrent_queue<vertex_id_t> free_list;

    template <typename T>
    void grow_vector_if_smaller(tbb::concurrent_vector<T>& v, size_t s) {
      if (v.capacity() <= s) {  // Only synchronize with other threads if potentially necessary
        {
          lock_guard<mutex> l(growing_vector_mutex);
          if (v.capacity() <= s) {
            v.grow_to_at_least(v.capacity() * 2);
          }
        }
      }
    }
};


#endif //LIVE_GRAPH_TWO_VERTEXINDEX_H
