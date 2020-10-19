#include <string>
#include <data-src/SortedCSRDataSource.h>
#include <experiments/Configuration.h>
#include <fstream>
#include <iostream>


using namespace std;

float get_density(dst_t min, dst_t max, size_t size) {
  return (float) size / (float) (max - min);
}

uint find_dense_regions(SortedCSRDataSource& src, vertex_id_t v) {
  uint dense_regions = 0;
  float one_16th = 1.0 / 16.0;
  dst_t* start = &src.adjacency_lists[src.adjacency_index[v]];
  for (int i = 0; i + 128 < src.neighbourhood_size(v); i++) {
    if (one_16th < get_density(*(start + i), *(start + i + 127), 128)) {
      dense_regions += 1;
      i += 128;
    }
  }
  return dense_regions;
}

int main(int argc, char **argv) {
  string OUTPUT_PATH = "/home/fuchs/bitset-analysis/";

  cout << argv[1] << endl;

  string dataset_path = string(argv[1]);

  Dataset dataset(dataset_path, CSR_SRC);

  SortedCSRDataSource src;
  src.read_from_binary_file(dataset_path);


  // We define density as (range(ns)/size(ns)).
  // A single destination is stored in 32 bits. Hence, a density of above 1/32 saves storage and potentially execution speed.

  // Neighbourhood which are denser than 1/32th
  float one_32th = 1.0 / 32.0;
  uint dense_neighbourhoods = 0;
  uint dense_size_avg = 0;
  // Neighbourhoods which are denser than 1/16th - they can be stored in half the space in a bitset.
  float one_16th = 1.0 / 16.0;
  uint double_dense_neighbourhoods = 0;
  uint double_dense_size_avg = 0;

  uint dense_regions = 0;

  // Neighbourhood below 16 neighbours. 16 neighbours fit into a single cache line, therefore it's unlikely that compressing them will help execution speed.
  uint small_neighbourhoods = 0;

  for (vertex_id_t v = 0; v < src.vertex_count(); v++) {
    if (src.neighbourhood_size(v) <= 16) {
      small_neighbourhoods++;
    } else {
      float density = get_density(src.get_min_neighbour(v), src.get_max_neighbour(v), src.neighbourhood_size(v));
      if (one_16th < density) {
        double_dense_neighbourhoods++;
        double_dense_size_avg += src.neighbourhood_size(v);
      } else if (one_32th < density) {
        dense_neighbourhoods++;
        dense_size_avg += src.neighbourhood_size(v);
      } else if(256 < src.neighbourhood_size(v)) {
        dense_regions += find_dense_regions(src, v);
      }
    }
  }

  ofstream f(OUTPUT_PATH + dataset.get_name() + ".txt");

  auto total = src.vertex_count() - small_neighbourhoods;
  f << "Total: " << total << endl;
  f << "Small: " << small_neighbourhoods << " (" << ((float) small_neighbourhoods / (float) (src.vertex_count())) * 100 << ")" << endl;
  f << "Dense: " << dense_neighbourhoods << " (" << ((float) dense_neighbourhoods / (float) (total)) * 100 << ")" << "Average size: " << (float) dense_size_avg / (float) dense_neighbourhoods <<  endl;
  f << "Double dense: " << double_dense_neighbourhoods << " (" << ((float) double_dense_neighbourhoods / (float) (total)) * 100
    << ") " << "Average size: " << (float) double_dense_size_avg / (float) double_dense_neighbourhoods << endl;

  f << "Dense regions: " << dense_regions << " (" << ((float) dense_regions / ((float) src.adjacency_lists.size() / 128)) * 100 << ")" << endl;

  f.close();
  return 0;
}



