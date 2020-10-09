//
// Created by per on 31.08.20.
//

#include <memory>
#include <iostream>
#include <chrono>
#include <random>

#include <data-structures/CSR.h>
#include <data-structures/VectorAdjacencyLists.h>
#include <data-structures/MallocAdjacencyLists.h>
#include <data-structures/CSRMallocAdjacencyLists.h>
#include <queue>
#include <functional>
#include <data-structures/BlockedLinkedListAdjacencyLists.h>
#include <data-structures/BlockedSkipListAdjacencyLists.h>
#include <cassert>
#include <map>
#include "Driver.h"

#include "BFSSourceSelector.h"
#include "Algorithms.h"
#include "TwoNeighbourSourceSelector.h"

vector<vector<vertex_id_t>> Driver::select_2_neighbourhood_src(const SortedCSRDataSource &src, int count) {
  vector<vector<vertex_id_t>> out;

  TwoNeighbourSourceSelector s(src);
  for (int r = 0; r < config.repetitions; r++) {
    out.push_back(s.get_sources(count));
  }

  return out;
}


void Driver::run() {
  cout << "Starting to run experiments." << endl;

  cout << "Reading base dataset " << config.base.path << endl;
  SortedCSRDataSource base = read_base_dataset();

  EdgeList inserts;
  if (config.experiments.find(INSERT) != config.experiments.end()) {
    cout << "Reading insert dataset " << config.insertions.path << endl;
    inserts = read_insert_dataset();
  }

  EdgeList deletes;
  if (config.experiments.find(DELETE) != config.experiments.end()) {
    cout << "Reading delete dataset " << config.deletions.path << endl;
    inserts = read_delete_dataset();
  }

  vector<vector<vertex_id_t>> neighbour_2_sources;
  if (config.experiments.find(NEIGHBOUR_2) != config.experiments.end()) {
    neighbour_2_sources = select_2_neighbourhood_src(base, 1000);
  }

  reporter.set_dataset(config.base);

  for (const auto &ds : config.data_structures) {
    cout << "Running data structure: " << ds.first << endl;
    run_data_structure(base, inserts, deletes, ds.first, ds.second, neighbour_2_sources);
  }
}

void Driver::run_data_structure(SortedCSRDataSource &base, EdgeList &inserts, EdgeList &deletes,
                                DataStructures ds, const vector<string> &ds_parameters,
                                vector<vector<vertex_id_t>> &neighbourhood_2_sources) {
  reporter.set_data_structure(ds, ds_parameters);

  TopologyInterface* data_structure;
  switch (ds) {
    case CSR_DS: {
      data_structure = new CSR();
      break;
    }
    case VECTOR_ADJACENCY_LIST: {
      bool unordered = true;
      if (!ds_parameters.empty()) {
        unordered = stoi(ds_parameters[0]);
      }
      data_structure = new VectorAdjacencyLists(unordered);
      break;
    }
    case MALLOC_ADJACENCY_LIST: {
      bool unordered = true;
      if (!ds_parameters.empty()) {
        unordered = stoi(ds_parameters[0]);
      }
      data_structure = new MallocAdjacencyLists(unordered);
      break;
    }
    case CSR_MALLOC_ADJACENCY_LIST: {
      bool unordered = true;
      size_t malloc_limit = 0;
      if (!ds_parameters.empty()) {
        malloc_limit = stoi(ds_parameters[0]);
        unordered = stoi(ds_parameters[1]);
      }
      data_structure = new CSRMallocAdjacencyLists(malloc_limit, unordered);
      break;
    }
    case BLOCKED_LINKED_LIST_AL: {
      bool unordered = false;
      size_t block_size = 128;
      if (!ds_parameters.empty()) {
        block_size = stoi(ds_parameters[0]);
        unordered = stoi(ds_parameters[1]);
      }
      data_structure = new BlockedLinkedListAdjacencyLists(block_size, unordered,
                                                           base.adjacency_lists.size() + inserts.edges.size() + 100,
                                                           base.vertex_count());
      break;
    }
    case BLOCKED_SKIP_LIST_AL: {
      bool unordered = false;
      size_t block_size = 128;
      if (!ds_parameters.empty()) {
        block_size = stoi(ds_parameters[0]);
        unordered = stoi(ds_parameters[1]);
      }
      data_structure = new BlockedSkipListAdjacencyLists(block_size, 6, unordered,
                                                         base.adjacency_lists.size() + inserts.edges.size() + 100,
                                                         base.vertex_count());
      break;
    }
    default: {
      throw ConfigurationError("Forgot to implement data structure: " + ds);
    }
  }

  cout << "Loading base dataset." << endl;
  load_base_dataset(*data_structure, base);

  if (config.experiments.find(NEIGHBOUR_2) != config.experiments.end()) {
    run_neighbourhood_2_experiment(*data_structure, neighbourhood_2_sources);
  }
  if (config.experiments.find(BFS) != config.experiments.end()) {
    run_bfs_experiment(*data_structure);
  }
  if (config.experiments.find(TRIANGLE_COUNTING) != config.experiments.end()) {
    run_triangle_counting_experiment(*data_structure);
  }
  if (config.experiments.find(COMMUNITY_DETECTION) != config.experiments.end()) {
    run_community_detection(*data_structure);
  }
  if (config.experiments.find(INSERT) != config.experiments.end()) {
    run_insert_experiment(*data_structure, inserts);
  }
  if (config.experiments.find(DELETE) != config.experiments.end()) {
    run_delete_experiment(*data_structure, deletes);
  }

  if (config.validate_datastructures) {
    validate_graph_structure(*data_structure, base, inserts, deletes);
  }
}

