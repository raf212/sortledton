//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_UTILS_H
#define LIVE_GRAPH_TWO_UTILS_H

#include <string>
#include <vector>

using namespace std;

bool file_exists (const string& name);
bool endsWith(const string& fullString, const string& ending);

template<typename n>
n sum(const vector<n>& v) {
  n s = 0;
  for (const auto& l : v) {
    s += l;
  }
  return s;
};

#endif //LIVE_GRAPH_TWO_UTILS_H
