//
// Created by per on 31.08.20.
//

#ifndef LIVE_GRAPH_TWO_DATA_TYPES_H
#define LIVE_GRAPH_TWO_DATA_TYPES_H

#include <cstdint>

typedef uint32_t vertex_id_t;
typedef vertex_id_t dst_t;

struct edge_t {
    vertex_id_t src;
    dst_t dst;
};


#endif //LIVE_GRAPH_TWO_DATA_TYPES_H
