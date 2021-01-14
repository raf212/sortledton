//
// Created by per on 23.12.20.
//

#ifndef LIVE_GRAPH_TWO_PRECONDITION_H
#define LIVE_GRAPH_TWO_PRECONDITION_H


#include "VersionedTopologyInterface.h"

class Precondition {
public:
    virtual ~Precondition();

    virtual bool assert_it(VersionedTopologyInterface& ds, version_t version) = 0;
    virtual vector<vertex_id_t> requires_vertex_locks() = 0;
};


#endif //LIVE_GRAPH_TWO_PRECONDITION_H
