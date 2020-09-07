//
// Created by per on 31.08.20.
//

#include <memory>
#include <iostream>
#include <chrono>
#include <random>

#include <data-structures/CSR.h>
#include <data-structures/VectorAdjacencyLists.h>
#include <queue>
#include <functional>
#include "Driver.h"

vector<vertex_id_t> select_2_neighbourhood_src(const SortedCSRDataSource& src, int count) {
  vector<vertex_id_t> out;

  auto vertex_count = src.vertex_count();

  mt19937 engine (43);
  uniform_int_distribution<vertex_id_t> distribution(0, vertex_count - 1);

  auto ran = bind(distribution, engine);

  for (int i = 0; i < count; i++) {
    out.push_back(ran());
  }

  return out;
}


void Driver::run() {
  cout << "Starting to run experiments." << endl;

  cout << "Reading base dataset " << config.base.path << endl;
  SortedCSRDataSource base = read_base_dataset();

  EdgeList inserts;
  if (config.experiments.find(INSERT) != config.experiments.end()) {
    cout << "Reading insert dataset " << config.base.path << endl;
    inserts = read_insert_dataset();
  }

  EdgeList deletes;
  if (config.experiments.find(DELETE) != config.experiments.end()) {
    cout << "Reading delete dataset " << config.base.path << endl;
    inserts = read_delete_dataset();
  }

  vector<vertex_id_t> neighbour_2_sources;
  if (config.experiments.find(NEIGHBOUR_2) == config.experiments.end()) {
    neighbour_2_sources = select_2_neighbourhood_src(base, 10000);
  }

  reporter.set_dataset(config.base);

  for (const auto &ds : config.data_structures) {
    cout << "Running data structure: " << ds << endl;
    run_data_structure(base, inserts, deletes, ds, neighbour_2_sources);
  }
}

void Driver::run_data_structure(SortedCSRDataSource &base, EdgeList &inserts, EdgeList &deletes,
                                DataStructures ds, vector<vertex_id_t>& neighbourhood_2_sources) {
  reporter.set_data_structure(ds);

  TopologyInterface *data_structure;
  switch (ds) {
    case CSR_DS: {
      data_structure = new CSR();
      break;
    }
    case VECTOR_ADJACENCY_LIST: {
      data_structure = new VectorAdjacencyLists();
      break;
    }
  }

  shared_ptr<TopologyInterface> wrapped_ds = shared_ptr<TopologyInterface>(data_structure);

  cout << "Loading base dataset." << endl;
  load_base_dataset(wrapped_ds, base);

  if (config.experiments.find(BFS) != config.experiments.end()) {
    run_bfs_experiment(wrapped_ds);
  }
  if (config.experiments.find(TRIANGLE_COUNTING) != config.experiments.end()) {
    run_triangle_counting_experiment(wrapped_ds);
  }
  if (config.experiments.find(NEIGHBOUR_2) != config.experiments.end()) {
    run_neighbourhood_2_experiment(wrapped_ds, neighbourhood_2_sources);
  }
  if (config.experiments.find(INSERT) != config.experiments.end()) {
    run_insert_experiment(wrapped_ds, inserts);
  }
  if (config.experiments.find(DELETE) != config.experiments.end()) {
    run_delete_experiment(wrapped_ds, deletes);
  }
}

void Driver::run_bfs_experiment(shared_ptr<TopologyInterface> ds) {
  vertex_id_t start_vertex = 50;

  cout << "Running BFS experiment ";
  cout.flush();

  vector<size_t> run_times;
  for (int rep = 0; rep < config.repetitions; rep++) {
    // BFS
    auto start = chrono::steady_clock::now();
    ulong maxDistance = numeric_limits<ulong>::max();
    vector<ulong> distances(ds->vertex_count(), numeric_limits<ulong>::max());
    queue<vertex_id_t> work;
    work.push(start_vertex);

    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      BatchedEdgeIterator &iter = ds->neighbourhood(v);
      while (iter.has_next()) {
        auto &batch = dynamic_cast<ContiguousEdgeBatch &>(iter.next());

        dst_t *end = batch.start + batch.size;
        dst_t *n = batch.start;
        while (n < end) {
          if (distances[*n] == maxDistance) {
            distances[*n] = distances[v] + 1;
            work.push(*n);
          }
          n++;
        }
      }
    }
//    shared_ptr<CSR> csr = dynamic_pointer_cast<CSR>(ds);
//    while (!work.empty()) {
//      vertex_id_t v = work.front();
//      work.pop();
//
//      auto n = &(csr->adjacency_lists[csr->adjacency_index[v]]);
//      auto end = &(csr->adjacency_lists[csr->adjacency_index[v + 1]]);
//      while (n < end) {
//        if (distances[*n] == maxDistance) {
//          distances[*n] = distances[v] + 1;
//          work.push(*n);
//        }
//        n++;
//      }
//    }

    auto end = chrono::steady_clock::now();
    size_t microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
    run_times.push_back(microseconds);
    reporter.add_repetition(BFS, rep, microseconds);

    cout << ".";
    cout.flush();
  }

  double average = ((double) sum(run_times)) / (double) run_times.size() * 1000;
  cout << endl << "BFS run in average in " << average << " milliseconds " << endl;
}

