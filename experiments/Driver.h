//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_DRIVER_H
#define LIVE_GRAPH_TWO_DRIVER_H


#include <data-structures/ToplogyInterface.h>
#include <data-src/EdgeList.h>
#include "Configuration.h"

using namespace Configuration;

class Driver {
public:
    Driver() {
      config = Config::get_config();
    };

    void run();

private:
    Config config;
    SortedCSRDataSource read_base_dataset();
    EdgeList read_insert_dataset();
    EdgeList read_delete_dataset();

    void run_data_structure(const SortedCSRDataSource& base, const EdgeList& inserts, const EdgeList& deletes,
                            DataStructures ds);

    void load_base_dataset(shared_ptr<TopologyInterface> ds, const SortedCSRDataSource& base);

    void run_insert_experiment(shared_ptr<TopologyInterface> ds, const EdgeList& el);
    void run_delete_experiment(shared_ptr<TopologyInterface> ds, const EdgeList& el);

    void run_bfs_experiment(shared_ptr<TopologyInterface> ds);
    void run_triangle_counting_experiment(shared_ptr<TopologyInterface> ds);
    void run_neighbourhood_2_experiment(shared_ptr<TopologyInterface> ds);


};


#endif //LIVE_GRAPH_TWO_DRIVER_H
