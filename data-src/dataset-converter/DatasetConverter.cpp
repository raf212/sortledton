//
// Created by per on 01.09.20.
//

#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <unordered_map>
#include <limits>
#include <algorithm>
#include <unordered_set>

#include "DatasetConverter.h"
#include <data_types.h>
#include <data-src/SortedCSRDataSource.h>
#include "Options.h"

DatasetConverter::DatasetConverter(int argc, char **argv) {
  o = parseOptions(argc, argv);
}


void show_max_degree(const vector<temporal_edge_t>& edge_list, size_t vertex_count) {
  unordered_map<vertex_id_t, size_t> counter;
  counter.reserve(vertex_count);

  size_t m = 0;
  for (auto e : edge_list) {
    auto c = counter.find(e.src);
    if (c != counter.end()) {
      c->second++;
      m = std::max(m, c->second);
    } else {
      counter.insert({e.src, 1});
    }
  }

  cout << "Max degree: " << m << endl;
}

void DatasetConverter::run() {
  cout << "Parsing text file. Densify: " << o.densify << " Temporal value: " << o.temporal_value_position << endl;
  size_t vertex_count;
  vector<temporal_edge_t> edge_list = parse_text_file(o, vertex_count);

  if (o.make_undirected) {
    edge_list = make_undirected(edge_list);
  }

  edge_list = clean_data(edge_list, o.make_directed);

  size_t deletion_set_size = o.deletion_percentage * edge_list.size();
  size_t insertion_set_size = o.insert_percentage * edge_list.size();

  if (o.make_undirected) {
    SortedCSRDataSource csr = convert_to_sorted_csr(edge_list.begin(), edge_list.end(), vertex_count);
    write_base_dataset(csr);

    write_degree_information(csr);
  } else if (o.input_format == EDGELIST_TEXT) {
    cout << "Creating " << insertion_set_size << " updates and " << deletion_set_size << " deletions." << endl;
    cout << "Running as non temporal file" << endl;

    shuffle(edge_list.begin(), edge_list.end(), std::mt19937(std::random_device()()));

    write_insertion_set(edge_list.end() - insertion_set_size, edge_list.end());

    write_deletion_set(edge_list.begin(), edge_list.begin() + deletion_set_size);

    SortedCSRDataSource csr = convert_to_sorted_csr(edge_list.begin(), edge_list.end() - insertion_set_size,
                                                    vertex_count);
    write_base_dataset(csr);

    write_degree_information(csr);
  } else if (o.input_format == TEMPORAL_EDGELIST_TEXT) {
    cout << "Creating " << insertion_set_size << " updates and " << deletion_set_size << " deletions." << endl;
    cout << "Running as a temporal file." << endl;
    sort(edge_list.begin(), edge_list.end(),
         [](temporal_edge_t a, temporal_edge_t b) { return a.creation_timestamp < b.creation_timestamp; });


    auto insertions_begin = edge_list.end() - insertion_set_size;
    write_insertion_set(insertions_begin, edge_list.end());

    shuffle(edge_list.begin(), insertions_begin, std::mt19937(std::random_device()()));

    write_deletion_set(edge_list.begin(), edge_list.begin() + deletion_set_size);

    SortedCSRDataSource csr = convert_to_sorted_csr(edge_list.begin(), insertions_begin, vertex_count);

    write_base_dataset(csr);

    write_degree_information(csr);
  }

  cout << "End" << endl << endl;
}

bool isComment(const string &line) {
  char commentChars[] = {'#', '%'};

  for (char c : commentChars) {
    if (line.find(c) == 0) {
      return true;
    }
  }
  return false;
}

char DatasetConverter::detect_seperator(const string &input) {
  ifstream in;
  in.exceptions(fstream::failbit | fstream::badbit);

  in.open(input);

  const char seperators[] = {',', ' ', '\t'};

  string line;
  while (getline(in, line)) {
    if (isComment(line)) {
      continue;
    } else {
      for (char s : seperators) {
        if (line.find(s) != string::npos) {
          return s;
        }
      }
      cout << "Couldn't detect seperator in line: " << line << endl;
      exit(Options::BAD_FORMAT);
    }
  }
}


