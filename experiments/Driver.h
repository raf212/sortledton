//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_DRIVER_H
#define LIVE_GRAPH_TWO_DRIVER_H

#include <memory>

#include <data-structures/ToplogyInterface.h>
#include <data-src/EdgeList.h>
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

    void run_insert_experiment(TopologyInterface& ds, EdgeList& el);
    void check_insert(TopologyInterface& ds, EdgeList& el);

    void run_delete_experiment(TopologyInterface& ds, EdgeList& el);

    void run_bfs_experiment(TopologyInterface& ds, bool run_on_raw_neighbourhood);

    /**
     * Checks the BFS search result (distances of all vertices to the start vertex) against a gold standard result.
     * @param start_vertex
     * @param distances
     * @param validate_inserts set to true if called after insertion experiments to validate it, influences the gold standard set picked.
     */
    void check_bfs(vertex_id_t start_vertex, vector<uint>& distances, bool validate_inserts);

    void run_page_rank_experiment(TopologyInterface& ds, bool run_on_raw_neighbourhood);

    void check_page_rank(vector<float> scores);

    // TODO remove shared pointer from everything to avoid shared counter overhead
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
};


#endif //LIVE_GRAPH_TWO_DRIVER_H
