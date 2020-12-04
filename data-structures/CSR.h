//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_CSR_H
#define LIVE_GRAPH_TWO_CSR_H

#include <utils/NotImplemented.h>
#include <data-structures/adjacency-lists/VectorBatchedEdgeIterator.h>
#include "ToplogyInterface.h"

class CSR : public TopologyInterface {
public:
    size_t total = 0;
    CSR() = default;

    size_t vertex_count() override { return adjacency_index.size() - 1; }

    vertex_id_t insert_vertex() override { throw NotImplemented(); };

    void delete_vertex() override { throw NotImplemented(); };

    void insert_edge(edge_t edge) override { throw NotImplemented(); };
    bool insert_safe(edge_t edge) override { throw NotImplemented(); };

    void delete_edge(edge_t edge) override { throw NotImplemented(); };

    size_t neighbourhood_size(vertex_id_t src) override;

    void neighbourhood(vertex_id_t src, BatchedEdgeIterator& iter) override;
    void neighbourhood(vertex_id_t src, EdgeIterator &iter) override { throw NotImplemented(); };
    void* raw_neighbourhood(vertex_id_t src) override;

    void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) override;

    bool has_edge(edge_t edge) override;

    void bulkload(const SortedCSRDataSource &src) override;

    vector<size_t> adjacency_index;
    vector<dst_t> adjacency_lists;
};


#endif //LIVE_GRAPH_TWO_CSR_H
