//
// Created by per on 03.12.20.
//

#include "PageRank.h"
#include <data-structures/CSR.h>
#include <data-structures/BlockedSkipListAdjacencyLists.h>
#include <third-party/gapbs.h>
#include <versioning/VersionedBlockedEdgeIterator.h>
#include "Algorithms.h"

vector<pair<vertex_id_t, double>>
PageRank::page_rank(Driver &driver, TopologyInterface &ds, int iterations, double damping_factor,
                    bool use_raw_neighbourhood, bool use_gapbs) {
  vector<double> scores;
  if (use_raw_neighbourhood && !use_gapbs) {
    scores = page_rank_raw_neighbourhood(driver, ds, iterations, damping_factor);
  } else if (use_gapbs && use_raw_neighbourhood) {
    scores = page_rank_bs_raw(ds, iterations, damping_factor);
  } else if (use_gapbs && !use_raw_neighbourhood) {
    scores = page_rank_bs(ds, iterations, damping_factor);
  } else {
    scores = page_rank_batched_interface(driver, ds, iterations, damping_factor);
  }

  return Algorithms::translate(ds, scores);
}


vector<double> PageRank::page_rank_batched_interface(Driver &driver, TopologyInterface &ds, int max_iters, double damping_factor) {
  const size_t vertices = ds.vertex_count();

  const double init_score = 1.0f / vertices;
  const double base_score = (1.0f - damping_factor) / vertices;

  vector<double> scores(vertices, init_score);
  vector<double> outgoing_contrib(vertices);

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
      scores[u] = base_score + damping_factor * incoming_total;
    }
  }
  return scores;
}

vector<double> PageRank::page_rank_raw_neighbourhood(Driver &driver, TopologyInterface &ds, int max_iters, double damping_factor) {
  const size_t vertices = ds.vertex_count();

  const double init_score = 1.0f / vertices;
  const double base_score = (1.0f - damping_factor) / vertices;

  vector<double> scores(vertices, init_score);
  vector<double> outgoing_contrib(vertices);

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
      for (vertex_id_t n = 0; n < vertices; n++) {
        outgoing_contrib[n] = scores[n] / out_degrees[n];
      }

      for (vertex_id_t u = 0; u < vertices; u++) {
        double incoming_total = 0;

        dst_t *ns = (dst_t *) ds.raw_neighbourhood(u);
        dst_t *end = ns + ds.neighbourhood_size(u);

        while (ns < end) {
          incoming_total += outgoing_contrib[*ns];
          ns++;
        }
        scores[u] = base_score + damping_factor * incoming_total;
      }
    }
  } else if (typeid(ds) == typeid(BlockedSkipListAdjacencyLists)) {
    auto block_size = dynamic_cast<BlockedSkipListAdjacencyLists &>(ds).get_block_size();

    for (int iter = 0; iter < max_iters; iter++) {
      for (vertex_id_t n = 0; n < vertices; n++) {
        outgoing_contrib[n] = scores[n] / out_degrees[n];
      }

      for (vertex_id_t u = 0; u < vertices; u++) {
        double incoming_total = 0;


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
        scores[u] = base_score + damping_factor * incoming_total;
      }
    }
  } else {
    throw NotImplemented();
  }

  return scores;
}

// Implementation based on the reference PageRank for the GAP Benchmark Suite
// https://github.com/sbeamer/gapbs
// The reference implementation has been written by Scott Beamer
//
// Copyright (c) 2015, The Regents of the University of California (Regents)
// All Rights Reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the Regents nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL REGENTS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/*
GAP Benchmark Suite
Kernel: PageRank (PR)
Author: Scott Beamer

Will return pagerank scores for all vertices once total change < epsilon

This PR implementation uses the traditional iterative approach. This is done
to ease comparisons to other implementations (often use same algorithm), but
it is not necesarily the fastest way to implement it. It does perform the
updates in the pull direction to remove the need for atomics.
*/

