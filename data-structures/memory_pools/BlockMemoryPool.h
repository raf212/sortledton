//
// Created by per on 09.09.20.
//

#ifndef LIVE_GRAPH_TWO_BLOCKMEMORYPOOL_H
#define LIVE_GRAPH_TWO_BLOCKMEMORYPOOL_H

#include <cstddef>
#include <cstdlib>
#include <queue>
#include <random>
#include <algorithm>
#include "OutOfMemoryError.h"

using namespace std;

class BlockMemoryPool {
public:
    /**
     *
     * @param size the number of blocks to pool
     * @param block_size size of a block in bytes
     * @param grow_rate the number of blocks to add once the original number of blocks is full.
     */
    BlockMemoryPool(size_t size, size_t block_size, size_t grow_rate,  bool shuffle_free_list, bool align_memory);

    BlockMemoryPool(const BlockMemoryPool&) = delete;
    BlockMemoryPool& operator=(const BlockMemoryPool&) = delete;

    BlockMemoryPool(BlockMemoryPool&& other) noexcept;
    BlockMemoryPool& operator=(BlockMemoryPool&& other) noexcept;

    ~BlockMemoryPool();
    void* get_block();
    void free_block(void* block);


private:
    vector<char*> pools;
    size_t size;
    size_t grow_rate;
    size_t block_size;
    bool shuffle_free_list;
    bool align_memory;

    deque<void*> free_list;

    void add_pool(size_t size);
};


#endif //LIVE_GRAPH_TWO_BLOCKMEMORYPOOL_H
