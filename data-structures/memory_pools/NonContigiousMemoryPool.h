//
// Created by per on 06.10.20.
//

#ifndef LIVE_GRAPH_TWO_NONCONTIGIOUSMEMORYPOOL_H
#define LIVE_GRAPH_TWO_NONCONTIGIOUSMEMORYPOOL_H


#include <cstddef>
#include "BlockMemoryPool.h"

class NonContigiousMemoryPool {
public:
    /**
     * @param max_size the maximal block size required in powers of two: passing 2 will allow
     * maximum block sizes of 4.
     */
    explicit NonContigiousMemoryPool(size_t max_size);

    void* get_memory(size_t size);

private:
    size_t min_size = 8;
    vector<BlockMemoryPool> pools;
};


#endif //LIVE_GRAPH_TWO_NONCONTIGIOUSMEMORYPOOL_H
