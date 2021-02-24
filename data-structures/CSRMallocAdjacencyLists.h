//
// Created by per on 08.09.20.
//

#ifndef LIVE_GRAPH_TWO_CSRMALLOCADJACENCYLISTS_H
#define LIVE_GRAPH_TWO_CSRMALLOCADJACENCYLISTS_H


#include <utils/NotImplemented.h>
#include <data-structures/memory_pools/NonContigiousMemoryPool.h>
#include "ToplogyInterface.h"

/**
 * A data structure that uses malloc for all adjacency lists larger than malloc_limit and CSR for smaller ones.
 */
class CSRMallocAdjacencyLists : public TopologyInterface {
public:
    explicit CSRMallocAdjacencyLists(size_t malloc_limit, bool unordered) : malloc_limit(malloc_limit), unordered(unordered) {}
    ~CSRMallocAdjacencyLists() override;

    size_t vertex_count() override { return adjacency_index.size() / 2; };

    bool insert_vertex(vertex_id_t v) override { throw NotImplemented(); };

    bool delete_vertex(vertex_id_t v) override { throw NotImplemented(); };

    size_t edge_count() override { throw NotImplemented(); };
    bool insert_edge(edge_t edge) override { throw NotImplemented(); };
    bool insert_safe(edge_t edge) override { throw NotImplemented(); };

    bool delete_edge(edge_t edge) override { throw NotImplemented(); };

    size_t neighbourhood_size_p(vertex_id_t src) override;

    void neighbourhood_p(vertex_id_t src, BatchedEdgeIterator &iter) override;
    void neighbourhood_p(vertex_id_t src, EdgeIterator &iter) override { throw NotImplemented(); };
    void* raw_neighbourhood(vertex_id_t src) override;

    void intersect_neighbourhood_p(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) override;

    bool has_edge_p(edge_t edge) override { throw NotImplemented(); };

    bool has_vertex_p(vertex_id_t v) override { throw NotImplemented(); };

    void bulkload(const SortedCSRDataSource &src) override;

    void report_storage_size() override { throw NotImplemented(); };

    size_t malloc_limit;
    vector<dst_t *> adjacency_index;

    dst_t* csr;

    bool unordered = true;

    NonContigiousMemoryPool pool = NonContigiousMemoryPool(15);
};


#endif //LIVE_GRAPH_TWO_CSRMALLOCADJACENCYLISTS_H