void Driver::load_base_dataset(shared_ptr<TopologyInterface> ds, SortedCSRDataSource &base) {
  ds->bulkload(base);
}

void Driver::run_insert_experiment(shared_ptr<TopologyInterface> ds, EdgeList &el) {
  throw NotImplemented();
}

void Driver::run_delete_experiment(shared_ptr<TopologyInterface> ds, EdgeList &el) {
  throw NotImplemented();
}

void Driver::run_triangle_counting_experiment(shared_ptr<TopologyInterface> ds) {
  cout << "Running triangle experiment ";
  cout.flush();

  vector<size_t> run_times;
  for (int rep = 0; rep < config.repetitions; rep++) {
    auto start = chrono::steady_clock::now();

    size_t triangles = 0;
    vector<dst_t > out;

    for (int a = 0; a < ds->vertex_count(); a++) {
      auto &a_neighbours = ds->neighbourhood(a);

      while (a_neighbours.has_next()) {
        auto &a_n_batch = dynamic_cast<ContiguousEdgeBatch &>(a_neighbours.next());

        for (auto b : a_n_batch) {
          ds->intersect_neighbourhood(a, b, out);
          triangles += out.size();
        }
      }
    }
    auto end = chrono::steady_clock::now();

    size_t microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
    run_times.push_back(microseconds);
    reporter.add_repetition(TRIANGLE_COUNTING, rep, microseconds);

    cout << ".";
    cout.flush();
  }

  double average = ((double) sum(run_times)) / (double) run_times.size() * 1000;
  cout << endl << "Triangle counting run in average in " << average << " milliseconds " << endl;
}

void Driver::run_neighbourhood_2_experiment(shared_ptr<TopologyInterface> ds,
        const vector<vertex_id_t>& sources) {
  cout << "Running 2 neighbourhood experiment ";
  cout.flush();

  vector<size_t> run_times;
  for (int rep = 0; rep < config.repetitions; rep++) {
    auto start = chrono::steady_clock::now();

    // Does count neighbours more than once.
    for (const auto& s : sources) {
      size_t count = 0;

      auto& neighbours = ds->neighbourhood(s);
      while (neighbours.has_next()) {

        // TODO make static cast to save time
        auto& batch = dynamic_cast<ContiguousEdgeBatch&>(neighbours.next());

        for (const auto& n : batch) {
          auto& neigbour_neighbours = ds->neighbourhood(n);
          count++;

          while (neigbour_neighbours.has_next()) {
            auto& batch2 = dynamic_cast<ContiguousEdgeBatch&>(neigbour_neighbours.next());

            for (const auto& nn : batch2) {
              count++;
            }
          }
        }
      }
    }
    auto end = chrono::steady_clock::now();

    size_t microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
    run_times.push_back(microseconds);
    reporter.add_repetition(NEIGHBOUR_2, rep, microseconds);

    cout << ".";
    cout.flush();
  }

  double average = ((double) sum(run_times)) / (double) run_times.size() * 1000;
  cout << endl << "2 neighbourhood counting run in average in " << average << " milliseconds " << endl;

}

EdgeList Driver::read_insert_dataset() {
  throw NotImplemented();
}

EdgeList Driver::read_delete_dataset() {
  throw NotImplemented();
}

SortedCSRDataSource Driver::read_base_dataset() {
  SortedCSRDataSource out;
  out.read_from_binary_file(config.base.path);
  return out;
}
