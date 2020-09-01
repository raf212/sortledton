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

#include "DatasetConverter.h"
#include <data_types.h>
#include <data-src/SortedCSRDataSource.h>
#include "Options.h"

DatasetConverter::DatasetConverter(int argc, char **argv) {
  o = parseOptions(argc, argv);
}

void DatasetConverter::run() {
  vector<temporal_edge_t> edge_list = parse_text_file(o);

  size_t deletion_set_size = o.deletion_percentage * edge_list.size();
  size_t insertion_set_size = o.insert_percentage * edge_list.size();

  if (o.input_format == EDGELIST_TEXT) {
    shuffle(edge_list.begin(), edge_list.end(), std::mt19937(std::random_device()()));

    write_insertion_set(edge_list.end() - insertion_set_size, edge_list.end());
    write_deletion_set(edge_list.begin(), edge_list.begin() + deletion_set_size);

    SortedCSRDataSource csr = convert_to_sorted_csr(edge_list.begin(), edge_list.end() - insertion_set_size);

    write_base_dataset(csr);

  } else if (o.input_format == TEMPORAL_EDGELIST_TEXT) {
    sort(edge_list.begin(), edge_list.end(), [](temporal_edge_t a, temporal_edge_t b) -> { return a.creation_timestamp < b.creation_timestamp; });

    auto insertions_begin = edge_list.end() - insertion_set_size;
    write_insertion_set(insertions_begin, edge_list.end());

    shuffle(edge_list.begin(), insertions_begin, std::mt19937(std::random_device()()));
    write_deletion_set(edge_list.begin(), edge_list.begin() + deletion_set_size);

    SortedCSRDataSource csr = convert_to_sorted_csr(edge_list.begin(), insertions_begin);

    write_base_dataset(csr);
  }
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

char DatasetConverter::detect_seperator(const string & input) {
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


temporal_edge_t parse_edge(const string &line, const char seperator) {
  auto first_seperator_index = line.find(seperator);
  auto second_seperator_index = line.find(seperator, first_seperator_index + 1);

  string src_string = line.substr(0, first_seperator_index);
  string dst_string = line.substr(first_seperator_index + 1, second_seperator_index - first_seperator_index - 1);

  vertex_id_t src = stoi(src_string);
  dst_t dst = stoi(dst_string);
  return temporal_edge_t { src, dst, 0 };
}


temporal_edge_t parse_edge_densifying(const string &line, const char seperator, unordered_map<string, size_t>& densifyer, size_t& next_vertex_id) {
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

  return temporal_edge_t { src, dst, 0};
}

temporal_edge_t parse_temporal_edge(const string &line, const char seperator, size_t temporal_value_position, bool densify, unordered_map<string, size_t>& densifyer, size_t& next_vertex_id) {
  temporal_edge_t edge {};
  if (densify) {
    edge = parse_edge_densifying(line, seperator, densifyer, next_vertex_id);
  } else{
    edge = parse_edge(line, seperator);
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

vector<temporal_edge_t> DatasetConverter::parse_text_file(Options o) {
  cout << "Parsing file " << o.input_path << " with densify " << o.densify << endl;

  vector<temporal_edge_t> out;

  char seperator = detect_seperator(o.input_path);

  ifstream in {o.input_path};

  // Map for densifying
  unordered_map<string, size_t> translation;
  size_t next_vertex_id = 0;

  string line;
  while (getline(in, line)) {
    if (isComment(line)) {
      continue;
    }

    temporal_edge_t e = parse_temporal_edge(line, seperator, o.temporal_value_position, o.densify, translation, next_vertex_id);
    out.push_back(e);
  }

  in.close();
  return out;


  return out;
}

void write_edle_list(const string& path, vector<temporal_edge_t>::iterator begin, vector<temporal_edge_t>::iterator end) {
  ofstream f {path, ofstream::out | ofstream::binary | ofstream::trunc };

  size_t count = end - begin;
  f.write((char*) &count, sizeof(count));

  auto pos = begin;
  while (pos < end) {
    edge_t e { pos->src, pos->dst };
    f.write((char*) &e, sizeof(e));
    pos++;
  }

  f.close();
}

void DatasetConverter::write_insertion_set(
        vector<temporal_edge_t>::iterator begin, vector<temporal_edge_t>::iterator end) {
  write_edle_list(o.output_path + o.insertion_file_name, begin, end);
}

void DatasetConverter::write_deletion_set(vector<temporal_edge_t>::iterator begin, vector<temporal_edge_t>::iterator end) {
  write_edle_list(o.output_path + o.deletion_file_name, begin, end);
}

SortedCSRDataSource DatasetConverter::convert_to_sorted_csr(vector<temporal_edge_t>::iterator begin,
                                                            vector<temporal_edge_t>::iterator end) {
  sort(begin, end, [] (temporal_edge_t a, temporal_edge_t b) -> { return a.src == b.src ?  a.dst < b.dst : a.src < b.src; });

  SortedCSRDataSource out;
  out.adjacency_index.push_back(0);
  out.adjacency_lists.reserve(end - begin);

  vertex_id_t current_src = begin->src;
  auto pos = begin;
  while (pos < end) {
    if (current_src != pos->src) {
      out.adjacency_index.push_back(pos - begin - 1);
      current_src = pos->src;
    }
    out.adjacency_lists.push_back(pos->dst);
  }

  return out;
}

void DatasetConverter::write_base_dataset(SortedCSRDataSource csr) {
  ofstream f {o.output_path + o.base_file_name, ofstream::out | ofstream::binary };

  SortedCSRDataSource::FileHeader header;
  header.vertex_count = csr.adjacency_index.size() - 1;
  header.edge_count = csr.adjacency_lists.size();

  SortedCSRDataSource::FileBody body;
  body.offsets = csr.adjacency_index.data();
  body.adjacency_lists = csr.adjacency_lists.data();

  f.write((char*) &header, sizeof(header));
  f.write((char*) &body.offsets, sizeof(size_t) * sizeof(header.vertex_count + 1));
  f.write((char*) &body.adjacency_lists, sizeof(dst_t) * sizeof(header.edge_count));

  f.close();
}




