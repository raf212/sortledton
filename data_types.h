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
// The first bit of a dst_t type is set if the edge is versioned.
#define VERSION_MASK (1 << 31)

bool is_versioned(dst_t e);

dst_t make_versioned(dst_t e);

dst_t make_unversioned(dst_t e);

// version timestamp if the first bit is set there are further versions, if the second bit is set this version is a deletion.
typedef uint32_t version_t;
#define MORE_VERSION_MASK (1 << 31)
#define DELETION_MASK (1 << 30)

bool more_versions_existing(version_t v);

bool is_deletion(version_t v);

version_t timestamp(version_t v);

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
