//
// Created by per on 31.08.20.
//

#include <memory>
#include <iostream>
#include <chrono>
#include <random>
#include <iomanip>

#include <data-structures/CSR.h>
#include <data-structures/VectorAdjacencyLists.h>
#include <data-structures/MallocAdjacencyLists.h>
#include <data-structures/CSRMallocAdjacencyLists.h>
#include <queue>
#include <functional>
#include <data-structures/BlockedLinkedListAdjacencyLists.h>
#include <data-structures/BlockedSkipListAdjacencyLists.h>
#include <data-structures/HashSetSimulatorAdjacencyList.h>
#include <cassert>
#include <map>
#include <thread>
#include <atomic>
#include <data-structures/HashSetAdjacencyLists.h>
#include <versioning/SnapshotTransaction.h>
#include <versioning/TransactionManager.h>
#include <versioning/VersioningBlockedSkipListAdjacencyList.h>
#include "Driver.h"

#include "BFSSourceSelector.h"
#include "Algorithms.h"
#include "TwoNeighbourSourceSelector.h"
#include "TwoNeighbour.h"

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
  if (config.experiment_set.find(INSERT) != config.experiment_set.end() ||
      config.experiment_set.find(INSERT_TRANSACTIONS) != config.experiment_set.end()) {
    cout << "Reading insert dataset " << config.insertions.path << endl;
    inserts = read_insert_dataset();
  }

  EdgeList deletes;
  if (config.experiment_set.find(DELETE) != config.experiment_set.end()) {
    cout << "Reading delete dataset " << config.deletions.path << endl;
    inserts = read_delete_dataset();
  }

  vector<vector<vertex_id_t>> neighbour_2_sources;
  if (config.experiment_set.find(NEIGHBOUR_2) != config.experiment_set.end()) {
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

  TopologyInterface *data_structure;
  string ds_name;

  TransactionManager tm(config.insert_threads + 1);
  auto master_thread_id = tm.register_thread();
  VersionedTopologyInterface *versioned_data_structure = nullptr;
  SnapshotTransaction transaction(0, nullptr);

  switch (ds) {
    case CSR_DS: {
      data_structure = new CSR();
      ds_name = "CSR";
      break;
    }
    case VECTOR_ADJACENCY_LIST: {
      bool unordered = true;
      if (!ds_parameters.empty()) {
        unordered = stoi(ds_parameters[0]);
      }
      data_structure = new VectorAdjacencyLists(unordered);
      ds_name = "vectorAL";
      break;
    }
    case MALLOC_ADJACENCY_LIST: {
      bool unordered = true;
      bool use_hash_index = false;
      if (!ds_parameters.empty()) {
        unordered = stoi(ds_parameters[0]);
        use_hash_index = stoi(ds_parameters[1]);
      }
      data_structure = new MallocAdjacencyLists(unordered, use_hash_index);
      ds_name = "mallocAL";
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
      ds_name = "csrMallocAL";
      break;
    }
    case BLOCKED_LINKED_LIST_AL: {
      bool unordered = false;
      size_t block_size = 128;
      bool adjust_pool_sizes = false;
      bool size_in_index = false;
      if (!ds_parameters.empty()) {
        block_size = stoi(ds_parameters[0]);
        unordered = stoi(ds_parameters[1]);
        if (ds_parameters[2] == "adjust") {
          cout << "Adjusting pool sizes activated" << endl;
          adjust_pool_sizes = true;
        }
        if (ds_parameters[3] == "si") {
          cout << "Size stored in index" << endl;
          size_in_index = true;
        }
      }
      data_structure = new BlockedLinkedListAdjacencyLists(block_size, unordered,
                                                           base.adjacency_lists.size() + inserts.edges.size() + 100,
                                                           base.vertex_count(),
                                                           adjust_pool_sizes,
                                                           size_in_index);
      ds_name = "bllAL";
      break;
    }
    case BLOCKED_SKIP_LIST_AL: {
      bool unordered = false;
      size_t block_size = 128;
      if (!ds_parameters.empty()) {  // TODO better parameter sanitization
        block_size = stoi(ds_parameters[0]);
        unordered = stoi(ds_parameters[1]);
      }
      data_structure = new BlockedSkipListAdjacencyLists(block_size, 6, unordered,
                                                         base.adjacency_lists.size() + inserts.edges.size() + 100,
                                                         base.vertex_count());
      ds_name = "blsAL";
      break;
    }
    case VERSIONED: {
      size_t block_size = 128;
      if (!ds_parameters.empty()) {  // TODO better parameter sanitization
        block_size = stoi(ds_parameters[0]);
      }
      versioned_data_structure = new VersioningBlockedSkipListAdjacencyList(block_size, tm);
      ds_name = "versioned";
      break;
    }
    case HASH_SET_SIMULATOR_AL: {
      float fill_factor = 0.9;
      if (!ds_parameters.empty()) {
        fill_factor = stof(ds_parameters[0]);
      }
      data_structure = new HashSetSimulatorAdjacencyList(fill_factor);
      ds_name = "hssAL";
      break;
    }
    case HASH_SET_AL: {
      data_structure = new HashSetAdjacencyLists();
      ds_name = "hsAL";
      break;
    }
    default: {
      throw ConfigurationError("Forgot to implement data structure: " + ds);
    }
  }

  cout << "Loading base dataset." << endl;
  if (versioned_data_structure != nullptr) {
    transaction = tm.getSnapshotTransaction(versioned_data_structure, master_thread_id);
    data_structure = &transaction;
  }
  load_base_dataset(*data_structure, base);
  if (versioned_data_structure != nullptr) {
    tm.transactionCompleted(transaction, master_thread_id);
  }

  bool run_on_raw_neighbourhood = false;
  if (find(ds_parameters.begin(), ds_parameters.end(), "raw") != ds_parameters.end()) {
    run_on_raw_neighbourhood = true;
  }

  bool inserts_run = false;
  for (auto e : config.experiments) {
    if (versioned_data_structure != nullptr) {
      transaction = tm.getSnapshotTransaction(versioned_data_structure, master_thread_id);
      data_structure = &transaction;
    }
    switch (e.first) {
      case (NEIGHBOUR_2): {
        run_neighbourhood_2_experiment(*data_structure, neighbourhood_2_sources, run_on_raw_neighbourhood);
        if (versioned_data_structure != nullptr) {
          tm.transactionCompleted(transaction, master_thread_id);
        }
        break;
      }
      case (BFS): {
        bool aquire_locks = false;
        if (!e.second.empty()) {
          aquire_locks = stoi(e.second[0]);
        }
        run_bfs_experiment(*data_structure, run_on_raw_neighbourhood, aquire_locks, inserts_run);
        if (versioned_data_structure != nullptr) {
          tm.transactionCompleted(transaction, master_thread_id);
        }
        break;
      }
      case (TRIANGLE_COUNTING): {
        if (run_on_raw_neighbourhood) {
          throw NotImplemented();
        }
        run_triangle_counting_experiment(*data_structure);
        if (versioned_data_structure != nullptr) {
          tm.transactionCompleted(transaction, master_thread_id);
        }
        break;
      }
      case (COMMUNITY_DETECTION): {
        throw NotImplemented("Current implementation is incorrect");
        if (run_on_raw_neighbourhood) {
          throw NotImplemented();
        }
        run_community_detection(*data_structure);
        if (versioned_data_structure != nullptr) {
          tm.transactionCompleted(transaction, master_thread_id);
        }
        break;
      }
      case (PR): {
        run_page_rank_experiment(*data_structure, run_on_raw_neighbourhood);
        if (versioned_data_structure != nullptr) {
          tm.transactionCompleted(transaction, master_thread_id);
        }
        break;
      }
      case (INSERT): {
        run_insert_experiment(tm, *data_structure, inserts);
        inserts_run = true;
        break;
      }
      case (INSERT_TRANSACTIONS): {
        if (typeid(*data_structure) != typeid(SnapshotTransaction)) {
          cout << "Skipping experiment insert transactions for data structure " << ds_name << endl;
          continue;
        }
        // Master thread generates its own transaction after insertion happened.
        if (versioned_data_structure != nullptr) {
          tm.transactionCompleted(transaction, master_thread_id);
        }
        run_insert_experiment_one_by_one(tm, versioned_data_structure, inserts);
        inserts_run = true;
        break;
      }
      case (DELETE): {
        if (run_on_raw_neighbourhood) {
          throw NotImplemented();
        }
        // Master thread generates its own transaction after insertion happened.
        if (versioned_data_structure != nullptr) {
          tm.transactionCompleted(transaction, master_thread_id);
        }
        run_delete_experiment(*data_structure, deletes);
        break;
      }
      case (GC): {
        if (versioned_data_structure == nullptr) {
          cout << "Skipping GC experiment for data structure " << ds_name << endl;
          continue;
        }
        tm.transactionCompleted(transaction, master_thread_id);
        run_gc_experiment(tm, *versioned_data_structure, inserts_run, inserts);
        break;
      }

      case (STORAGE): {
        show_storage_sizes(ds_name, *data_structure);
        if (versioned_data_structure != nullptr) {
          tm.transactionCompleted(transaction, master_thread_id);
        }
        break;
      }
      default:
        throw ConfigurationError("Unknown experiment type");
    }
    cout << endl;
  }

  if (config.validate_datastructures) {
    validate_graph_structure(*data_structure, base, inserts, deletes);
  }
}

