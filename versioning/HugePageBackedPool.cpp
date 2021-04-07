//
// Created by per on 07.04.21.
//

#include "HugePageBackedPool.h"

#include <sys/mman.h>
#include <cassert>

#define MB_TO_RESERVE 2
#define LENGTH (MB_TO_RESERVE*1024*1024)

HugePageBackedPool::HugePageBackedPool(size_t block_size) : block_size(block_size) {
  scoped_lock<mutex> l(lock);
  add_page();
}

HugePageBackedPool::~HugePageBackedPool() {
#ifdef DEBUG
  assert(used_list.empty());
#endif
  for (auto p : pages) {
    if (munmap(p, LENGTH)) {
      perror("munmap");
      exit(1);
    }
  }
}

void *HugePageBackedPool::get_block() {
  scoped_lock<mutex> l(lock);
  if (free_list.empty()) {
    add_page();
  }
  auto r = free_list.back();
  free_list.pop_back();

#ifdef DEBUG
  used_list.insert(r);
#endif

  return r;
}

void HugePageBackedPool::free_block(void* block) {
  scoped_lock<mutex> l(lock);
  free_list.push_back(block);

#ifdef DEBUG
  used_list.erase(block);
#endif
}

void HugePageBackedPool::add_page() {
  char* r = (char*) mmap(nullptr, LENGTH, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
  if (r == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  pages.push_back(r);

  for (auto i = r; i < r + LENGTH - block_size; i += block_size) {
    free_list.push_back(i);
  }
}