void Driver::run_bfs_experiment(TopologyInterface& ds) {
  BFSSourceSelector ss(*this, config.base, ds);
  vertex_id_t start_vertex = ss.get_source();

  cout << "Running BFS experiment ";
  cout.flush();

  vector<uint> distances;

  vector<size_t> run_times;
  for (int rep = 0; rep < config.repetitions; rep++) {
    // BFS
    auto start = chrono::steady_clock::now();
    distances = Algorithms::bfs(*this, ds, start_vertex);
    auto end = chrono::steady_clock::now();

    size_t microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();

    run_times.push_back(microseconds);
    reporter.add_repetition(BFS, rep, microseconds);


    cout << ".";
    cout.flush();

#ifdef DEBUG
    check_bfs(start_vertex, distances, false);
#endif
  }

  auto traversed_vertices = Algorithms::traversed_vertices(ds, distances);
  cout << "Traversed vertices " << traversed_vertices << " from " << ds.vertex_count() << " " << (float) traversed_vertices / (float) ds.vertex_count() << "%" << endl;
  double average = ((double) sum(run_times)) / (double) run_times.size() * 1000;
  cout << endl << "BFS run in average in " << average << " milliseconds " << endl;
}

void Driver::load_base_dataset(TopologyInterface& ds, SortedCSRDataSource &base) {
  ds.bulkload(base);
}

void Driver::run_insert_experiment(TopologyInterface& ds, EdgeList &el) {
  cout << "Running insert experiment " << endl;

  vector<size_t> run_times;

  auto start = chrono::steady_clock::now();

  try {
    for (const auto &edge : el) {
      ds.insert_edge(edge);
    }
    auto end = chrono::steady_clock::now();

    size_t microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
    run_times.push_back(microseconds);
    reporter.add_repetition(INSERT, 0, microseconds);

    double average = ((double) sum(run_times)) / (double) run_times.size() * 1000;
    cout << "Inserting took: " << average << " milliseconds " << endl;

#ifdef DEBUG
    check_insert(ds, el);
#endif
  } catch (const NotImplemented &e) {
    cout << "Insertion not supported by ds: " << typeid(ds).name() << endl;
  }
}

void Driver::run_delete_experiment(TopologyInterface& ds, EdgeList &el) {
  throw NotImplemented();
}

void Driver::run_triangle_counting_experiment(TopologyInterface& ds) {
  cout << "Running triangle experiment ";
  cout.flush();

  vector<size_t> run_times;
  size_t triangles;
  for (int rep = 0; rep < config.repetitions; rep++) {
    auto start = chrono::steady_clock::now();

    triangles = 0;
    vector<dst_t> out;

    VectorBatchedEdgeIterator a_neighbours;
    for (int a = 0; a < ds.vertex_count(); a++) {
      ds.neighbourhood(a, a_neighbours);

      while (a_neighbours.has_next()) {
        auto &a_n_batch = a_neighbours.next();

        for (auto b : a_n_batch) {
          if (a < b) {
            ds.intersect_neighbourhood(a, b, out);
            for (auto c : out) {
              if (b < c) {
                triangles += 1;
              }
            }
          }
        }
      }
    }
    auto end = chrono::steady_clock::now();

    size_t microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
    run_times.push_back(microseconds);
    reporter.add_repetition(TRIANGLE_COUNTING, rep, microseconds);

    cout << ".";
    cout.flush();

#ifdef DEBUG
    check_triangle_counting(triangles);
#endif
  }


  double average = ((double) sum(run_times)) / (double) run_times.size() * 1000;
  cout << endl << "Triangle counting run in average in " << average << " milliseconds " << endl;
  cout << "Counted " << triangles << " triangles." << endl;
}