void
Driver::run_bfs_experiment(TopologyInterface &ds, bool run_on_raw_neighbourhood, bool aquire_locks,
                           bool after_inserts) {
  BFSSourceSelector ss(*this, config.base, ds);
  vertex_id_t start_vertex = ss.get_source();

  cout << "Running BFS experiment ";
  cout.flush();

  vector<uint> distances;

  vector<size_t> run_times;
  for (int rep = 0; rep < config.repetitions; rep++) {
    // BFS
    auto start = chrono::steady_clock::now();
    distances = Algorithms::bfs(*this, ds, start_vertex, run_on_raw_neighbourhood, aquire_locks);
    auto end = chrono::steady_clock::now();

    size_t microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();

    run_times.push_back(microseconds);
    reporter.add_repetition(BFS, rep, microseconds);


    cout << ".";
    cout.flush();

#ifdef DEBUG
    version_t version = after_inserts ? 1 : FIRST_VERSION;
    if (typeid(ds) == typeid(SnapshotTransaction)) {
      version = after_inserts ? dynamic_cast<SnapshotTransaction &>(ds).get_version() : FIRST_VERSION;
    }
    check_bfs(start_vertex, distances, version);
#endif
  }

  auto traversed_vertices = Algorithms::traversed_vertices(ds, distances);
  cout << "Traversed vertices " << traversed_vertices << " from " << ds.vertex_count() << " "
       << (float) traversed_vertices / (float) ds.vertex_count() * 100 << "%" << endl;
  double average = ((double) sum(run_times)) / (double) run_times.size() * 1000;
  cout << "BFS run in average in " << average << " milliseconds " << endl;
}

