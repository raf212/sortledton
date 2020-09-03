//
// Created by per on 31.08.20.
//

#include <memory>
#include <iostream>
#include <chrono>

#include <data-structures/CSR.h>
#include <data-structures/VectorAdjacencyLists.h>
#include <queue>
#include "Driver.h"

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

  reporter.set_dataset(config.base);

  for (const auto& ds : config.data_structures) {
    cout << "Running data structure: " << ds << endl;
    run_data_structure(base, inserts, deletes, ds);
  }
}

void Driver::run_data_structure(SortedCSRDataSource &base, EdgeList &inserts, EdgeList &deletes,
                                DataStructures ds) {
  reporter.set_data_structure(ds);

  TopologyInterface* data_structure;
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
    run_neighbourhood_2_experiment(wrapped_ds);
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
    vector<ulong> distances (ds->vertex_count(), numeric_limits<ulong>::max());
    queue<vertex_id_t> work;
    work.push(start_vertex);

    while (!work.empty()) {
      vertex_id_t v = work.front();
      work.pop();

      BatchedEdgeIterator& iter = ds->neighbourhood(v);
      while (iter.has_next()) {
        auto& batch = dynamic_cast<ContiguousEdgeBatch&>(iter.next());

        dst_t* end = batch.start + batch.size;
        dst_t* n = batch.start;
        while (n < end) {
          if (distances[*n] == maxDistance) {
            distances[*n] = distances[v] + 1;
            work.push(*n);
          }
          n++;
        }
      }
    }

    auto end = chrono::steady_clock::now();
    size_t microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
    run_times.push_back(microseconds);
    reporter.add_repetition(BFS, microseconds);

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
  throw NotImplemented();
}

void Driver::run_neighbourhood_2_experiment(shared_ptr<TopologyInterface> ds) {
  throw NotImplemented();
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



