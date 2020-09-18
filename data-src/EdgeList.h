//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_EDGELIST_H
#define LIVE_GRAPH_TWO_EDGELIST_H

#include <vector>
#include <string>
#include "../data_types.h"
#include "DataSource.h"

using namespace std;

class EdgeList : DataSource {
public:
    void read_from_binary_file(const string& path);

    vector<edge_t>::iterator begin() { return edges.begin(); };
    vector<edge_t>::iterator end() { return edges.end(); };

    vector<edge_t> edges;
};

#include "DataSource.h"


#endif //LIVE_GRAPH_TWO_EDGELIST_H
