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
    // TODO lower to Topology interface
    virtual bool has_vertex(vertex_id_t v) = 0;
    virtual void insert_vertex(vertex_id_t v) = 0;
    virtual void delete_vertex(vertex_id_t v) = 0;

    virtual size_t edge_count() = 0;  // TODO lower to Topology interface
    vertex_id_t insert_vertex() override { throw NotImplemented(); };
    void delete_vertex() override { throw NotImplemented(); };

    bool insert_safe(edge_t e) override { throw NotImplemented(); }

    virtual version_t get_version() const = 0;
};


#endif //LIVE_GRAPH_TWO_TRANSACTION_H