void Driver::run_neighbourhood_2_experiment(TopologyInterface& ds,
                                            const vector<vector<vertex_id_t>> &sources) {
  cout << "Running 2 neighbourhood experiment ";
  cout.flush();

  vector<size_t> run_times;

  // Does count neighbours more than once.
  ContigiousBlockIterator &neighbour_neighbours = getIter(ds);
  ContigiousBlockIterator &neighbours = getIter(ds);
  ContigiousBlockIterator &neighbours_3 = getIter(ds);
  for (int rep = 0; rep < config.repetitions; rep++) {
    auto start = chrono::steady_clock::now();

    unordered_map<vertex_id_t, size_t> neighbour_counts;
    for (const auto &s : sources[rep]) {
      size_t count = 0;
      ds.neighbourhood(s, neighbours);
      while (neighbours.has_next()) {
        auto &batch = neighbours.next();
        for (const auto &n : batch) {
          ds.neighbourhood(n, neighbours_3);
          count++;

          while (neighbours_3.has_next()) {
            auto &batch2 = neighbours_3.next();

            for (const auto &nn : batch2) {
//              cout << s << " " << n << " " << nn << endl;
              count++;
            }
          }
        }
      }
//      if (count > 10000) {
//        break;
//      }
      neighbour_counts.insert(make_pair(s, count));
    }
    auto end = chrono::steady_clock::now();

    size_t microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
    run_times.push_back(microseconds);
    reporter.add_repetition(NEIGHBOUR_2, rep, microseconds);

    cout << ".";
    cout.flush();

#ifdef DEBUG
    if (rep == 0) { // Gold standard only saves the result from rep==0 runs, they differ in the set of sources.
      check_neighbourhood_2(neighbour_counts);
    }
#endif
  }

  double average = ((double) sum(run_times)) / (double) run_times.size() * 1000;
  cout << endl << "2 neighbourhood counting run in average in " << average << " milliseconds " << endl;

}

EdgeList Driver::read_insert_dataset() {
  EdgeList edge_list;
  edge_list.read_from_binary_file(config.insertions.path);
  return edge_list;
}

EdgeList Driver::read_delete_dataset() {
  throw NotImplemented();
}

SortedCSRDataSource Driver::read_base_dataset() {
  SortedCSRDataSource out;
  out.read_from_binary_file(config.base.path);
  return out;
}

ContigiousBlockIterator &Driver::getIter(TopologyInterface &ds) {
  if (typeid(ds) == typeid(BlockedLinkedListAdjacencyLists) || typeid(ds) == typeid(BlockedSkipListAdjacencyLists)) {
    blockIterators.push_back(BlockedBatchedEdgeIterator());
    return blockIterators[blockIterators.size() - 1];
  } else {
    vectorIterators.push_back(VectorBatchedEdgeIterator());
    return vectorIterators[vectorIterators.size() - 1];
  }
}

void Driver::check_bfs(vertex_id_t start_vertex, vector<uint>& distances, bool validate_inserts) {
  cout << "Validating bfs experiment" << endl;
  string inserts = "base";
  if (validate_inserts) {
    inserts = "inserts";
  }
  const string gold_standard_file =
          config.gold_standard_directory + "/bfs_" + config.base.get_name() + "_" + to_string(start_vertex) + "_" +
          inserts + ".goldStandard";
  if (!file_exists(gold_standard_file)) {
    cout << "Writing new gold standard for: " << gold_standard_file << endl;
    ofstream f(gold_standard_file, ofstream::binary | ofstream::out);

    if (!f.good()) {
      assert(false);
    }

    auto size = distances.size();
    f.write((char *) &size, sizeof(size));

    for (auto d : distances) {
      f.write((char *) &d, sizeof(d));
    }
    f.close();
  } else {
    ifstream f(gold_standard_file, ifstream::in | ifstream::binary);

    size_t size;
    f.read((char *) &size, sizeof(size));

    assert(size == distances.size());

    ulong e;
    for (auto d : distances) {
      f.read((char *) &e, sizeof(d));
      assert(d == e);
    }

    f.close();
  }
}

