//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_CONTIGUOUSEDGEBATCH_H
#define LIVE_GRAPH_TWO_CONTIGUOUSEDGEBATCH_H


#include <cstddef>
#include "../data_types.h"
#include "EdgeBatch.h"

class ContiguousEdgeBatch : public EdgeBatch {
public:
    dst_t* start;
    size_t size;

    ContiguousEdgeBatch(dst_t* start, size_t size) : start(start), size(size) {};
};


#endif //LIVE_GRAPH_TWO_CONTIGUOUSEDGEBATCH_H