temporal_edge_t parse_edge(const string &line, const char seperator, unordered_set<size_t> &vertex_set) {
  auto first_seperator_index = line.find(seperator);
  auto second_seperator_index = line.find(seperator, first_seperator_index + 1);

  string src_string = line.substr(0, first_seperator_index);
  string dst_string = line.substr(first_seperator_index + 1, second_seperator_index - first_seperator_index - 1);

  vertex_id_t src = stoi(src_string);
  dst_t dst = stoi(dst_string);

  vertex_set.insert(src);
  vertex_set.insert(dst);
  return temporal_edge_t{src, dst, 0};
}


temporal_edge_t
parse_edge_densifying(const string &line, const char seperator, unordered_map<string, size_t> &densifyer,
                      size_t &next_vertex_id) {
  auto first_seperator_index = line.find(seperator);
  auto second_seperator_index = line.find(seperator, first_seperator_index + 1);

  string src_string = line.substr(0, first_seperator_index);
  string dst_string = line.substr(first_seperator_index + 1, second_seperator_index - first_seperator_index - 1);
  vertex_id_t src;
  dst_t dst;

  auto t = densifyer.find(src_string);
  if (t == densifyer.end()) {
    densifyer.insert(make_pair(src_string, next_vertex_id));
    src = next_vertex_id;
    next_vertex_id++;
  } else {
    src = t->second;
  }

  t = densifyer.find(dst_string);
  if (t == densifyer.end()) {
    densifyer.insert(make_pair(dst_string, next_vertex_id));
    dst = next_vertex_id;
    next_vertex_id++;
  } else {
    dst = t->second;
  }

  return temporal_edge_t{src, dst, 0};
}

temporal_edge_t parse_temporal_edge(const string &line, const char seperator, size_t temporal_value_position,
                                    bool densify, unordered_map<string, size_t> &densifyer, size_t &next_vertex_id,
                                    unordered_set<size_t> &vertex_set) {
  temporal_edge_t edge{};
  if (densify) {
    edge = parse_edge_densifying(line, seperator, densifyer, next_vertex_id);
  } else {
    edge = parse_edge(line, seperator, vertex_set);
  }

  if (temporal_value_position != numeric_limits<size_t>::max()) {
    size_t tempIndex = -1;
    for (int i = 0; i < temporal_value_position; i++) {
      tempIndex = line.find(seperator, tempIndex + 1);
    }
    size_t afterTempIndex = line.find(seperator, tempIndex + 1);
    edge.creation_timestamp = stoi(line.substr(tempIndex + 1, afterTempIndex - tempIndex - 1));
  }

  return edge;
}

vector<temporal_edge_t> DatasetConverter::parse_text_file(Options o, size_t &vertex_count) {
  cout << "Parsing file " << o.input_path << " with densify " << o.densify << endl;

  vector<temporal_edge_t> out;

  char seperator = detect_seperator(o.input_path);

  ifstream in{o.input_path};

  // Map for densifying
  unordered_map<string, size_t> translation;
  unordered_set<size_t> vertex_set;
  size_t next_vertex_id = 0;

  string line;
  while (getline(in, line)) {
    if (isComment(line)) {
      continue;
    }

    temporal_edge_t e = parse_temporal_edge(line, seperator, o.temporal_value_position, o.densify,
                                            translation, next_vertex_id, vertex_set);
    out.push_back(e);
  }

  if (o.densify) {
    vertex_count = next_vertex_id;
  } else {
    vertex_count = vertex_set.size();
  }

  in.close();
  return out;
}

void
write_edle_list(const string &path, vector<temporal_edge_t>::iterator begin, vector<temporal_edge_t>::iterator end) {
  ofstream f{path, ofstream::out | ofstream::binary | ofstream::trunc};

  size_t count = end - begin;
  f.write((char *) &count, sizeof(count));

  auto pos = begin;
  while (pos < end) {
    edge_t e{pos->src, pos->dst};
    f.write((char *) &e, sizeof(e));
    pos++;
  }

  f.close();
}

void DatasetConverter::write_insertion_set(
        vector<temporal_edge_t>::iterator begin, vector<temporal_edge_t>::iterator end) {
  cout << "Writing insertions to " << o.output_path + o.insertion_file_name << endl;
  write_edle_list(o.output_path + o.insertion_file_name, begin, end);
}

