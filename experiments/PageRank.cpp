//
// Created by per on 03.12.20.
//

#include "PageRank.h"
#include <data-structures/CSR.h>
#include <data-structures/BlockedSkipListAdjacencyLists.h>

vector<float>
PageRank::page_rank_batched_interface(Driver &driver, TopologyInterface &ds, int max_iters, double epsilon) {
  const size_t vertices = ds.vertex_count();

  const float init_score = 1.0f / vertices;
  const float base_score = (1.0f - damping_factor) / vertices;

  vector<float> scores(vertices, init_score);
  vector<float> outgoing_contrib(vertices);

  vector<uint> out_degrees(vertices, 0);

  for (vertex_id_t v = 0; v < vertices; v++) {
    auto &in_neighbours = driver.getIter(ds);

    ds.neighbourhood(v, in_neighbours);
    while (in_neighbours.has_next()) {
      auto &batch = in_neighbours.next();

      for (auto n : batch) {
        out_degrees[n]++;
      }
    }
  }

  auto &neighbours = driver.getIter(ds);
  for (int iter = 0; iter < max_iters; iter++) {
    double error = 0;

    for (vertex_id_t n = 0; n < vertices; n++) {
      outgoing_contrib[n] = scores[n] / out_degrees[n];
    }

    for (vertex_id_t u = 0; u < vertices; u++) {
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

vector<float> PageRank::page_rank_raw_neighbourhood(Driver &driver, TopologyInterface &ds, int max_iters, double epsilon) {
  const size_t vertices = ds.vertex_count();

  const float init_score = 1.0f / vertices;
  const float base_score = (1.0f - damping_factor) / vertices;

  vector<float> scores(vertices, init_score);
  vector<float> outgoing_contrib(vertices);

  vector<uint> out_degrees(vertices, 0);

  for (vertex_id_t v = 0; v < vertices; v++) {
    auto &in_neighbours = driver.getIter(ds);

    ds.neighbourhood(v, in_neighbours);
    while (in_neighbours.has_next()) {
      auto &batch = in_neighbours.next();

      for (auto n : batch) {
        out_degrees[n]++;
      }
    }
  }

  if (typeid(ds) == typeid(CSR)) {
    for (int iter = 0; iter < max_iters; iter++) {
      double error = 0;

      for (vertex_id_t n = 0; n < vertices; n++) {
        outgoing_contrib[n] = scores[n] / out_degrees[n];
      }

      for (vertex_id_t u = 0; u < vertices; u++) {
        float incoming_total = 0;

        dst_t *ns = (dst_t *) ds.raw_neighbourhood(u);
        dst_t *end = ns + ds.neighbourhood_size(u);

        while (ns < end) {
          incoming_total += outgoing_contrib[*ns];
          ns++;
        }

        float old_score = scores[u];
        scores[u] = base_score + damping_factor * incoming_total;
        error += fabs(scores[u] - old_score);
      }
      if (error < epsilon)
        break;
    }
  } else if (typeid(ds) == typeid(BlockedSkipListAdjacencyLists)) {
    auto block_size = dynamic_cast<BlockedSkipListAdjacencyLists &>(ds).get_block_size();

    for (int iter = 0; iter < max_iters; iter++) {
      double error = 0;

      for (vertex_id_t n = 0; n < vertices; n++) {
        outgoing_contrib[n] = scores[n] / out_degrees[n];
      }

      for (vertex_id_t u = 0; u < vertices; u++) {
        float incoming_total = 0;


        if (ds.neighbourhood_size(u) <= block_size) {
          dst_t *ns = (dst_t *) ds.raw_neighbourhood(u);
          dst_t *end = ns + ds.neighbourhood_size(u);

          while (ns < end) {
            incoming_total += outgoing_contrib[*ns];
            ns++;
          }
        } else {
          BlockHeader *block = (BlockHeader *) ds.raw_neighbourhood(u);
          while (block != nullptr) {
            auto data = block->data;
            dst_t *end = block->data + block->size;

            while (data < end) {
              incoming_total += outgoing_contrib[*data];
              data++;
            }
            block = block->next;
          }
        }

        float old_score = scores[u];
        scores[u] = base_score + damping_factor * incoming_total;
        error += fabs(scores[u] - old_score);
      }
      if (error < epsilon)
        break;
    }
  } else {
    throw NotImplemented();
  }

  return scores;
}