// The error computation has been removed and the concept of dangling sum has been added from the original GAPBS implementation.
vector<double> PageRank::page_rank_bs(TopologyInterface &ds, int num_iterations, double damping_factor) {
  const uint64_t num_vertices = ds.vertex_count();
  const uint64_t max_physical_vertices = ds.max_physical_vertex();

  const double init_score = 1.0 / num_vertices;
  const double base_score = (1.0 - damping_factor) / num_vertices;

  vector<double> scores(max_physical_vertices);

#pragma omp parallel for
  for (uint64_t v = 0; v < max_physical_vertices; v++) {
    scores[v] = init_score;
  }

  gapbs::pvector<double> outgoing_contrib(max_physical_vertices, 0.0);

  // pagerank iterations
  for (int iteration = 0; iteration < num_iterations; iteration++) {
    double dangling_sum = 0.0;

    // for each node, precompute its contribution to all of its outgoing neighbours and, if it's a sink,
    // add its rank to the `dangling sum' (to be added to all nodes).
#pragma omp parallel for reduction(+:dangling_sum)
    for (uint64_t v = 0; v < max_physical_vertices; v++) {
      uint64_t out_degree = ds.neighbourhood_size_p(v);
      if (out_degree == 0) { // this is a sink
        dangling_sum += scores[v];
      } else {
        outgoing_contrib[v] = scores[v] / out_degree;
      }
    }

    dangling_sum /= num_vertices;

#pragma omp parallel
    {
      // TODO assert when we enter this that we can only do this for Versioned yet. Or built an alterantive way of getting iterators for others.
      sortledton_iterator iter(*dynamic_cast<VersioningBlockedSkipListAdjacencyList*>(dynamic_cast<SnapshotTransaction&>(ds).raw_ds()));
      // compute the new score for each node in the graph
#pragma omp for schedule(dynamic, 64)
      for (uint64_t v = 0; v < max_physical_vertices; v++) {
        ds.neighbourhood_p(v, iter);
        double incoming_total = 0;

        while (iter.has_next()) {
          auto d = iter.next();
          incoming_total += outgoing_contrib[d];
        }
        scores[v] = base_score + damping_factor * (incoming_total + dangling_sum);
      }
    }
  }

  return scores;
}

// The error computation has been removed and the concept of dangling sum has been added from the original GAPBS implementation.
vector<double> PageRank::page_rank_bs_raw(TopologyInterface &ds, int num_iterations, double damping_factor) {
  cout << "Using pagerank on raw data structure" << endl;
  if (typeid(ds) != typeid(SnapshotTransaction &)) {
    throw ConfigurationError("Cannot run GAPBS page rank for anything but VersioningBlockedAdjacencyList");
  }

  auto tx = dynamic_cast<SnapshotTransaction &>(ds);
  auto raw_ds = dynamic_cast<VersioningBlockedSkipListAdjacencyList *>(tx.raw_ds());

  auto start = chrono::steady_clock::now();

  const uint64_t num_vertices = ds.vertex_count();
  const uint64_t max_physical_vertices = ds.max_physical_vertex();

  const double init_score = 1.0 / num_vertices;
  const double base_score = (1.0 - damping_factor) / num_vertices;

  vector<double> scores(max_physical_vertices);

#pragma omp parallel for
  for (uint64_t v = 0; v < max_physical_vertices; v++) {
    scores[v] = init_score;
  }

  gapbs::pvector<double> outgoing_contrib(max_physical_vertices, 0.0);

  // pagerank iterations
  for (int iteration = 0; iteration < num_iterations; iteration++) {
    double dangling_sum = 0.0;

    // for each node, precompute its contribution to all of its outgoing neighbours and, if it's a sink,
    // add its rank to the `dangling sum' (to be added to all nodes).
#pragma omp parallel for reduction(+:dangling_sum)
    for (uint64_t v = 0; v < max_physical_vertices; v++) {
      uint64_t out_degree = ds.neighbourhood_size_p(v);
      if (out_degree == 0) { // this is a sink
        dangling_sum += scores[v];
      } else {
        outgoing_contrib[v] = scores[v] / out_degree;
      }
    }

    dangling_sum /= num_vertices;

    auto version = tx.get_version();

    // compute the new score for each node in the graph
#pragma omp parallel for schedule(dynamic, 64)
    for (uint64_t v = 0; v < max_physical_vertices; v++) {
      double incoming_total = 0;

      // Only works for the unversioned case
      switch (raw_ds->get_set_type(v, version)) {
        case VSKIP_LIST: {
          auto sl = (VSkipListHeader *) raw_ds->raw_neighbourhood_version(v, version);
          while (sl != nullptr) {
            auto n = sl->data;
            auto end = n + sl->size;
            while (n < end) {
              incoming_total += outgoing_contrib[*n];
              n++;
            }
            sl = sl->next_levels[0];
          }
          break;
        }
        case VSINGLE_BLOCK: {
          auto n = (dst_t *) raw_ds->raw_neighbourhood_version(v, version);
          auto end = n + raw_ds->neighbourhood_size_version_p(v, version);
          while (n < end) {
            incoming_total += outgoing_contrib[*n];
            n++;
          }
          break;
        }
      }

      scores[v] = base_score + damping_factor * (incoming_total + dangling_sum);
    }

    auto end = chrono::steady_clock::now();
    size_t milliseconds = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    cout << "Iter " << iteration << " took " << milliseconds << "m illiseconds" << endl;
  }

  auto end = chrono::steady_clock::now();
  size_t milliseconds = chrono::duration_cast<chrono::milliseconds>(end - start).count();
  cout << "PR (" << num_iterations << ") took " << milliseconds << " milliseconds" << endl;

  return scores;
}


