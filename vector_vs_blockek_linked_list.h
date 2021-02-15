//
// Created by per on 15.02.21.
//

#ifndef LIVE_GRAPH_TWO_VECTOR_VS_BLOCKEK_LINKED_LIST_H
#define LIVE_GRAPH_TWO_VECTOR_VS_BLOCKEK_LINKED_LIST_H

#include <cstdint>
#include <cstddef>
#include "tbb/scalable_allocator.h"


using namespace std;

typedef uint64_t data_type;
int main(int argc, char** argv);

class BlockedLinkedList {
public:
    struct Block {
        Block* next;
        size_t size;

        data_type* data();
    };

    BlockedLinkedList(size_t size, data_type fill);

    data_type sum();

    Block* start;
    size_t block_size;
};



#endif //LIVE_GRAPH_TWO_VECTOR_VS_BLOCKEK_LINKED_LIST_H
