//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_ADJACENCYLISTS_H
#define LIVE_GRAPH_TWO_ADJACENCYLISTS_H

#include <vector>
#include <memory>

#include "adjacency-lists/AdjacencyList.h"
#include "ToplogyInterface.h"

using namespace std;

class AdjacencyLists : public TopologyInterface {
public:
    size_t vertex_count() override { return adjacency_index.size(); }

    vertex_id_t insert_vertex() override;
    void delete_vertex() override;

    void insert_edge(edge_t edge) override;
    void delete_edge(edge_t edge) override;

    void neighbourhood(vertex_id_t src, BatchedEdgeIterator& iter) override;
    void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t>& out) override;

    void bulkload(const SortedCSRDataSource& src) override;

protected:
    virtual unique_ptr<AdjacencyList> construct_adjacency_list(vector<dst_t>::const_iterator begin, vector<dst_t>::const_iterator end) = 0;

private:
    vector<unique_ptr<AdjacencyList>> adjacency_index;
};


#endif //LIVE_GRAPH_TWO_ADJACENCYLISTS_H