void Driver::validate_graph_structure(TopologyInterface& ds, SortedCSRDataSource &base, EdgeList &inserts,
                                      EdgeList &deletes) {
  cout << "Validating data structure." << endl;
  auto vertices = base.vertex_count();

  unordered_multimap<vertex_id_t, dst_t> insert_map;
  unordered_multimap<vertex_id_t, dst_t> delete_map;
  if (config.experiments.find(INSERT) != config.experiments.end()) {
    insert_map = inserts.to_map();
  }
  if (config.experiments.find(DELETE) != config.experiments.end()) {
    delete_map = deletes.to_map();
  }
  for (vertex_id_t v = 0; v < vertices; v++) {
//    cout << "Vertex " << v << endl;
    unordered_set<dst_t> e_neighbours = base.get_neighbour_set(v);

    unordered_set<dst_t> e_deleted = get_values_from_multimap(delete_map, v);
    unordered_set<dst_t> e_inserted = get_values_from_multimap(insert_map, v);

    unordered_set<dst_t> a_neighbours = get_neighbours(ds, v);

    for (auto n : a_neighbours) {
      if (e_neighbours.find(n) == e_neighbours.end()) {
        assert(e_inserted.find(n) != e_inserted.end());
      } else {
        assert(e_deleted.find(n) == e_deleted.end());
      }
    }

    for (auto n : e_neighbours) {
      assert(a_neighbours.find(n) != a_neighbours.end() || e_deleted.find(n) != e_deleted.end());
    }
    for (auto n: e_inserted) {
      assert(a_neighbours.find(n) != a_neighbours.end() || e_deleted.find(n) != e_deleted.end());
    }
  }
}


unordered_set<dst_t> Driver::get_neighbours(TopologyInterface& ds, vertex_id_t v) {
  unordered_set<dst_t> neighbours;

  auto &ns = getIter(ds);
  ds.neighbourhood(v, ns);
  while (ns.has_next()) {
    auto block = ns.next();

    for (auto e : block) {
      neighbours.insert(e);
    }
  }
  return neighbours;
}

void Driver::check_insert(TopologyInterface& ds, EdgeList &el) {
  cout << "checking inserts" << endl;
  for (auto e : el.edges) {
    assert(ds.has_edge(e));
  }

  BFSSourceSelector ss(*this, config.base, ds);
  vertex_id_t start_vertex = ss.get_source();

  auto distances = Algorithms::bfs(*this, ds, start_vertex);
  check_bfs(start_vertex, distances, true);
  check_bfs(start_vertex, distances, true);
}

void Driver::check_neighbourhood_2(unordered_map<vertex_id_t, size_t> neighbour_counts) {
  cout << "Validating 2-neighbourhood experiment" << endl;
  const string gold_standard_file =
          config.gold_standard_directory + "/neighbour2_" + config.base.get_name() + ".goldStandard";
  if (!file_exists(gold_standard_file)) {
    cout << "Writing new gold standard for: " << gold_standard_file << endl;
    ofstream f(gold_standard_file, ofstream::binary | ofstream::out);

    if (!f.good()) {
      assert(false);
    }

    size_t size = neighbour_counts.size();
    f.write((char *) &size, sizeof(size));

    for (auto nc : neighbour_counts) {
      f.write((char *) &(nc.first), sizeof(vertex_id_t));
      f.write((char *) &(nc.second), sizeof(size_t));
    }
    f.close();
  } else {
    ifstream f(gold_standard_file, ifstream::in | ifstream::binary);

    size_t size;
    f.read((char *) &size, sizeof(size));

    assert(size == neighbour_counts.size());

    vertex_id_t v;
    size_t c;
    for (int i = 0; i < size; i++) {
      f.read((char *) &v, sizeof(v));
      f.read((char *) &c, sizeof(c));

      auto a = neighbour_counts.find(v);
      assert(a != neighbour_counts.end() && a->second == c);
    }


    f.close();
  }
}

void Driver::check_triangle_counting(size_t count) {
  cout << "Validating triangle experiment" << endl;
  const string gold_standard_file =
          config.gold_standard_directory + "/triangle_" + config.base.get_name() + ".goldStandard";
  if (!file_exists(gold_standard_file)) {
    cout << "Writing new gold standard for: " << gold_standard_file << endl;
    ofstream f(gold_standard_file, ofstream::binary | ofstream::out);

    if (!f.good()) {
      assert(false);
    }

    f.write((char *) &count, sizeof(count));
    f.close();
  } else {
    ifstream f(gold_standard_file, ifstream::in | ifstream::binary);

    size_t e_count;
    f.read((char *) &e_count, sizeof(e_count));

    assert(e_count == count);

    f.close();
  }
}

