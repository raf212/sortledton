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

    bool insert_vertex(vertex_id_t v) override { throw NotImplemented(); };
    bool delete_vertex(vertex_id_t v) override { throw NotImplemented(); };

    size_t edge_count() override { throw NotImplemented(); };
    bool insert_edge(edge_t edge) override { throw NotImplemented(); };
    bool insert_safe(edge_t edge) override { throw NotImplemented(); };

    bool delete_edge(edge_t edge) override { throw NotImplemented(); };

    size_t neighbourhood_size_p(vertex_id_t src) override;

    void neighbourhood_p(vertex_id_t src, BatchedEdgeIterator& iter) override;
    void neighbourhood_p(vertex_id_t src, EdgeIterator &iter) override { throw NotImplemented(); };
    void* raw_neighbourhood(vertex_id_t src) override;

    void intersect_neighbourhood_p(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) override;

    bool has_edge_p(edge_t edge) override;

    bool has_vertex_p(vertex_id_t v) override { throw NotImplemented(); };

    void bulkload(const SortedCSRDataSource &src) override;

    void report_storage_size() override;

    vector<size_t> adjacency_index;
    vector<dst_t> adjacency_lists;
};


#endif //LIVE_GRAPH_TWO_CSR_H
