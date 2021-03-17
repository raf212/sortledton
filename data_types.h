//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_DATA_TYPES_H
#define LIVE_GRAPH_TWO_DATA_TYPES_H

#include <cstdint>
#include <ctime>
#include <unordered_set>
#include <algorithm>


//#ifdef BITS64
// Used vertex identifier and destination data structure for all data structrues.
  typedef uint64_t vertex_id_t;
  typedef vertex_id_t dst_t;
  typedef double weight_t;

  // Version used to indicate that this is the first version of any version chain. This does not need to be
  // the original first version from system start but could be a later version after GC.
  #define FIRST_VERSION 0L
  // The first bit of a dst_t type is set if the edge is versioned.
  #define VERSION_MASK (1L << 63)

// version timestamp if the second bit is set there are further versions, if the third bit is set this version is a deletion.
// it is important that the first bit is never set
// TODO change this around, a first bit set indicates a version. while an unset first bit indicates that is not a version.
  typedef uint64_t version_t;
  #define MORE_VERSION_MASK (1L << 62)
  #define DELETION_MASK (1L << 61)
//#endif
#ifdef BITS32
// Used vertex identifier and destination data structure for all data structrues.
  typedef uint32_t vertex_id_t;
  typedef vertex_id_t dst_t;


  // Version used to indicate that this is the first version of any version chain. This does not need to be
  // the original first version from system start but could be a later version after GC.
  #define FIRST_VERSION 0
  // The first bit of a dst_t type is set if the edge is versioned.
  #define VERSION_MASK (1 << 31)

  // version timestamp if the second bit is set there are further versions, if the third bit is set this version is a deletion.
// it is important that the first bit is never set
// TODO change this around, a first bit set indicates a version. while an unset first bit indicates that is not a version.
  typedef uint32_t version_t;
  #define MORE_VERSION_MASK (1 << 30)
  #define DELETION_MASK (1 << 29)
#endif

#define make_versioned(e) (e | VERSION_MASK)
#define make_unversioned(e) (e & ~VERSION_MASK)
#define is_versioned(e) (e & VERSION_MASK)

//bool is_versioned(dst_t e);
//
//dst_t make_versioned(dst_t e);
//
//dst_t make_unversioned(dst_t e);

bool more_versions_existing(version_t v);

bool is_deletion(version_t v);

version_t timestamp(version_t v);

struct edge_t {
    vertex_id_t src;
    dst_t dst;

    edge_t() : src(0), dst(0) {};
    edge_t(vertex_id_t src, dst_t dst) : src(src), dst(dst) {};

    bool operator==(const edge_t other) const {
      return src == other.src && dst == other.dst;
    }
};

struct weighted_edge_t {
    vertex_id_t src;
    dst_t dst;
    weight_t weight;

    weighted_edge_t() : src(0), dst(0), weight(0.0) {};
    weighted_edge_t(vertex_id_t src, dst_t dst, weight_t weight) : src(src), dst(dst), weight(weight) {};
    weighted_edge_t(edge_t e) : weighted_edge_t(e.src, e.dst, 0.0) {};

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
