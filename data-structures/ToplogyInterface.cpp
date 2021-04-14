//
// Created by per on 31.08.20.
//

#include "ToplogyInterface.h"
#include <iostream>

#include <versioning/VersionedBlockedEdgeIterator.h>

TopologyInterface::~TopologyInterface() {
}

vertex_id_t TopologyInterface::logical_id(vertex_id_t id) {
  return id;
}

vertex_id_t TopologyInterface::physical_id(vertex_id_t id) {
  return id;
}

size_t TopologyInterface::neighbourhood_size(vertex_id_t src) {
  return neighbourhood_size_p(physical_id(src));
}

void TopologyInterface::neighbourhood(vertex_id_t src, BatchedEdgeIterator &iter) {
  return neighbourhood_p(physical_id(src), iter);
}

void TopologyInterface::neighbourhood(vertex_id_t src, EdgeIterator &iter) {
  return neighbourhood_p(physical_id(src), iter);
}

void TopologyInterface::intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t> &out) {
  return intersect_neighbourhood_p(physical_id(a), physical_id(b), out);
}

bool TopologyInterface::has_edge(edge_t edge) {
  return has_edge_p(edge_t(physical_id(edge.src), physical_id(edge.dst)));
}

bool TopologyInterface::has_vertex(vertex_id_t v) {
  return has_vertex_p(physical_id(v));
}

size_t TopologyInterface::max_physical_vertex() {
  return vertex_count(); // Assumes dense vertex sets
}

VersionedBlockedEdgeIterator TopologyInterface::neighbourhood_blocked_p(vertex_id_t src) {
  throw NotImplemented();
}
