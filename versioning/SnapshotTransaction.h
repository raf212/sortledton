//
// Created by per on 23.12.20.
//

#ifndef LIVE_GRAPH_TWO_SNAPSHOTTRANSACTION_H
#define LIVE_GRAPH_TWO_SNAPSHOTTRANSACTION_H

#include <memory>
#include <vector>
#include <set>

#include "Transaction.h"
#include "IllegalOperation.h"
#include "VertexExistsPrecondition.h"

class SnapshotTransaction : public Transaction {
public:
    SnapshotTransaction(version_t version, VersionedTopologyInterface* ds);
    ~SnapshotTransaction();

    void use_vertex_does_not_exists_semantics() override;
    void use_edge_does_not_exists_semantics() override;

    void register_precondition(Precondition* c);

    bool execute();

    size_t vertex_count() override;

    bool has_vertex_p(vertex_id_t v) override;
    bool insert_vertex(vertex_id_t v) override;
    bool delete_vertex(vertex_id_t v) override;

    size_t edge_count() override;
    bool insert_edge(edge_t edge) override;
    bool delete_edge(edge_t edge) override;

    size_t neighbourhood_size_p(vertex_id_t src) override;

    void neighbourhood_p(vertex_id_t src, BatchedEdgeIterator& iter) override { throw NotImplemented(); };

    void neighbourhood_p(vertex_id_t src, EdgeIterator& iter) override;

    /**
     * Cannot be used. Use raw_ds instead.
     */
    void* raw_neighbourhood(vertex_id_t src) override { throw NotImplemented(); };

    /**
     *  Be aware that you need to handle locking and versioning yourself.
     *
     * Passing any other version than the one of this transaction is undefined behaviour.
     *
     * Using the pointer after calling execute or TransactionManager.transactionCompleted is undefined behaviour.
     */
    VersionedTopologyInterface* raw_ds();

    void intersect_neighbourhood_p(vertex_id_t a, vertex_id_t b, vector<dst_t>& out) override;

    bool has_edge_p(edge_t edge) override;

    void bulkload(const SortedCSRDataSource& src) override;

    void report_storage_size() override;

    version_t get_version() const override;
    void set_version(version_t v);

    void clear();
protected:
    version_t version;
    VersionedTopologyInterface* ds;

private:
    // TODO check how much performanc it cost to make this class thread safe by a mutex on each writing function.
    // if this is to expensive reintroduce a thread safe readonly transaction and note about thread safety in the documentation.

    void aquire_locks();
    void release_locks();
    void assert_preconditions();

    bool vertex_does_not_exists_semantic_activated = false;
    bool edge_does_not_exists_semantic_activated = false;

    vector<Precondition*> preconditions {};
    vector<vertex_id_t> locks_to_aquire {};
    vector<vertex_id_t> vertices_to_delete {};
    vector<vertex_id_t> vertices_to_insert {};
    vector<vertex_id_t> vertices_to_insert_if_not_exists {};
    vector<edge_t> edges_to_delete {};
    vector<edge_t> edges_to_insert {};
};


#endif //LIVE_GRAPH_TWO_SNAPSHOTTRANSACTION_H