void
DatasetConverter::write_deletion_set(vector<temporal_edge_t>::iterator begin, vector<temporal_edge_t>::iterator end) {
  cout << "Writing deletions to " << o.output_path + o.deletion_file_name << endl;
  write_edle_list(o.output_path + o.deletion_file_name, begin, end);
}

SortedCSRDataSource DatasetConverter::convert_to_sorted_csr(vector<temporal_edge_t>::iterator begin,
                                                            vector<temporal_edge_t>::iterator end,
                                                            size_t vertex_count) {
  cout << "Creating CSR." << endl;
  sort(begin, end, [](temporal_edge_t a, temporal_edge_t b) { return a.src == b.src ? a.dst < b.dst : a.src < b.src; });

  SortedCSRDataSource out;
  out.adjacency_index.resize(vertex_count + 1);

  out.adjacency_lists.reserve(end - begin);

  vertex_id_t current_src = 0;

  while (current_src < begin->src) {
    out.adjacency_index[current_src] = 0;
    current_src++;
  }
  out.adjacency_index[current_src] = 0;

  auto pos = begin;
  while (pos < end) {
    if (current_src != pos->src) {
      while (current_src < pos->src) {
        out.adjacency_index[current_src + 1] = (pos - begin);
        current_src++;
      }
    }
    out.adjacency_lists.push_back(pos->dst);
    pos++;
  }

  while (current_src < vertex_count) {
    out.adjacency_index[current_src + 1] = (pos - begin);
    current_src++;
  }

  return out;
}

void DatasetConverter::write_base_dataset(SortedCSRDataSource csr) {
  cout << "Writing CSR to " << o.output_path + o.base_file_name << endl;
  ofstream f{o.output_path + o.base_file_name, ofstream::out | ofstream::binary};

  SortedCSRDataSource::FileHeader header;
  header.vertex_count = csr.vertex_count();
  header.edge_count = csr.adjacency_lists.size();

  f.write((char *) &header, sizeof(header));
  f.write((char *) &csr.adjacency_index[0], sizeof(size_t) * (header.vertex_count + 1));
  f.write((char *) &csr.adjacency_lists[0], sizeof(dst_t) * (header.edge_count));

  f.close();

  SortedCSRDataSource rr;
  const string s = o.output_path + o.base_file_name;
  rr.read_from_binary_file(s);
}

vector<temporal_edge_t> DatasetConverter::make_undirected(vector<temporal_edge_t> &edges) {
  cout << "Creating undirected dataset" << endl;
  vector<temporal_edge_t> undirected;
  undirected.reserve(edges.size() * 2);

  for (auto e : edges) {
    undirected.push_back({e.src, e.dst, e.creation_timestamp});
    undirected.push_back({e.dst, e.src, e.creation_timestamp});
  }

  return undirected;
}


vector<temporal_edge_t> DatasetConverter::clean_data(vector<temporal_edge_t> &edges, bool make_directed) {
  cout << "Cleaning data" << endl;
  vector<temporal_edge_t> clean;
  clean.reserve(edges.size());

  unordered_set<temporal_edge_t, TemporalEdgeHash, TemporalEdgeEqual> dedup;

  for (auto e : edges) {
    if (make_directed && e.dst < e.src) {
      swap(e.src, e.dst);
    }
    if (e.src != e.dst && dedup.find(e) == dedup.end()) {
      clean.push_back(e);
      dedup.insert(e);
    }
  }

  return clean;
}

void DatasetConverter::write_degree_information(SortedCSRDataSource &graph) {
  string file_path = o.output_path + o.base_file_name + ".degrees";
  cout << "Writing degree information to " << file_path << endl;
  ofstream o(file_path, ofstream::out | ofstream::binary);

  size_t vc = graph.vertex_count();
  o.write((char *) &vc, sizeof(vc));

  size_t m = 0;
  for (vertex_id_t v = 0; v < graph.vertex_count(); v++) {
    size_t d = graph.adjacency_index[v + 1] - graph.adjacency_index[v];
    o.write((char *) &v, sizeof(v));
    o.write((char *) &d, sizeof(d));
    m = max(m, d);
  }

  cout << "max degree according to csr: " << m << endl;

  o.close();
}
