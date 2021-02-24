//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_ADJACENCYLISTS_H
#define LIVE_GRAPH_TWO_ADJACENCYLISTS_H

#include <vector>
#include <memory>

#include "adjacency-lists/AdjacencyList.h"
#include "ToplogyInterface.h"
#include "utils/NotImplemented.h"

using namespace std;

class AdjacencyLists : public TopologyInterface {
public:
    size_t vertex_count() override { return adjacency_index.size(); }

    bool insert_vertex(vertex_id_t v) override;
    bool delete_vertex(vertex_id_t v) override;

    size_t edge_count() override { throw NotImplemented(); };
    bool insert_edge(edge_t edge) override;
    bool insert_safe(edge_t edge) override { throw NotImplemented(); };
    bool delete_edge(edge_t edge) override;

    size_t neighbourhood_size_p(vertex_id_t src) override { throw NotImplemented(); };

    void neighbourhood_p(vertex_id_t src, BatchedEdgeIterator& iter) override;
    void neighbourhood_p(vertex_id_t src, EdgeIterator &iter) override { throw NotImplemented(); };
    void* raw_neighbourhood(vertex_id_t src) override { throw NotImplemented(); };

    void intersect_neighbourhood_p(vertex_id_t a, vertex_id_t b, vector<dst_t>& out) override;

    bool has_edge_p(edge_t e) override;

    bool has_vertex_p(vertex_id_t v) override { throw NotImplemented(); };

    void bulkload(const SortedCSRDataSource& src) override;

protected:
    virtual unique_ptr<AdjacencyList> construct_adjacency_list(vector<dst_t>::const_iterator begin, vector<dst_t>::const_iterator end) = 0;

private:
    vector<unique_ptr<AdjacencyList>> adjacency_index;
};


#endif //LIVE_GRAPH_TWO_ADJACENCYLISTS_H