void Driver::load_base_dataset(TopologyInterface &ds, SortedCSRDataSource &base) {
  ds.bulkload(base);
}

void run_inserts(EdgeList &el, atomic_uint &insert_position, TopologyInterface &ds) {
  // Effects the batch size on performance have never been tested. I tested it only for the versioned data structure.
  // But it is likely that it applies for this case as well, in particular, since jobs here are smaller/take less time.
  const int batch_size = 3000;

  const int total_work = el.edges.size();
  while (insert_position.load() < total_work) {
    int work = insert_position.fetch_add(batch_size);
    int work_end = min(total_work, work + batch_size);

    while (work < work_end) {
      ds.insert_safe(el.edges[work]);
      work++;
    }
  }
}

void run_inserts_in_transactions(TransactionManager &tm, EdgeList &el, atomic_uint &insert_position,
                                 VersionedTopologyInterface *ds, uint total_partitions, uint partition) {
  auto thread_id = tm.register_thread();
  const int batch_size = 3000;

  const int total_work = el.edges.size();
  SnapshotTransaction tx = tm.getSnapshotTransaction(ds, thread_id);
  while (insert_position.load() < total_work) {
    int work = insert_position.fetch_add(batch_size);
    int work_end = min(total_work, work + batch_size);

    while (work < work_end) {
      tx.insert_edge(el.edges[work]);
      tx.execute();
      tm.transactionCompleted(tx, thread_id);
      tm.getSnapshotTransaction(ds, thread_id, tx);
      work++;
    }
  }
  tm.transactionCompleted(tx, thread_id);

//  cout << "total" << total_partitions << endl;
//  cout << "Partition: " << partition << endl;
//    SnapshotTransaction tx = tm.getSnapshotTransaction(ds, thread_id);
//  for (const auto& e : el) {
//    if (e.src % total_partitions == partition) {
//      tx.insert_edge(e);
//      tx.execute();
//      tm.transactionCompleted(tx, thread_id);
//      tm.getSnapshotTransaction(ds, thread_id, tx);
//    }
//  }
}

