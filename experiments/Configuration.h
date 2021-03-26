//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_CONFIGURATION_H
#define LIVE_GRAPH_TWO_CONFIGURATION_H

#include <unordered_set>
#include <utils/utils.h>
#include <vector>
#include <experimental/filesystem>
#include <boost/algorithm/string.hpp>

#include <utils/utils.h>

using namespace std;

class ConfigurationError : exception {
public:
    explicit ConfigurationError(string &&what) : w(what) {};

    const char *what() const noexcept override { return w.c_str(); }

private:
    string w;
};


enum SourceType {
    CSR_SRC,
    EDGE_LIST
};

enum DataStructures {
    CSR_DS,
    VECTOR_ADJACENCY_LIST,
    MALLOC_ADJACENCY_LIST,
    CSR_MALLOC_ADJACENCY_LIST,
    BLOCKED_LINKED_LIST_AL,
    BLOCKED_SKIP_LIST_AL,
    HASH_SET_SIMULATOR_AL,
    HASH_SET_AL,
    VERSIONED
};

enum Experiments {
    INSERT,   // Inserts edges one-by-one but uses only one transaction in the versioned case.
    INSERT_TRANSACTIONS,  // Inserts edges one-by-one using one transaction per edge.
    DELETE,
    TRIANGLE_COUNTING,
    BFS,
    PR,
    NEIGHBOUR_2,
    STORAGE,
    GC,
    GAPBS_BFS,
    GAPBS_PR,
    SSSP,
    WCC,
    LCC,
    CDLP
};

class Dataset {
public:
    Dataset() {};

    Dataset(const string &path, const SourceType expected_type) : path(path) {
      if (!file_exists(path)) {
        throw ConfigurationError("Path " + path + " does not exists.");
      }
      if (expected_type == CSR_SRC && !endsWith(path, "csr")) {
        throw ConfigurationError("Expected dataset " + path + " to be a CSR.");
      }
      if (expected_type == EDGE_LIST && !endsWith(path, "edgeList")) {
        throw ConfigurationError("Expected dataset " + path + " to be a edge list.");
      }

      if (expected_type == CSR_SRC) {
        name = path;
      } else {
        name = "insert or delete dataset";
      }
    };
    string path;
    string name;

    string get_name() {
      vector<string> strs;
      boost::split(strs,name,boost::is_any_of("/"));
      return strs[strs.size() - 2];
    }

    bool is_undirected() {
      return endsWith(get_name(), "-u");
    }
};

class Config {
public:
    const static string gold_standard_directory;

    const static unordered_map<DataStructures, string> DATA_STRUCTURE_MAPPING;
    const static unordered_map<Experiments, string> EXPERIMENT_MAPPING;

    vector<pair<DataStructures, vector<string>>> data_structures;
    vector<pair<Experiments, vector<string>>> experiments;
    unordered_set<Experiments> experiment_set;

    Dataset base;
    Dataset insertions;
    Dataset deletions;

    uint repetitions;

    constexpr static double PAGE_RANK_ERROR = 1e-4;
    constexpr static int PAGE_RANK_ITERATIONS = 5;
    constexpr static double PAGE_RANK_DAMPING_FACTOR = 0.85;
    constexpr static int CDLP_MAX_ITERATIONS = 30;


    // TODO remove it turned out to be not beneficial and is not used
    /**
     * Configures the BlockedBatchedEdgeIterator to prefetch <prefetch_blocks> ahead.
     */
    uint prefetch_blocks = 0;

    bool release = false;

    /**
     * Uses undirected insert mode, meaning each edge is inserted in both directions and the graph is validated as
     * an undirected graph.
     *
     * This requires the input dataset to match, in other words, each edge in the edge stream should exists only in one
     * direction. CSR like base datasets are not supported yet.
     */
    bool undirected = false;

    bool validate_datastructures = false;

    uint insert_threads = 1;

    uint omp_threads = 0;

    bool weighted = false;

    void initialize(int argc, char **argv);

private:
    vector<string> string_split(char seperator, string list);

    vector<pair<DataStructures, vector<string>>> parse_data_structures(string arg);

    vector<pair<Experiments, vector<string>>> parse_experiments(string arg);
};

#endif //LIVE_GRAPH_TWO_CONFIGURATION_H
