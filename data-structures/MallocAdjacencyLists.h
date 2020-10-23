//
// Created by per on 07.09.20.
//

#ifndef LIVE_GRAPH_TWO_MALLOCADJACENCYLISTS_H
#define LIVE_GRAPH_TWO_MALLOCADJACENCYLISTS_H


#include <unordered_map>
#include "utils/robin_hood.h"
#include <utils/NotImplemented.h>
#include "ToplogyInterface.h"
#include "memory_pools/NonContigiousMemoryPool.h"

class MallocAdjacencyLists : public TopologyInterface {
public:
    MallocAdjacencyLists(bool unordered, bool hash_index) : unordered(unordered), use_hash_index(hash_index) {};
    ~MallocAdjacencyLists() override;

    size_t vertex_count() override;

    vertex_id_t insert_vertex() override { throw NotImplemented(); };

    void delete_vertex() override { throw NotImplemented(); };

    void insert_edge(edge_t edge) override { throw NotImplemented(); };
    bool insert_safe(edge_t edge) override { throw NotImplemented(); };

    void delete_edge(edge_t edge) override { throw NotImplemented(); };

    size_t neighbourhood_size(vertex_id_t src) override;

    void neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) override;
    void neighbourhood(vertex_id_t src, EdgeIterator &iter) override { throw NotImplemented(); };
    void* raw_neighbourhood(vertex_id_t src) override { return adjacency_index[src]; };

    void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) override;

    bool has_edge(edge_t edge) override { throw NotImplemented(); };

    void bulkload(const SortedCSRDataSource &src) override;

private:
    vector<dst_t *> adjacency_index;
    robin_hood::unordered_map<vertex_id_t, dst_t*> hash_index;
    bool unordered;
    bool use_hash_index;

    NonContigiousMemoryPool pool{15};
};


#endif //LIVE_GRAPH_TWO_MALLOCADJACENCYLISTS_H
