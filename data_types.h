//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_DATA_TYPES_H
#define LIVE_GRAPH_TWO_DATA_TYPES_H

#include <cstdint>
#include <ctime>
#include <unordered_set>
#include <algorithm>

typedef uint32_t vertex_id_t;
typedef vertex_id_t dst_t;

struct edge_t {
    vertex_id_t src;
    dst_t dst;

    bool operator==(const edge_t other) const {
      return src == other.src && dst == other.dst;
    }
};

struct temporal_edge_t {
    vertex_id_t src;
    dst_t dst;
    time_t creation_timestamp;
};

struct TemporalEdgeEqual {
public:
    bool operator()(const temporal_edge_t& a, const temporal_edge_t& b) const {
      return a.src == b.src && a.dst == b.dst;
    }
};

struct TemporalEdgeHash {
public:
    size_t operator()(const temporal_edge_t& e) const {
      return std::hash<vertex_id_t>()(e.src) + 31 * std::hash<dst_t>()(e.dst);
    }
};

#endif //LIVE_GRAPH_TWO_DATA_TYPES_H