void Driver::run_insert_experiment(TransactionManager &tm, TopologyInterface &ds, EdgeList &el) {
  cout << "Running insert experiment inserting " << el.edges.size() << " edges." << endl;
  uint threads = config.insert_threads;

  auto start = chrono::steady_clock::now();
  if (threads == 1) {
    for (auto e : el.edges) {
      ds.insert_edge(e);
    }
  } else {
    atomic<uint> insert_index(0);
    vector<thread> ts;

    if (typeid(ds) == typeid(SnapshotTransaction)) {
      throw ConfigurationError("Cannot run batch insertions with transactions as off yet.");
    }

    for (int i = 0; i < threads; i++) {
      ts.emplace_back(run_inserts, ref(el), ref(insert_index), ref(ds));
    }

    for (auto &t : ts) {
      t.join();
    }
  }
  if (typeid(ds) == typeid(SnapshotTransaction)) {
    dynamic_cast<SnapshotTransaction &>(ds).execute();
    tm.transactionCompleted(dynamic_cast<SnapshotTransaction &>(ds), tm.register_thread());
  }
  auto end = chrono::steady_clock::now();

  size_t microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
  reporter.add_repetition(INSERT, 0, microseconds);

  cout << "Inserting took: " << microseconds / 1000 << " milliseconds " << endl;
  cout << "This is " << ((float) el.edges.size() / ((float) microseconds / 1000000.0)) << " edges per second" << endl;
#ifdef DEBUG
  check_insert(ds, el);
#endif
}

void Driver::run_insert_experiment_one_by_one(TransactionManager &tm, VersionedTopologyInterface *ds, EdgeList &el) {
  cout << "Running insert experiment inserting " << el.edges.size() << " edges." << endl;
  uint threads = config.insert_threads;

  auto thread_id = tm.register_thread();
  auto start = chrono::steady_clock::now();
  if (threads == 1) {
    SnapshotTransaction tx = tm.getSnapshotTransaction(ds, thread_id);
    for (auto e : el.edges) {
      tx.insert_edge(e);
      tx.execute();
      tm.transactionCompleted(tx, thread_id);
      tm.getSnapshotTransaction(ds, thread_id, tx);
    }
    tm.transactionCompleted(tx, thread_id);
  } else {
    atomic<uint> insert_index(0);
    vector<thread> ts;
    uint partition = 0;
    for (int i = 0; i < threads; i++) {
      ts.emplace_back(run_inserts_in_transactions, ref(tm), ref(el), ref(insert_index), ds, config.insert_threads,
                      partition);
      partition++;
    }

    for (auto &t : ts) {
      t.join();
    }
  }
  auto end = chrono::steady_clock::now();

  size_t microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
  reporter.add_repetition(INSERT, 0, microseconds);

  cout << "Inserting took: " << microseconds / 1000 << " milliseconds " << endl;
  cout << "This is " << ((float) el.edges.size() / ((float) microseconds / 1000000.0)) << " edges per second" << endl;
#ifdef DEBUG
  auto tx = tm.getSnapshotTransaction(ds, thread_id);
  check_insert(tx, el);
  tm.transactionCompleted(tx, thread_id);
#endif
  cout << "checked" << endl;
}


void Driver::run_delete_experiment(TopologyInterface &ds, EdgeList &el) {
  throw NotImplemented();
}

