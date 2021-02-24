//
// Created by per on 23.02.21.
//

#include <cassert>
#include <optional>
#include <iostream>

#include "VertexIndex.h"
#include <ToplogyInterface.h>

logical_vertex_id_t VertexIndex::logical_id(vertex_id_t v) const {
  assert(v < high_water_mark);
  return physical_to_logical[v];
}

optional<vertex_id_t> VertexIndex::physical_id(const logical_vertex_id_t v) const {
  l_t_p_table::accessor a;
  logical_to_physical.find(a, v);
  return a.empty() ? nullopt : make_optional(a->second);
}

void* const & VertexIndex::operator[](size_t v) const {
  return index[v];
}

void*& VertexIndex::operator[](size_t v) {
  return index[v];
}

vertex_id_t VertexIndex::insert_vertex(logical_vertex_id_t id, version_t version) {
  vertex_id_t p_id;
//  if (free_list.try_pop(p_id)) {
    p_id = high_water_mark.fetch_add(1);

    grow_vector_if_smaller(index, p_id * 2 + 1);
    grow_vector_if_smaller(physical_to_logical, p_id);
//  }

  // Update physical mapping, scoping to release accessor.
  {
    l_t_p_table::accessor w;
    logical_to_physical.insert(w, id);
    w->second = p_id;
  }

  // Update index
  index[p_id * 2] = nullptr;
  index[p_id * 2 + 1] = 0;

  // Update logical mapping
  physical_to_logical[p_id] = id;

  // Update vertex count
  vertex_count.fetch_add(1);

  return p_id;
}

size_t VertexIndex::get_high_water_mark() {
  return high_water_mark.load();
}

size_t VertexIndex::get_vertex_count(version_t version) {
  return vertex_count.load();
}