// TODO add get_neighbourcount function for data structure

void Driver::run_community_detection(TopologyInterface& ds) {
  cout << "Running community detection experiment ";
  cout.flush();

  const uint max_iterations = 5;

  vector<size_t> run_times;

  size_t vertex_count = ds.vertex_count();

  for (int rep = 0; rep < config.repetitions; rep++) {
    auto start = chrono::steady_clock::now();

    vector<bool> active1(vertex_count, true);
    vector<bool> active2(vertex_count, false);

    auto &active_old = active1;
    auto &active_new = active2;

    vector<vertex_id_t> labels1(vertex_count);
    vector<vertex_id_t> labels2(vertex_count);

    auto &l_old = labels1;
    auto &l_new = labels2;

    ContigiousBlockIterator &neighbours = getIter(ds);

    for (vertex_id_t v = 0; v < ds.vertex_count(); v++) {
      l_old[v] = v;
    }

    // Needs to be ordered for correctness; to find the minimum label.
    map<vertex_id_t, size_t> label_counts;
    bool done = false;

    uint iterations = 0;
    while (!done && iterations <= max_iterations) {
      size_t vertices_changed = 0;
      done = true;

      for (vertex_id_t v = 0; v < vertex_count; v++) {
        label_counts.clear();

        ds.neighbourhood(v, neighbours);
        while (neighbours.has_next()) {
          auto &block = neighbours.next();

          for (auto n : block) {
            auto l = l_old[n];
            auto lc = label_counts.find(l);
            if (lc == label_counts.end()) {
              label_counts.insert({l, 1});
            } else {
              lc->second++;
            }
          }
        }

        vertex_id_t new_label;
        auto max_count = 0;
        for (auto lc : label_counts) {
          if (max_count < lc.second) {
            max_count = lc.second;
            new_label = lc.first;
          }
        }
        l_new[v] = new_label;
        if (l_old[v] != l_new[v]) {
          done = false;
          vertices_changed++;
        }
      }

      swap(l_old, l_new);
      iterations++;
    }

    auto end = chrono::steady_clock::now();

    size_t microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
    run_times.push_back(microseconds);
    reporter.add_repetition(COMMUNITY_DETECTION, rep, microseconds);

    cout << ".";
    cout.flush();

#ifdef DEBUG
    check_community_detection(l_new);
#endif
  }

  double average = ((double) sum(run_times)) / (double) run_times.size() * 1000;
  cout << endl << "community detection run in average in " << average << " milliseconds " << endl;

}


// TODO shouldn't community detection converge?
void Driver::check_community_detection(vector<vertex_id_t> labels) {
  cout << "Validating community experiment" << endl;
  const string gold_standard_file =
          config.gold_standard_directory + "/community_" + config.base.get_name() + ".goldStandard";
  if (!file_exists(gold_standard_file)) {
    cout << "Writing new gold standard for: " << gold_standard_file << endl;
    ofstream f(gold_standard_file, ofstream::binary | ofstream::out);

    if (!f.good()) {
      assert(false);
    }

    size_t vertex_count = labels.size();
    f.write((char *) &vertex_count, sizeof(vertex_count));

    for (vertex_id_t v = 0; v < vertex_count; v++) {
      f.write((char *) &labels[v], sizeof(vertex_id_t));
    }
    f.close();
  } else {
    ifstream f(gold_standard_file, ifstream::in | ifstream::binary);

    size_t vertex_count;
    f.read((char *) &vertex_count, sizeof(vertex_count));

    assert(vertex_count == labels.size());

    vertex_id_t l;
    for (vertex_id_t v = 0; v < vertex_count; v++) {
      f.read((char *) &l, sizeof(l));
      assert(labels[v] == l);
    }

    f.close();
  }

}

void Driver::print_graph(TopologyInterface& ds) {
  ContigiousBlockIterator &ns = getIter(ds);
  for (vertex_id_t v = 0; v < ds.vertex_count(); v++) {
    ds.neighbourhood(v, ns);
    while (ns.has_next()) {
      auto &block = ns.next();

      for (dst_t &n : block) {
        cout << v << " " << n << endl;
      }
    }
  }
}