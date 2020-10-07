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

    vertex_id_t insert_vertex() override { throw NotImplemented(); };

    void delete_vertex() override { throw NotImplemented(); };

    void insert_edge(edge_t edge) override { throw NotImplemented(); };

    void delete_edge(edge_t edge) override { throw NotImplemented(); };

    void neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) override;

    void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) override;

    bool has_edge(edge_t edge) override { throw NotImplemented(); };

    void bulkload(const SortedCSRDataSource &src) override;

private:
    size_t malloc_limit;
    vector<dst_t *> adjacency_index;

    dst_t* csr;

    bool unordered = true;

    NonContigiousMemoryPool pool = NonContigiousMemoryPool(15);
};


#endif //LIVE_GRAPH_TWO_CSRMALLOCADJACENCYLISTS_H
