//
// Created by per on 31.08.20.
//

#include "Configuration.h"

#include <getopt.h>

#include "utils/utils.h"

void Config::initialize(int argc, char **argv) {
  int c;
  int digit_optind = 0;

  while (1) {
    int this_option_optind = optind ? optind : 1;
    int option_index = 0;
    static struct option long_options[] = {
            {"experiments", required_argument, 0, 'e'},
            {"data_structures", required_argument, 0, 's'},
            {"dataset_base", required_argument, 0, 'b'},
            {"dataset_insert", required_argument, 0, 'i'},
            {"dataset_delete", required_argument, 0, 'd'},
            {"validate", no_argument, 0, 'v'},
            {"repetitions", required_argument, 0, 'r'},
            {"release_run", no_argument, 0, 'l'},
            {"prefetch_blocks", required_argument, 0, 'p'},
            {"insert_threads", required_argument, 0, 't'}

    };

    c = getopt_long(argc, argv, "",
                    long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 't':
        insert_threads = stoi(optarg);
        break;
      case 'p':
        prefetch_blocks = stoi(optarg);
        break;
      case 'l':
        release = true;
        break;
      case 'e':
        experiments = parse_experiments(optarg);
        for (auto e : experiments) {
          experiment_set.insert(e.first);
        }
        if (experiment_set.find(INSERT) != experiment_set.end() && experiment_set.find(INSERT_TRANSACTIONS) != experiment_set.end()) {
          throw ConfigurationError("Cannot run INSERT and INSERT_TRANSACTION in one go.");
        }
        break;
      case 's':
        data_structures = parse_data_structures(optarg);
        break;
      case 'b':
        base = Dataset(optarg, CSR_SRC);
        break;
      case 'i':
        insertions = Dataset(optarg, EDGE_LIST);
        break;
      case 'd':
        deletions = Dataset(optarg, EDGE_LIST);
        break;
      case 'r':
        repetitions = stoi(optarg);
        break;
      case 'v':
        validate_datastructures = true;
        break;
      case '?':
        printf("No help provided read src.\n");
        break;
      default:
        printf("?? getopt returned character code 0%o ??\n", c);
    }
  }

  if (optind < argc) {
    throw ConfigurationError("Unknown positional argument.");
  }
}

vector <string> Config::string_split(char seperator, string list) {
  char delim = seperator;
  std::size_t current, previous = 0;
  vector<string> cont;
  current = list.find(delim);
  while (current != std::string::npos) {
    cont.push_back(list.substr(previous, current - previous));
    previous = current + 1;
    current = list.find(delim, previous);
  }
  cont.push_back(list.substr(previous, current - previous));
  return cont;
}

vector<pair<DataStructures, vector<string>>> Config::parse_data_structures(string arg) {
  vector<pair<DataStructures, vector<string>>> ret;
  auto ds = string_split(',', arg);

  auto ds_map = reverse_map(DATA_STRUCTURE_MAPPING);
  for (const auto& d : ds) {
    auto data_structure_name = d;

    vector<string> parameters;
    auto parameters_start = d.find('(');

    if (parameters_start != string::npos) {
        data_structure_name = d.substr(0, parameters_start);
        parameters = string_split('\'', d.substr(parameters_start + 1, d.find(')') - (parameters_start + 1)));
    }

    auto mapping = ds_map.find(data_structure_name);
    if (mapping == ds_map.end()) {
      throw ConfigurationError("Unknown data structure " + d);
    } else {
      ret.emplace_back(mapping->second, parameters);
    }
  }

  return ret;
}

vector<pair<Experiments, vector<string>>> Config::parse_experiments(string arg) {
  vector<pair<Experiments, vector<string>>> ret;
  auto es = string_split(',', arg);

  auto map = reverse_map(EXPERIMENT_MAPPING);
  for (const auto& e : es) {
    vector<string> parameters;
    auto experiment_name = e;
    auto parameters_start = e.find('(');

    if (parameters_start != string::npos) {
      experiment_name = e.substr(0, parameters_start);
      parameters = string_split('\'', e.substr(parameters_start + 1, e.find(')') - (parameters_start + 1)));
    }

    auto mapping = map.find(experiment_name);
    if (mapping == map.end()) {
      throw ConfigurationError("Unknown experiment " + e);
    } else {
      ret.emplace_back(mapping->second, parameters);
    }
  }

  return ret;
}

const unordered_map<DataStructures, string> Config::DATA_STRUCTURE_MAPPING {
        {CSR_DS, "csr"},
        {VECTOR_ADJACENCY_LIST, "vectorAL"},
        {MALLOC_ADJACENCY_LIST, "mallocAL"},
        {CSR_MALLOC_ADJACENCY_LIST, "csrMallocAL"},
        {BLOCKED_LINKED_LIST_AL, "bllAL"},
        {BLOCKED_SKIP_LIST_AL, "bslAL"},
        {HASH_SET_SIMULATOR_AL, "hsAL"},
        {HASH_SET_AL, "rhAL"},
        {VERSIONED, "v"}
};

const unordered_map<Experiments, string> Config::EXPERIMENT_MAPPING{
        {INSERT, "insert"},
        {INSERT_TRANSACTIONS, "insert_tx"},
        {DELETE, "delete"},
        {BFS,    "bfs"},
        {PR, "pr"},
        {TRIANGLE_COUNTING, "triangle"},
        {NEIGHBOUR_2, "2-neighbour"},
        {COMMUNITY_DETECTION, "community"},
        {STORAGE, "storage"}
};

const string Config::gold_standard_directory = "/space/fuchs/shared/graph_two_gold_standards";
