//
// Created by per on 14.01.21.
//

#ifndef LIVE_GRAPH_TWO_VERTEXEXISTSPRECONDITION_H
#define LIVE_GRAPH_TWO_VERTEXEXISTSPRECONDITION_H


#include "Precondition.h"

class VertexExistsPrecondition : Precondition {
public:
  ~VertexExistsPrecondition();
  bool assert_it(VersionedTopologyInterface& ds, version_t version) override;
  vector<vertex_id_t> requires_vertex_locks() override;
};


#endif //LIVE_GRAPH_TWO_VERTEXEXISTSPRECONDITION_H
