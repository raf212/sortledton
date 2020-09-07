//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_CONFIGURATION_H
#define LIVE_GRAPH_TWO_CONFIGURATION_H

#include <unordered_set>
#include <utils/utils.h>
#include <vector>
#include <experimental/filesystem>

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
    VECTOR_ADJACENCY_LIST
};

enum Experiments {
    INSERT,
    DELETE,
    TRIANGLE_COUNTING,
    BFS,
    NEIGHBOUR_2
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
      if (expected_type == EDGE_LIST && !endsWith(path, "el")) {
        throw ConfigurationError("Expected dataset " + path + " to be a CSR.");
      }

      if (expected_type == CSR_SRC) {
        name = path;
      } else {
        name = "insert or delete dataset";
      }
    };
    string path;
    string name;
};

class Config {
public:
    const static unordered_map<DataStructures, string> DATA_STRUCTURE_MAPPING;
    const static unordered_map<Experiments, string> EXPERIMENT_MAPPING;

    unordered_set<DataStructures> data_structures;
    unordered_set<Experiments> experiments;

    Dataset base;
    Dataset insertions;
    Dataset deletions;

    uint repetitions;

    void initialize(int argc, char **argv);

private:
    vector<string> parse_comma_separated_list(string list);

    unordered_set<DataStructures> parse_data_structures(string arg);

    unordered_set<Experiments> parse_experiments(string arg);

};

#endif //LIVE_GRAPH_TWO_CONFIGURATION_H
