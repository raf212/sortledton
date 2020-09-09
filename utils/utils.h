//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_UTILS_H
#define LIVE_GRAPH_TWO_UTILS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <data_types.h>

using namespace std;

bool file_exists (const string& name);
bool endsWith(const string& fullString, const string& ending);

string string_join(const string& join, const vector<string>& list);

template<typename n>
n sum(const vector<n>& v) {
  n s = 0;
  for (const auto& l : v) {
    s += l;
  }
  return s;
};


template<typename k, typename v>
unordered_map<v, k> reverse_map(unordered_map<k, v> mapping) {
  unordered_map<v, k> out;
  for (const auto& kv : mapping) {
    out.insert({kv.second, kv.first});
  }
  return out;
}

string get_filename(string path);

string get_home_dir();

void intersect_edge_block(dst_t* start_a, dst_t* end_a, dst_t* start_b, dst_t* end_b, vector<dst_t>& out);

#endif //LIVE_GRAPH_TWO_UTILS_H
