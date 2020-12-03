//
// Created by per on 03.12.20.
//

#include "PageRank.h"

vector<float> PageRank::page_rank_batched_interface(Driver &driver, TopologyInterface &ds, int max_iters, double epsilon) {
  const size_t vertices = ds.vertex_count();

  const float init_score = 1.0f / vertices;
  const float base_score = (1.0f - damping_factor) / vertices;

  vector<float> scores(vertices, init_score);
  vector<float> outgoing_contrib(vertices);

  vector<uint> out_degrees(vertices, 0);

  for (vertex_id_t v = 0; v < vertices; v++) {
    auto& in_neighbours = driver.getIter(ds);

    ds.neighbourhood(v, in_neighbours);
    while(in_neighbours.has_next()) {
      auto &batch = in_neighbours.next();

      for (auto n : batch) {
        out_degrees[n]++;
      }
    }
  }

  auto& neighbours = driver.getIter(ds);
  for (int iter=0; iter < max_iters; iter++) {
    double error = 0;

    for (vertex_id_t n=0; n < vertices; n++) {
      outgoing_contrib[n] = scores[n] / out_degrees[n];
    }

    for (vertex_id_t u=0; u < vertices; u++) {
      float incoming_total = 0;

      ds.neighbourhood(u, neighbours);

      while (neighbours.has_next()) {
        auto &batch = neighbours.next();
        for (vertex_id_t v : batch) {
          incoming_total += outgoing_contrib[v];
        }
      }


      float old_score = scores[u];
      scores[u] = base_score + damping_factor * incoming_total;
      error += fabs(scores[u] - old_score);
    }
    if (error < epsilon)
      break;
  }
  return scores;
}
