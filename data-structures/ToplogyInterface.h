#ifndef LIVE_GRAPH_TWO_TOPLOGYINTERFACE_H
#define LIVE_GRAPH_TWO_TOPLOGYINTERFACE_H

#include <data-src/DataSource.h>
#include <data-src/SortedCSRDataSource.h>
#include "../data_types.h"
#include "adjacency-lists/BatchedEdgeIterator.h"

class TopologyInterface {
public:
    virtual size_t vertex_count() = 0;

    virtual vertex_id_t insert_vertex() = 0;
    virtual void delete_vertex() = 0;

    virtual void insert_edge(edge_t edge) = 0;
    virtual void delete_edge(edge_t edge) = 0;

    virtual void neighbourhood(vertex_id_t src, BatchedEdgeIterator& iter) = 0;
    virtual void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t>& out) = 0;

    virtual void bulkload(const SortedCSRDataSource& src) = 0;
};


#endif //LIVE_GRAPH_TWO_TOPLOGYINTERFACE_H
