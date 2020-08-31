//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_SORTEDCSRDATASOURCE_H
#define LIVE_GRAPH_TWO_SORTEDCSRDATASOURCE_H

#include <cstddef>
#include <vector>
#include "DataSource.h"
#include "../data_types.h"

using namespace std;

class SortedCSRDataSource : DataSource {
public:
    vector<size_t> adjacency_index;
    vector<dst_t> adjacency_lists;
};


#endif //LIVE_GRAPH_TWO_SORTEDCSRDATASOURCE_H
