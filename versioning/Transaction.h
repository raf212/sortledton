//
// Created by per on 23.12.20.
//

#ifndef LIVE_GRAPH_TWO_TRANSACTION_H
#define LIVE_GRAPH_TWO_TRANSACTION_H

#include <data_types.h>
#include <utils/NotImplemented.h>
#include <data-structures/ToplogyInterface.h>
#include "Precondition.h"

class Transaction : public TopologyInterface {
public :
    virtual void use_does_not_exists_semantics();
    virtual void use_vertex_does_not_exists_semantics() = 0;
    virtual void use_edge_does_not_exists_semantics() = 0;

    bool insert_safe(edge_t e) override { throw NotImplemented(); }

    virtual version_t get_version() const = 0;
};


#endif //LIVE_GRAPH_TWO_TRANSACTION_H
