//
// Created by per on 09.09.20.
//

#include "BlockMemoryPool.h"

void *BlockMemoryPool::get_block() {
  if (free_list.empty()) {
    add_pool(grow_rate);
  }
  void *ret = free_list.front();
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
        pools(std::move(other.pools)), size(other.size), block_size(other.block_size),
        free_list(std::move(other.free_list)) {
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

  other.size = 0;

  return *this;
}

BlockMemoryPool::BlockMemoryPool(size_t size, size_t block_size, size_t grow_rate, bool shuffle_free_list)
 : grow_rate(grow_rate), block_size(block_size), shuffle_free_list(shuffle_free_list) {
  add_pool(size * block_size);
}

void BlockMemoryPool::add_pool(size_t additional_blocks) {
  auto pool =(char*) malloc(additional_blocks * block_size);
  if (pool == nullptr) {
    throw OutOfMemoryError();
  }
  pools.push_back(pool);

  for (int i = 0; i < additional_blocks; i++) {
    char* ptr = pool + i * block_size;
    free_list.push_back(ptr);
  }

  if (shuffle_free_list) {
    shuffle(free_list.begin(), free_list.end(), std::mt19937(std::random_device()()));
  }
}
