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
    HASH_SET_SIMULATOR_AL
};

enum Experiments {
    INSERT,
    DELETE,
    TRIANGLE_COUNTING,
    BFS,
    NEIGHBOUR_2,
    COMMUNITY_DETECTION
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
    unordered_set<Experiments> experiments;

    Dataset base;
    Dataset insertions;
    Dataset deletions;

    uint repetitions;

    /**
     * Configures the BlockedBatchedEdgeIterator to prefetch <prefetch_blocks> ahead.
     */
    uint prefetch_blocks = 0;

    bool release = false;

    bool validate_datastructures = false;

    void initialize(int argc, char **argv);

private:
    vector<string> string_split(char seperator, string list);

    vector<pair<DataStructures, vector<string>>> parse_data_structures(string arg);

    unordered_set<Experiments> parse_experiments(string arg);

};

#endif //LIVE_GRAPH_TWO_CONFIGURATION_H
