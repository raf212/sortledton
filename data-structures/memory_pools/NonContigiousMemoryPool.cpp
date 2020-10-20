//
// Created by per on 06.10.20.
//

#include "NonContigiousMemoryPool.h"

#include <iostream>

NonContigiousMemoryPool::NonContigiousMemoryPool(size_t max_size) {
  for (int i = min_size; i <= max_size; i++) {
    pools.emplace_back(500, 1L<<i, 500, true, false);
  }
  if (max_size <= min_size) {
    pools.emplace_back(500, 1L<<min_size, 500, true, false);
  }
}

void *NonContigiousMemoryPool::get_memory(size_t size) {
  size_t pool_index = round(ceil(log2(size))) - min_size;

  if (pools.size() <= pool_index) {
    return malloc(size);
  } else if (pool_index < 0) {
    pool_index = 0;
  }

  return pools[pool_index].get_block();
}


