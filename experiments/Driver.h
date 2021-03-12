//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_DRIVER_H
#define LIVE_GRAPH_TWO_DRIVER_H

#include <memory>

#include <data-structures/ToplogyInterface.h>
#include <data-src/EdgeList.h>
#include <versioning/TransactionManager.h>
#include <versioning/VersionedTopologyInterface.h>
#include <versioning/VersionedEdgeIterator.h>
#include "data-structures/adjacency-lists/BlockedBatchedEdgeIterator.h"
#include "data-structures/adjacency-lists/VectorBatchedEdgeIterator.h"
#include "Reporter.h"
#include "Configuration.h"

#include "data-structures/adjacency-lists/FilteredVectorIterator.h"

class Driver {
public:
    Driver(Config config) : config(config), reporter(config) { };

    void run();

    ContigiousBlockIterator& getIter(TopologyInterface& ds);
    EdgeIterator& getSingleEdgeIter(TopologyInterface& ds);
    unordered_set<dst_t> get_neighbours(TopologyInterface& ds, vertex_id_t v);

private:
    Config config;
    Reporter reporter;
    SortedCSRDataSource read_base_dataset();
    EdgeList read_insert_dataset();
    EdgeList read_delete_dataset();

    void run_data_structure(SortedCSRDataSource& base, EdgeList& inserts, EdgeList& deletes,
                            DataStructures ds,
                            const vector<string>& ds_parameters,
                            vector<vector<vertex_id_t>>& neighbour_2_sources);

    void load_base_dataset(TopologyInterface& ds, SortedCSRDataSource& base);

    void run_insert_experiment(TransactionManager &tm, TopologyInterface &ds, EdgeList &el, size_t base_edge_count);
    void run_insert_experiment_one_by_one(TransactionManager &tm, VersionedTopologyInterface *ds, EdgeList &el,
                                          size_t base_edge_count);
    void check_insert(TopologyInterface& ds, EdgeList& el, size_t base_edge_count);

    void run_delete_experiment(TopologyInterface& ds, EdgeList& el);

    void run_bfs_experiment(TopologyInterface &ds, bool run_on_raw_neighbourhood, bool aquire_locks, bool after_inserts,
                            bool gabbs);

    /**
     * Checks the BFS search result (distances of all vertices to the start vertex) against a gold standard result.
     * @param start_vertex
     * @param distances
     * @param version 0 for base version without inserts, 1 for version after all inserts, all others for that specific version.
     */
    void check_bfs(vertex_id_t start_vertex, vector<pair<vertex_id_t, uint>>& distances, version_t version);

    void run_page_rank_experiment(TopologyInterface& ds, bool run_on_raw_neighbourhood);

    void check_page_rank(vector<float>& scores);

    void run_triangle_counting_experiment(TopologyInterface& ds);
    void check_triangle_counting(size_t count);

    vector<vector<vertex_id_t>> select_2_neighbourhood_src(const SortedCSRDataSource &src, int count);
    void run_neighbourhood_2_experiment(TopologyInterface& ds, const vector<vector<vertex_id_t>>& sources, bool run_on_raw_neighbourhood);
    void check_neighbourhood_2(unordered_map<vertex_id_t, size_t> neighbour_counts);

    void run_community_detection(TopologyInterface& ds);
    void check_community_detection(vector<vertex_id_t> labels);

    void validate_graph_structure(TopologyInterface& ds, SortedCSRDataSource &base, EdgeList &inserts, EdgeList &deletes);

    void print_graph(TopologyInterface& ds);

    // TODO rename to _ naming convention
    vector<VectorBatchedEdgeIterator> vectorIterators;

    vector<BlockedBatchedEdgeIterator> blockIterators;
    vector<FilteredVectorIterator> filteredBlockIterators;
    vector<VersionedEdgeIterator> versionedIterators;

    void show_storage_sizes(string ds_name, TopologyInterface& ds);

    void run_gc_experiment(TransactionManager& tm, VersionedTopologyInterface& ds, bool inserts_run, EdgeList &inserts);
    void check_gc_experiment(VersionedTopologyInterface& ds);

};


#endif //LIVE_GRAPH_TWO_DRIVER_H