// The error computation has been removed and the concept of dangling sum has been added from the original GAPBS implementation.
vector<double> PageRank::page_rank_bs_blocked(TopologyInterface &ds, int num_iterations, double damping_factor) {
  cout << "Using pagerank with blocked iterators" << endl;
  if (typeid(ds) != typeid(SnapshotTransaction &)) {
    throw ConfigurationError("Cannot run GAPBS page rank for anything but VersioningBlockedAdjacencyList");
  }

  auto tx = dynamic_cast<SnapshotTransaction &>(ds);
  auto raw_ds = dynamic_cast<VersioningBlockedSkipListAdjacencyList *>(tx.raw_ds());


  const uint64_t num_vertices = ds.vertex_count();
  const uint64_t max_physical_vertices = ds.max_physical_vertex();

  const double init_score = 1.0 / num_vertices;
  const double base_score = (1.0 - damping_factor) / num_vertices;

  vector<double> scores(max_physical_vertices);

#pragma omp parallel for
  for (uint64_t v = 0; v < max_physical_vertices; v++) {
    scores[v] = init_score;
  }

  gapbs::pvector<double> outgoing_contrib(max_physical_vertices, 0.0);

  // pagerank iterations
  for (int iteration = 0; iteration < num_iterations; iteration++) {
    double dangling_sum = 0.0;

    // for each node, precompute its contribution to all of its outgoing neighbours and, if it's a sink,
    // add its rank to the `dangling sum' (to be added to all nodes).
#pragma omp parallel for reduction(+:dangling_sum)
    for (uint64_t v = 0; v < max_physical_vertices; v++) {
      uint64_t out_degree = ds.neighbourhood_size_p(v);
      if (out_degree == 0) { // this is a sink
        dangling_sum += scores[v];
      } else {
        outgoing_contrib[v] = scores[v] / out_degree;
      }
    }

    dangling_sum /= num_vertices;

#pragma omp parallel
    {
      // TODO assert when we enter this that we can only do this for Versioned yet. Or built an alterantive way of getting iterators for others.
      sortledton_iterator iter(*dynamic_cast<VersioningBlockedSkipListAdjacencyList*>(dynamic_cast<SnapshotTransaction&>(ds).raw_ds()));
      // compute the new score for each node in the graph
#pragma omp for schedule(dynamic, 64)
      for (uint64_t v = 0; v < max_physical_vertices; v++) {
        double incoming_total = 0;
        auto iter = raw_ds->neighbourhood_version_p_blocked(v, tx.get_version());
        while (iter.has_next_block()) {
          auto [start, end] = iter.next_block();
          for (auto n = start; n < end; n++) {
            incoming_total += outgoing_contrib[*n];
          }
        }

        scores[v] = base_score + damping_factor * (incoming_total + dangling_sum);
      }
    }
  }

  return scores;
}
