//
// Created by per on 09.10.20.
//

#include "BFSSourceSelector.h"

#include "Algorithms.h"


const string BFSSourceSelector::SOURCE_FOLDER =  "/home/fuchs/graph-two-bfs-sources/";

vertex_id_t BFSSourceSelector::get_source() {
  const string file_path = SOURCE_FOLDER + dataset.get_name() + ".source";
  if (!file_exists(file_path)) {
    vertex_id_t source = find_source();

    string s_source = to_string(source);

    ofstream f(file_path);
    f << s_source << endl;
    f.close();
  } else {
    ifstream f(file_path);

    string s_source;
    getline(f, s_source);

    f.close();

    return stoi(s_source);
  }
}

vertex_id_t BFSSourceSelector::find_source() {
  cout << "Finding source for bfs in graph " << dataset.get_name() << endl;
  uint target_vertices = ds.vertex_count() * TARGET_PERCENTAGE;

  vector<uint> distances;
  for (vertex_id_t v = 0; v < ds.vertex_count(); v++) {
    distances = Algorithms::bfs(driver, ds, v);

    cout << "Traversed " << Algorithms::traversed_vertices(ds, distances) << endl;
    cout << "Percentage " << (float) Algorithms::traversed_vertices(ds, distances) / (float) ds.vertex_count() << endl;
    if (target_vertices < Algorithms::traversed_vertices(ds, distances)) {
      cout << "Found source " << v << endl;
      return v;
    }
  }
}
