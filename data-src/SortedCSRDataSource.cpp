//
// Created by per on 31.08.20.
//

#include <fstream>
#include "SortedCSRDataSource.h"
#include "IOException.h"

void SortedCSRDataSource::read_from_binary_file(const string &path) {
  char* buffer = nullptr;

  ifstream f {path, ifstream::in | ifstream::binary};

  FileHeader header;
  f.read((char*) &header, sizeof(header));
  if (!f.good()) {
    throw IOException("Could not read header of CSR file.");
  }

  adjacency_index.clear();
  adjacency_index.resize(header.vertex_count + 1);

  adjacency_lists.clear();
  adjacency_lists.resize(header.edge_count);

  FileBody body{ adjacency_index.data(),  adjacency_lists.data()};

  f.read((char*) body.offsets, adjacency_index.size() * sizeof(vertex_id_t));
  f.read((char*) body.adjacency_lists, adjacency_lists.size() * sizeof(dst_t));

  if (!f.eof()) {
    throw IOException("Could not read body of CSR file.");
  }

  f.close();
}
