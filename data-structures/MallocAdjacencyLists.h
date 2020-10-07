//
// Created by per on 07.09.20.
//

#ifndef LIVE_GRAPH_TWO_MALLOCADJACENCYLISTS_H
#define LIVE_GRAPH_TWO_MALLOCADJACENCYLISTS_H


#include <utils/NotImplemented.h>
#include "ToplogyInterface.h"
#include "memory_pools/NonContigiousMemoryPool.h"

class MallocAdjacencyLists : public TopologyInterface {
public:
    explicit MallocAdjacencyLists(bool unordered) : unordered(unordered) {};
    ~MallocAdjacencyLists() override;

    size_t vertex_count() override { return adjacency_index.size(); };

    vertex_id_t insert_vertex() override { throw NotImplemented(); };

    void delete_vertex() override { throw NotImplemented(); };

    void insert_edge(edge_t edge) override { throw NotImplemented(); };

    void delete_edge(edge_t edge) override { throw NotImplemented(); };

    void neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) override;

    void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) override;

    bool has_edge(edge_t edge) override { throw NotImplemented(); };

    void bulkload(const SortedCSRDataSource &src) override;

private:
    vector<dst_t *> adjacency_index;
    vector<dst_t *> seperators;  // Unused but malloced memory location to seperate adjacency lists
    bool unordered;

    NonContigiousMemoryPool pool{15};
};


#endif //LIVE_GRAPH_TWO_MALLOCADJACENCYLISTS_H
