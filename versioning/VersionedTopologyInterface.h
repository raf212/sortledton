//
// Created by per on 23.12.20.
//

#ifndef LIVE_GRAPH_TWO_VERSIONEDTOPOLOGYINTERFACE_H
#define LIVE_GRAPH_TWO_VERSIONEDTOPOLOGYINTERFACE_H

#include <vector>
#include <data_types.h>
#include <data-src/SortedCSRDataSource.h>
#include <adjacency-lists/EdgeIterator.h>

using namespace std;

class VersionedTopologyInterface {
public:
    virtual ~VersionedTopologyInterface();

    virtual size_t vertex_count_version(version_t version) = 0;

    // TODO define fault model for already existing vertices and edges

    virtual bool has_vertex_version(vertex_id_t v, version_t version) = 0;
    virtual void insert_vertex_version(vertex_id_t v, version_t version) = 0;
    virtual void delete_vertex_version(vertex_id_t v, version_t version) = 0;

    virtual void insert_edge_version(edge_t edge, version_t version) = 0;
    virtual void delete_edge_version(edge_t edge, version_t version) = 0;

    virtual size_t neighbourhood_size_version(vertex_id_t src, version_t version) = 0;

    virtual void neighbourhood_version(vertex_id_t src, EdgeIterator& iter, version_t version) = 0;
    virtual void* raw_neighbourhood_version(vertex_id_t src, version_t version) = 0;
    virtual void intersect_neighbourhood_version(vertex_id_t a, vertex_id_t b, vector<dst_t>& out, version_t version) = 0;

    virtual bool has_edge_version(edge_t edge, version_t version) = 0;

    virtual void aquire_vertex_lock(vertex_id_t vertex_lock) = 0;
    virtual void release_vertex_lock(vertex_id_t v) = 0;

    virtual void report_storage_size() = 0;

    virtual void bulkload(const SortedCSRDataSource& src) = 0;

    virtual void gc_all() = 0;
    virtual void gc_vertex(vertex_id_t v) = 0;
};


#endif //LIVE_GRAPH_TWO_VERSIONEDTOPOLOGYINTERFACE_H
