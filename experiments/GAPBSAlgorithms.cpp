//
// Created by per on 10.02.21.
//

#include <omp.h>

#include <versioning/SnapshotTransaction.h>
#include <versioning/VersioningBlockedSkipListAdjacencyList.h>
#include "data-structures/ToplogyInterface.h"
#include "third-party/gapbs.h"
#include "GAPBSAlgorithms.h"
#include "Algorithms.h"
#include <versioning/VersionedEdgeIterator.h>
#include <versioning/VersionedBlockedEdgeIterator.h>

using namespace gapbs;

/*****************************************************************************
    *                                                                           *
    *  BFS                                                                      *
    *                                                                           *
    ****************************************************************************/
namespace { // anonymous

    /*
    GAP Benchmark Suite
    Kernel: Breadth-First Search (BFS)
    Author: Scott Beamer
    Will return parent array for a BFS traversal from a source vertex
    This BFS implementation makes use of the Direction-Optimizing approach [1].
    It uses the alpha and beta parameters to determine whether to switch search
    directions. For representing the frontier, it uses a SlidingQueue for the
    top-down approach and a Bitmap for the bottom-up approach. To reduce
    false-sharing for the top-down approach, thread-local QueueBuffer's are used.
    To save time computing the number of edges exiting the frontier, this
    implementation precomputes the degrees in bulk at the beginning by storing
    them in parent array as negative numbers. Thus the encoding of parent is:
      parent[x] < 0 implies x is unvisited and parent[x] = -out_degree(x)
      parent[x] >= 0 implies x been visited
    [1] Scott Beamer, Krste Asanović, and David Patterson. "Direction-Optimizing
        Breadth-First Search." International Conference on High Performance
        Computing, Networking, Storage and Analysis (SC), Salt Lake City, Utah,
        November 2012.
    */

    static int64_t
    BUStep(VersioningBlockedSkipListAdjacencyList *ds, SnapshotTransaction &tx, pvector<int64_t> &distances,
           int64_t distance, Bitmap &front, Bitmap &next) {
      const int64_t N = tx.max_physical_vertex();
      int64_t awake_count = 0;
      next.reset();
#pragma omp parallel for reduction(+ : awake_count) schedule(dynamic, 1024)
      for (int64_t u = 0; u < N; u++) {
        if (distances[u] < 0) { // the node has not been visited yet
          bool done = false;

          // TODO needs implementation of early end
          sortledton_iterator iter(*ds);
          tx.neighbourhood_p(u, iter);
          while (!done && iter.has_next()) {
            dst_t n = iter.next();
            if (front.get_bit(n)) {
              distances[u] = distance; // on each BUStep, all nodes will have the same distance
              awake_count++;
              next.set_bit(u);
              done = true;
              iter.close();
            }
          }
        }
      }
      return awake_count;
    }

    static int64_t
    TDStep(VersioningBlockedSkipListAdjacencyList *ds, SnapshotTransaction &tx, pvector<int64_t> &distances,
           int64_t distance, SlidingQueue<int64_t> &queue) {
      int64_t scout_count = 0;

#pragma omp parallel
      {
        QueueBuffer<int64_t> lqueue(queue);
#pragma omp for reduction(+ : scout_count)
        for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++) {
          int64_t u = *q_iter;

          SORTLEDTON_ITERATE(tx, u, {
            int64_t curr_val = distances[e];

            if (curr_val < 0 && compare_and_swap(distances[e], curr_val, distance)) {
              lqueue.push_back(e);
              scout_count += -curr_val;
            }
          });
        }
        lqueue.flush();
      }
      return scout_count;
    }

    static void QueueToBitmap(const SlidingQueue<int64_t> &queue, Bitmap &bm) {
#pragma omp parallel for
      for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++) {
        int64_t u = *q_iter;
        bm.set_bit_atomic(u);
      }
    }

    static void BitmapToQueue(SnapshotTransaction &tx, const Bitmap &bm, SlidingQueue<int64_t> &queue) {
      const int64_t N = tx.max_physical_vertex();

#pragma omp parallel
      {
        QueueBuffer<int64_t> lqueue(queue);
#pragma omp for
        for (int64_t n = 0; n < N; n++)
          if (bm.get_bit(n))
            lqueue.push_back(n);
        lqueue.flush();
      }
      queue.slide_window();
    }

    static pvector<int64_t> InitDistances(SnapshotTransaction &tx) {
      const int64_t N = tx.max_physical_vertex();
      pvector<int64_t> distances(N);

#pragma omp parallel for
      for (int64_t n = 0; n < N; n++) {
        int64_t out_degree = tx.neighbourhood_size_p(n);
        distances[n] = out_degree != 0 ? -out_degree : -1;
      }
      return distances;
    }

} // anon namespace


vector<uint>
GAPBSAlgorithms::bfs(TopologyInterface &ti, uint64_t start_vertex, bool raw_neighbourhood, int alpha, int beta) {
  if (typeid(ti) != typeid(SnapshotTransaction &)) {
    throw NotImplemented();
  }
  if (raw_neighbourhood) {
    throw NotImplemented();
  }
  // The implementation from GAP BS reports the parent (which indeed it should make more sense), while the one required by
  // Graphalytics only returns the distance

  auto tx = dynamic_cast<SnapshotTransaction &>(ti);
  auto ds = dynamic_cast<VersioningBlockedSkipListAdjacencyList *>(tx.raw_ds());

  pvector<int64_t> distances = InitDistances(tx);
  distances[start_vertex] = 0;

  uint64_t vertex_count = tx.max_physical_vertex();
  SlidingQueue<int64_t> queue(vertex_count);
  queue.push_back(start_vertex);
  queue.slide_window();
  Bitmap curr(vertex_count);
  curr.reset();
  Bitmap front(vertex_count);
  front.reset();
  int64_t edges_to_check = tx.edge_count();  // TODO this could be a slow down given my implementation, we could sum up the adjacency set sizes in a parallel for loop
  int64_t scout_count = tx.neighbourhood_size_p(start_vertex);
  int64_t distance = 1; // current distance
  while (!queue.empty()) {
    if (scout_count > edges_to_check / alpha) {
      int64_t awake_count, old_awake_count;
      QueueToBitmap(queue, front);
      awake_count = queue.size();
      queue.slide_window();
      do {
        old_awake_count = awake_count;
        awake_count = BUStep(ds, tx, distances, distance, front, curr);
        front.swap(curr);
        distance++;
      } while ((awake_count >= old_awake_count) ||
               (awake_count > static_cast<int64_t>(vertex_count) / beta));
      BitmapToQueue(tx, front, queue);
      scout_count = 1;
    } else {
      edges_to_check -= scout_count;
      scout_count = TDStep(ds, tx, distances, distance, queue);
      queue.slide_window();
      distance++;
    }
  }

  // TODO remove and do later and only once
  // Translation for correct distnace return values.
  int N = distances.size();
  vector<uint> ret(N);
#pragma omp parallel for
  for (int i = 0; i < N; i++) {
    if (distances[i] < 0) {
      ret[i] = numeric_limits<uint>::max();
    } else {
      ret[i] = distances[i];
    }
  }


  return ret;
}
