//
// Created by per on 14.10.20.
//

#ifndef LIVE_GRAPH_TWO_HASHSETSIMULATORADJACENCYLIST_H
#define LIVE_GRAPH_TWO_HASHSETSIMULATORADJACENCYLIST_H

#include <unordered_map>
#include "utils/robin_hood.h"
#include <utils/NotImplemented.h>
#include "ToplogyInterface.h"
#include "memory_pools/NonContigiousMemoryPool.h"
#include "adjacency-lists/EdgeIterator.h"

class HashSetSimulatorAdjacencyList : public TopologyInterface {
public:
    explicit HashSetSimulatorAdjacencyList(float fill_rate);

    size_t vertex_count() override;

    vertex_id_t insert_vertex() override { throw NotImplemented(); };

    void delete_vertex() override { throw NotImplemented(); };

    void insert_edge(edge_t edge) override { throw NotImplemented(); };

    void delete_edge(edge_t edge) override { throw NotImplemented(); };

    void neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) override { throw NotImplemented(); };
    void neighbourhood(vertex_id_t src, EdgeIterator& iter) override;

    void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) override;

    bool has_edge(edge_t edge) override { throw NotImplemented(); };

    void bulkload(const SortedCSRDataSource &src) override;

private:
    vector<dst_t *> adjacency_index;
    NonContigiousMemoryPool pool{15};

    const float fill_rate;
};


#endif //LIVE_GRAPH_TWO_HASHSETSIMULATORADJACENCYLIST_H
