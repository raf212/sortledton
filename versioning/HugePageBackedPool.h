//
// Created by per on 07.04.21.
//

#ifndef LIVE_GRAPH_TWO_HUGEPAGEBACKEDPOOL_H
#define LIVE_GRAPH_TWO_HUGEPAGEBACKEDPOOL_H


#include <cstdlib>
#include <vector>
#include <mutex>
#include <unordered_set>

using namespace std;

class HugePageBackedPool {
public:
    explicit HugePageBackedPool(size_t block_size);
    ~HugePageBackedPool();

    HugePageBackedPool(HugePageBackedPool& other) = delete;
    HugePageBackedPool& operator=(const HugePageBackedPool&) = delete;

    void* get_block();
    void free_block(void* block);

private:
    const size_t block_size;

    mutex lock;
    vector<void*> free_list;
    vector<void*> pages;

#ifdef DEBUG
    unordered_set<void*> used_list;
#endif

    void add_page();
};


#endif //LIVE_GRAPH_TWO_HUGEPAGEBACKEDPOOL_H
