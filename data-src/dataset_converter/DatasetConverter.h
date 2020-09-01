//
// Created by per on 01.09.20.
//

#ifndef LIVE_GRAPH_TWO_DATASETCONVERTER_H
#define LIVE_GRAPH_TWO_DATASETCONVERTER_H


#include <cstddef>
#include <data-src/SortedCSRDataSource.h>

#include "Options.h"
#include "data_types.h"

using namespace std;

/**
 * Generate binary graph representations from text files.
 *
 * Can parse csv files as found on konect.cc.
 * Can parse files with edge creation timestamp.
 * Cannot parse files with creations and deletions yet, e.g. wikipedia hyperlinks.
 *
 * Insertions are sampled randomly and output is shuffled if the network is parsed as none temporal.
 * If the network is parsed as temporal network, insertions are chosen to be the last x% percent of edges and are
 * inserted according to their insertion timestamp.
 * Deletetions are sampled randomly and output is shuffled.
 *
 * Deletions and Insertion sets are none overlapping.
 */
class DatasetConverter {
public:
  DatasetConverter(int argc, char** argv);

  void run();

private:
    Options o;

    char detect_seperator(const string& path);
    vector<temporal_edge_t> parse_text_file(Options o);

    void write_insertion_set(vector<temporal_edge_t>::iterator begin, vector<temporal_edge_t>::iterator end);

    void write_deletion_set(vector<temporal_edge_t>::iterator begin, vector<temporal_edge_t>::iterator end);

    SortedCSRDataSource convert_to_sorted_csr(vector<temporal_edge_t>::iterator begin,
                                              vector<temporal_edge_t>::iterator end);

    void write_base_dataset(SortedCSRDataSource csr);
};


#endif //LIVE_GRAPH_TWO_DATASETCONVERTER_H
