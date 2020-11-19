//
// Created by per on 28.10.20.
//

#ifndef LIVE_GRAPH_TWO_HASHSETADJACENCYLISTS_H
#define LIVE_GRAPH_TWO_HASHSETADJACENCYLISTS_H

#include "utils/robin_hood.h"

#include <data_types.h>
#include <data-src/SortedCSRDataSource.h>
#include <utils/NotImplemented.h>
#include <data-structures/adjacency-lists/BatchedEdgeIterator.h>
#include <data-structures/adjacency-lists/EdgeIterator.h>
#include <utils/robin_hood.h>
#include "ToplogyInterface.h"

class HashSetAdjacencyLists : public TopologyInterface {
public:

    ~HashSetAdjacencyLists() override;

    size_t vertex_count() override;

    vertex_id_t insert_vertex() override { throw NotImplemented(); };

    void delete_vertex() override { throw NotImplemented(); };

    void insert_edge(edge_t edge) override { throw NotImplemented(); };

    bool insert_safe(edge_t edge) override { throw NotImplemented(); };

    void delete_edge(edge_t edge) override { throw NotImplemented(); };

    size_t neighbourhood_size(vertex_id_t src) override;

    void neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) override { throw NotImplemented(); };

    void neighbourhood(vertex_id_t src, EdgeIterator &iter) override { throw NotImplemented(); };

    void *raw_neighbourhood(vertex_id_t src) override { return adjacency_index[src]; };

    void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector <dst_t> &out) override;

    bool has_edge(edge_t edge) override { throw NotImplemented(); };

    void bulkload(const SortedCSRDataSource &src) override;

private:
    vector<robin_hood::unordered_flat_set<dst_t>*> adjacency_index;
};

#endif //LIVE_GRAPH_TWO_HASHSETADJACENCYLISTS_H
