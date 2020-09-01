//
// Created by per on 31.08.20.
//

#include <fstream>
#include "EdgeList.h"

void EdgeList::read_from_binary_file(const string &path) {
  ifstream f (path, ifstream::in | ifstream::binary);

  size_t edge_count;
  f.read((char*) &edge_count, sizeof(edge_count));

  edges.clear();
  edges.resize(edge_count);

  f.read((char*) edges.data(), edge_count * sizeof(edge_t));

  f.close();
}
