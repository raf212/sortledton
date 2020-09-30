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

class Driver {
public:
    Driver(Config config) : config(config), reporter() { };

    void run();

private:
    Config config;
    Reporter reporter;
    SortedCSRDataSource read_base_dataset();
    EdgeList read_insert_dataset();
    EdgeList read_delete_dataset();

    void run_data_structure(SortedCSRDataSource& base, EdgeList& inserts, EdgeList& deletes,
                            DataStructures ds,
                            const vector<string>& ds_parameters,
                            vector<vertex_id_t>& neighbour_2_sources);

    void load_base_dataset(shared_ptr<TopologyInterface> ds, SortedCSRDataSource& base);

    void run_insert_experiment(shared_ptr<TopologyInterface> ds, EdgeList& el);
    void check_insert(shared_ptr<TopologyInterface> ds, EdgeList& el);

    void run_delete_experiment(shared_ptr<TopologyInterface> ds, EdgeList& el);

    /**
     *
     * @param ds
     * @param validate_inserts set to true if called after insertion experiments to validate it, influences the gold standard set picked.
     */
    void run_bfs_experiment(shared_ptr<TopologyInterface> ds, bool validate_inserts);

    /**
     * Checks the BFS search result (distances of all vertices to the start vertex) against a gold standard result.
     * @param start_vertex
     * @param distances
     * @param validate_inserts set to true if called after insertion experiments to validate it, influences the gold standard set picked.
     */
    void check_bfs(vertex_id_t start_vertex, vector<ulong> distances, bool validate_inserts);

    void run_triangle_counting_experiment(shared_ptr<TopologyInterface> ds);
    void run_neighbourhood_2_experiment(shared_ptr<TopologyInterface> ds, const vector<vertex_id_t>& sources);
    void check_neighbourhood_2(unordered_map<vertex_id_t, size_t> neighbour_counts);

    void validate_graph_structure(shared_ptr<TopologyInterface> ds, SortedCSRDataSource &base, EdgeList &inserts, EdgeList &deletes);

    unordered_set<dst_t> get_neighbours(shared_ptr<TopologyInterface> ds, vertex_id_t v);

    ContigiousBlockIterator& getIter(TopologyInterface& ds);

    vector<VectorBatchedEdgeIterator> vectorIterators;
    vector<BlockedBatchedEdgeIterator> blockIterators;




};


#endif //LIVE_GRAPH_TWO_DRIVER_H
