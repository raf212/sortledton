//
// Created by per on 14.10.20.
//

#ifndef LIVE_GRAPH_TWO_EDGEITERATOR_H
#define LIVE_GRAPH_TWO_EDGEITERATOR_H


#include <data_types.h>

class EdgeIterator {
public:
    virtual dst_t next() = 0;
    virtual bool has_next() = 0;

};


#endif //LIVE_GRAPH_TWO_EDGEITERATOR_H
