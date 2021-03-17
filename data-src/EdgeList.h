//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_EDGELIST_H
#define LIVE_GRAPH_TWO_EDGELIST_H

#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <omp.h>
#include "../data_types.h"
#include "DataSource.h"

using namespace std;

template <typename et>
class EdgeList : DataSource {
public:
    typename vector<et>::iterator begin() { return edges.begin(); };
    typename vector<et>::iterator end() { return edges.end(); };

    vector<et> edges;

    void read_from_binary_file(const string& path) {
      ifstream f (path, ifstream::in | ifstream::binary);

      size_t edge_count;
      f.read((char*) &edge_count, sizeof(edge_count));

      edges.clear();
      edges.resize(edge_count);

      f.read((char*) edges.data(), edge_count * sizeof(edge_t));

      f.close();
    };

    unordered_multimap<vertex_id_t, dst_t> to_map() {
      unordered_multimap<vertex_id_t, dst_t> m;
      m.reserve(edges.size());

      for (auto e : edges) {
        m.emplace(e.src, e.dst);
      }
      return m;
    };

    EdgeList<weighted_edge_t> add_weights(bool generate_weights) {
      EdgeList<weighted_edge_t> with_weights;
      with_weights.edges.resize(edges.size());

#pragma omp parallel for
      for (auto i = 0u; i < edges.size(); i++) {
        auto e = edges[i];
        with_weights.edges[i] = weighted_edge_t(e.src, e.dst, e.dst);
      }
      return with_weights;
    }
};

#include "DataSource.h"


#endif //LIVE_GRAPH_TWO_EDGELIST_H
