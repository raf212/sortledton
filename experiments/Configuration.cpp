//
// Created by per on 31.08.20.
//

#include <getopt.h>
#include "Configuration.h"

void Configuration::initialize(int argc, char **argv) {
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
            {"repetitions", required_argument, 0, 'r'}
    };

    c = getopt_long(argc, argv, "",
                    long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 'e':
        singleton.experiments = parse_experiments(optarg);
        break;
      case 's':
        singleton.data_structures = parse_data_structures(optarg);
        break;
      case 'b':
        singleton.base = Dataset(optarg, CSR_SRC);
        break;
      case 'i':
        singleton.insertions = Dataset(optarg, EDGE_LIST);
        break;
      case 'd':
        singleton.deletions = Dataset(optarg, EDGE_LIST);
        break;
      case 'r':
        singleton.repetitions = stoi(optarg);
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

vector <string> Configuration::parse_comma_separated_list(string list) {
  char delim = ',';
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

unordered_set<DataStructures> Configuration::parse_data_structures(string arg) {
  unordered_set<DataStructures> ret;
  auto ds = parse_comma_separated_list(arg);

  for (const auto& d : ds) {
    if (d == "csr") {
      ret.insert(CSR_DS);
    } else if (d == "vector_al") {
      ret.insert(VECTOR_ADJACENCY_LIST);
    } else {
      throw ConfigurationError("Unknown data structure " + d);
    }
  }

  return ret;
}

unordered_set<Experiments> Configuration::parse_experiments(string arg) {
  unordered_set<Experiments> ret;
  auto es = parse_comma_separated_list(arg);

  for (const auto& e : es) {
    if (e == "insert") {
      ret.insert(INSERT);
    } else if (e == "delete") {
      ret.insert(DELETE);
    } else if (e == "bfs") {
      ret.insert(BFS);
    } else if (e == "triangle_counting") {
      ret.insert(TRIANGLE_COUNTING);
    } else if (e == "2-neighbour"){
      ret.insert(NEIGHBOUR_2);
    } else {
      throw ConfigurationError("Unknown experiment " + e);
    }
  }

  return ret;
}
