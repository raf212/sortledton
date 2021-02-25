//
// Created by per on 09.09.20.
//

#include <memory>
#include "BlockMemoryPool.h"
#include <iostream>
#include <cassert>

using namespace std;

void *BlockMemoryPool::get_block() {
  if (free_list.empty()) {
    add_pool(grow_rate);
  }
  void* ret = free_list.front();
  assert(ret != nullptr);
  free_list.pop_front();
  return ret;
}

void BlockMemoryPool::free_block(void *block) {
  free_list.push_back(block);
}

BlockMemoryPool::~BlockMemoryPool() {
  for (const auto& p : pools) {
    free(p);
  }
}

BlockMemoryPool::BlockMemoryPool(BlockMemoryPool &&other) noexcept:
        pools(std::move(other.pools)), size(other.size),
        grow_rate(other.grow_rate),  block_size(other.block_size), shuffle_free_list(other.shuffle_free_list),  align_memory(other.align_memory), free_list(std::move(other.free_list))
         {
  other.size = 0;
}

BlockMemoryPool &BlockMemoryPool::operator=(BlockMemoryPool &&other) noexcept {
  if (this == &other) {
    return *this;
  }

  for (const auto& p : pools) {
    free(p);
  }
  pools = std::move(other.pools);
  size = other.size;
  block_size = other.block_size;
  free_list = std::move(other.free_list);
  shuffle_free_list = other.shuffle_free_list;
  grow_rate = other.grow_rate;
  align_memory = other.align_memory;

  other.size = 0;

  return *this;
}

BlockMemoryPool::BlockMemoryPool(size_t size, size_t block_size, size_t grow_rate, bool shuffle_free_list, bool align_memory)
 : grow_rate(grow_rate), block_size(block_size), shuffle_free_list(shuffle_free_list), align_memory(align_memory) {
  add_pool(size);
}

void BlockMemoryPool::add_pool(size_t additional_blocks) {
  auto pool =(char*) malloc(additional_blocks * block_size);
  if (pool == nullptr) {
    throw OutOfMemoryError();
  }
  pools.push_back(pool);

  if (align_memory) {
    char *ptr = pool;
    size_t sp = additional_blocks * block_size;
    char *end = ptr + sp;
    while (1) {
      ptr = (char *) std::align(64, block_size, (void *&) ptr, sp);
      sp = additional_blocks * block_size;
      if (ptr + block_size < end) {
        free_list.push_back(ptr);
      } else {
        break;
      }
      ptr = ptr + block_size;
    }
  } else {
    auto ptr = pool;
    for (uint i = 0; i < additional_blocks; i++) {
      free_list.push_back(ptr);
      ptr += block_size;
    }
  }

  if (shuffle_free_list) {
    shuffle(free_list.begin(), free_list.end(), std::mt19937(std::random_device()()));
  }
}