void Driver::run_triangle_counting_experiment(TopologyInterface &ds) {
  cout << "Running triangle experiment ";
  cout.flush();

  vector<size_t> run_times;
  size_t triangles;
  vector<dst_t> out;
  for (int rep = 0; rep < config.repetitions; rep++) {
    auto start = chrono::steady_clock::now();

    triangles = 0;

    if (typeid(ds) == typeid(HashSetAdjacencyLists)) {
      for (int a = 0; a < ds.vertex_count(); a++) {
        auto a_neighbours = (robin_hood::unordered_flat_set<dst_t> *) ds.raw_neighbourhood(a);

        for (auto b : *a_neighbours) {
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
    } else {
//#pragma omp parallel
//    {


      ContigiousBlockIterator &a_neighbours = getIter(ds);

//#pragma omp for reduction(+ : triangles) schedule(dynamic, 64)
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
    }
//    }
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

void Driver::run_neighbourhood_2_experiment(TopologyInterface &ds,
                                            const vector<vector<vertex_id_t>> &sources,
                                            bool run_raw_neighbourhood) {
  cout << "Running 2 neighbourhood experiment ";
  cout.flush();

  vector<size_t> run_times;

  for (int rep = 0; rep < config.repetitions; rep++) {
    auto start = chrono::steady_clock::now();
    unordered_map<vertex_id_t, size_t> neighbour_counts = Algorithms::neighbourhood_2(*this, ds, sources[rep],
                                                                                      run_raw_neighbourhood);
    auto end = chrono::steady_clock::now();

    size_t microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();
    run_times.push_back(microseconds);
    reporter.add_repetition(NEIGHBOUR_2, rep, microseconds);

    cout << ".";
    cout.flush();

    uint all_neighbours = 0;
    for (auto nc : neighbour_counts) {
      all_neighbours += nc.second;
    }

    cout << "Counted " << all_neighbours << endl;

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
  } else if (typeid(ds) == typeid(MallocAdjacencyLists) || typeid(ds) == typeid(VectorAdjacencyLists)
             || typeid(ds) == typeid(CSRMallocAdjacencyLists) || typeid(ds) == typeid(CSR)) {
    vectorIterators.push_back(VectorBatchedEdgeIterator());
    return vectorIterators[vectorIterators.size() - 1];
  } else {
    throw NotImplemented();
  }
}

EdgeIterator &Driver::getSingleEdgeIter(TopologyInterface &ds) {
  if (typeid(ds) == typeid(HashSetSimulatorAdjacencyList)) {
    filteredBlockIterators.push_back(FilteredVectorIterator());
    return filteredBlockIterators[filteredBlockIterators.size() - 1];
  } else {
    throw NotImplemented();
  }
}

void Driver::check_bfs(vertex_id_t start_vertex, vector<uint> &distances, version_t version) {
  cout << "Validating bfs experiment" << endl;
  string inserts = "base";

  if (version == FIRST_VERSION) {
    inserts = "base";
  } else if (version == 1) {
    inserts = "inserts";
  } else {
    inserts = to_string(version);
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

    uint e;
    for (auto d : distances) {
      f.read((char *) &e, sizeof(d));
      assert(d == e);
    }

    f.close();
  }
}

void Driver::validate_graph_structure(TopologyInterface &ds, SortedCSRDataSource &base, EdgeList &inserts,
                                      EdgeList &deletes) {
  cout << "Validating data structure." << endl;
  auto vertices = base.vertex_count();

  unordered_multimap<vertex_id_t, dst_t> insert_map;
  unordered_multimap<vertex_id_t, dst_t> delete_map;
  if (config.experiment_set.find(INSERT) != config.experiment_set.end()) {
    insert_map = inserts.to_map();
  }
  if (config.experiment_set.find(DELETE) != config.experiment_set.end()) {
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


unordered_set<dst_t> Driver::get_neighbours(TopologyInterface &ds, vertex_id_t v) {
  unordered_set<dst_t> neighbours;

  if (typeid(ds) == typeid(HashSetSimulatorAdjacencyList)) {
    auto &ns = getSingleEdgeIter(ds);
    ds.neighbourhood(v, ns);
    while (ns.has_next()) {
      neighbours.insert(ns.next());
    }
  } else {
    auto &ns = getIter(ds);
    ds.neighbourhood(v, ns);
    while (ns.has_next()) {
      auto block = ns.next();

      for (auto e : block) {
        neighbours.insert(e);
      }
    }
  }
  return neighbours;
}

void Driver::check_insert(TopologyInterface &ds, EdgeList &el) {
  cout << "Validating insert experiment" << endl;
  auto i = 0;
  for (auto e : el.edges) {
    i++;
//    if (i % 1000 == 0) {
//      cout << ".";
//    }
    assert(ds.has_edge(e));
  }


  const string gold_standard_file_sizes =
          config.gold_standard_directory + "/insert_adjacency_set_sizes_" + config.base.get_name() + ".goldStandard";
  if (!file_exists(gold_standard_file_sizes)) {
    cout << "Writing new gold standard for: " << gold_standard_file_sizes << endl;
    ofstream f(gold_standard_file_sizes, ofstream::binary | ofstream::out);

    if (!f.good()) {
      assert(false);
    }

    size_t size = ds.vertex_count();
    f.write((char *) &size, sizeof(size));

    for (auto v = 0; v < ds.vertex_count(); v++) {
      size_t neighbourhood_size = ds.neighbourhood_size(v);
      f.write((char *) &neighbourhood_size, sizeof(neighbourhood_size));
    }
    f.close();
  } else {
    ifstream f(gold_standard_file_sizes, ifstream::in | ifstream::binary);

    size_t size;
    f.read((char *) &size, sizeof(size));

    assert(size == ds.vertex_count());

    size_t neighbourhood_size;
    for (int i = 0; i < size; i++) {
      f.read((char *) &neighbourhood_size, sizeof(neighbourhood_size));
      assert(ds.neighbourhood_size(i) == neighbourhood_size);
    }

    f.close();
  }

  BFSSourceSelector ss(*this, config.base, ds);
  vertex_id_t start_vertex = ss.get_source();

  vector<uint> distances;
  if (typeid(ds) == typeid(SnapshotTransaction)) {
    distances = Algorithms::bfs(*this, ds, start_vertex, true, false);
  } else {
    distances = Algorithms::bfs(*this, ds, start_vertex);
  }
  check_bfs(start_vertex, distances, 1);
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

void Driver::run_community_detection(TopologyInterface &ds) {
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

void Driver::print_graph(TopologyInterface &ds) {
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

void Driver::run_page_rank_experiment(TopologyInterface &ds, bool run_on_raw_neighbourhood) {
  cout << "Running PR experiment ";
  cout.flush();

  vector<float> scores;

  vector<size_t> run_times;
  for (int rep = 0; rep < config.repetitions; rep++) {
    // BFS
    auto start = chrono::steady_clock::now();
    scores = Algorithms::page_rank(*this, ds, run_on_raw_neighbourhood);
    auto end = chrono::steady_clock::now();

    size_t microseconds = chrono::duration_cast<chrono::microseconds>(end - start).count();

    run_times.push_back(microseconds);
    reporter.add_repetition(PR, rep, microseconds);


    cout << ".";
    cout.flush();

#ifdef DEBUG
    check_page_rank(scores);
#endif
  }

  double average = ((double) sum(run_times)) / (double) run_times.size() * 1000;
  cout << endl << "PR run in average in " << average << " milliseconds " << endl;
}

void Driver::check_page_rank(vector<float> &scores) {
  cout << "Validating Page Rank experiment" << endl;
  string inserts = "base";

  const string gold_standard_file =
          config.gold_standard_directory + "/pr_" + config.base.get_name() + ".goldStandard";
  if (!file_exists(gold_standard_file)) {
    cout << "Writing new gold standard for: " << gold_standard_file << endl;
    ofstream f(gold_standard_file, ofstream::binary | ofstream::out);

    if (!f.good()) {
      assert(false);
    }

    size_t size = scores.size();
    f.write((char *) &size, sizeof(size));

    for (float s : scores) {
      f.write((char *) &s, sizeof(s));
    }
    f.close();
  } else {
    ifstream f(gold_standard_file, ifstream::in | ifstream::binary);

    size_t size;
    f.read((char *) &size, sizeof(size));
    assert(size == scores.size());

    float e;
    for (float d : scores) {
      f.read((char *) &e, sizeof(d));
      assert(fabs(d - e) < 1e-4);  // TODO move PR precission to Configuration
    }

    f.close();
  }
}

void Driver::show_storage_sizes(string ds_name, TopologyInterface &ds) {
  cout << "Storage size of " << setw(10) << ds_name << endl;
  ds.report_storage_size();
}

void Driver::run_gc_experiment(TransactionManager& tm, VersionedTopologyInterface& ds, bool inserts_run, EdgeList &inserts) {
  cout << "Running GC experiment " << endl;

  auto start = chrono::steady_clock::now();
  ds.gc_all();
  auto end = chrono::steady_clock::now();

  size_t milliseconds = chrono::duration_cast<chrono::milliseconds>(end - start).count();
  reporter.add_repetition(GC, 0, milliseconds);

  cout << ".";
  cout.flush();

#ifdef DEBUG
  if (inserts_run) {
    // TODO add function to return the thread ID.
    auto tx = tm.getSnapshotTransaction(&ds, tm.register_thread());
    check_insert(tx, inserts);
  }
  check_gc_experiment(ds);
#endif

  cout << endl << "GC run in " << milliseconds << " milliseconds " << endl;
}

void Driver::check_gc_experiment(VersionedTopologyInterface& ds) {
}

