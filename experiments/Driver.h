//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_DRIVER_H
#define LIVE_GRAPH_TWO_DRIVER_H

#include <memory>

#include <data-structures/ToplogyInterface.h>
#include <data-src/EdgeList.h>
#include "Configuration.h"

class Driver {
public:
    Driver(Config config) : config(config) { };

    void run();

private:
    Config config;
    SortedCSRDataSource read_base_dataset();
    EdgeList read_insert_dataset();
    EdgeList read_delete_dataset();

    void run_data_structure(SortedCSRDataSource& base, EdgeList& inserts, EdgeList& deletes,
                            DataStructures ds);

    void load_base_dataset(shared_ptr<TopologyInterface> ds, SortedCSRDataSource& base);

    void run_insert_experiment(shared_ptr<TopologyInterface> ds, EdgeList& el);
    void run_delete_experiment(shared_ptr<TopologyInterface> ds, EdgeList& el);

    void run_bfs_experiment(shared_ptr<TopologyInterface> ds);
    void run_triangle_counting_experiment(shared_ptr<TopologyInterface> ds);
    void run_neighbourhood_2_experiment(shared_ptr<TopologyInterface> ds);


};


#endif //LIVE_GRAPH_TWO_DRIVER_H
