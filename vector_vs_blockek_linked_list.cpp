//
// Created by per on 15.02.21.
//

#include <vector>
#include <malloc.h>
#include <chrono>
#include <iostream>
#include "vector_vs_blockek_linked_list.h"


int main(int argc, char **argv) {
  size_t elements = 1000000;

  vector<data_type> v(elements, 1u);
  BlockedLinkedList l(elements, 1u);

  auto start = chrono::steady_clock::now();
  data_type s = 0;
  for (auto e : v) {
    s += e;
  }
  auto end = chrono::steady_clock::now();
  size_t microseconds_v = chrono::duration_cast<chrono::nanoseconds>(end - start).count();

  cout << "Summing over a vector took " << microseconds_v << endl;
  cout << s << endl;

  start = chrono::steady_clock::now();
  s = l.sum();
  end = chrono::steady_clock::now();
  auto microseconds_l = chrono::duration_cast<chrono::nanoseconds>(end - start).count();

  cout << "Summing over a blocked linked list took " << microseconds_l << endl;
  cout << s << endl;

  cout << "Overhead " << (double) microseconds_l / (double) microseconds_v << endl;

}

BlockedLinkedList::BlockedLinkedList(size_t size, data_type fill) {
  auto random_calls = 100;
  size_t page_sz = 4096;

  int elements_per_block = (page_sz - sizeof(Block)) / sizeof(data_type) * 1.5;
  block_size = elements_per_block;

  auto elements_allocated = 0;
  Block *last_block = nullptr;
  auto blocks = 0;
  int random_malloc = 0;
  while (elements_allocated < size) {
    for (auto k = 0; k < random_calls; k++) {
      aligned_alloc(page_sz, page_sz);
      aligned_alloc(page_sz / 2, page_sz /2 );
      aligned_alloc(page_sz, page_sz * 3);
      random_malloc++;
    }


    auto b = (Block *) aligned_alloc(page_sz, page_sz * 2);
    if (last_block != nullptr) {
      last_block->next = b;
    } else {
      start = b;
    }
    auto data = b->data();
    b->size = elements_per_block < (size - elements_allocated) ? elements_per_block : (size - elements_allocated);
    for (auto j = 0; j < elements_per_block && elements_allocated < size; j++) {
      data[j] = fill;
      elements_allocated++;
    }
    last_block = b;
    blocks++;
  }
  cout << "Random malloc calls " << random_malloc * 2 << endl;
  cout << "Blocks " << blocks << endl;
}

data_type BlockedLinkedList::sum() {
  auto sum = 0;

  auto b = start;
  while (b != nullptr) {
    auto data = b->data();
    for (auto i = 0; i < b->size; i++) {
      sum += data[i];
    }
    b = b->next;
  }
  return sum;
}

data_type *BlockedLinkedList::Block::data() {
  return (data_type*) ((char*) this + sizeof(Block));
}
