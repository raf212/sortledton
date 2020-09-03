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
    CSR() = default;

    size_t vertex_count() override { return adjacency_index.size() - 1; }

    vertex_id_t insert_vertex() override { throw NotImplemented(); };

    void delete_vertex() override { throw NotImplemented(); };

    void insert_edge(edge_t edge) override { throw NotImplemented(); };

    void delete_edge(edge_t edge) override { throw NotImplemented(); };

    // TODO make BatchedEdgeIterator a out parameter, to avoid needing to provide it per adjacency list
    VectorBatchedEdgeIterator& neighbourhood(vertex_id_t src) override;

    void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) override;

    void bulkload(const SortedCSRDataSource &src) override;

private:
    vector<size_t> adjacency_index;
    vector<dst_t> adjacency_lists;
    vector<VectorBatchedEdgeIterator> iterators;

};


#endif //LIVE_GRAPH_TWO_CSR_H
