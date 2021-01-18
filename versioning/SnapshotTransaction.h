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

    void register_precondition(unique_ptr<Precondition> c);

    bool execute();

    size_t vertex_count() override;

    void insert_vertex(vertex_id_t v) override;
    void delete_vertex(vertex_id_t v) override;

    void insert_edge(edge_t edge) override;
    void delete_edge(edge_t edge) override;

    size_t neighbourhood_size(vertex_id_t src) override;

    void neighbourhood(vertex_id_t src, BatchedEdgeIterator& iter) override { throw NotImplemented(); };

    void neighbourhood(vertex_id_t src, EdgeIterator& iter) override { throw NotImplemented(); };

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

    void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t>& out) override;

    bool has_edge(edge_t edge) override;

    void bulkload(const SortedCSRDataSource& src) override;

    void report_storage_size() override;

    version_t get_version() const override;
protected:
    version_t version;
    VersionedTopologyInterface* ds;

private:
    void aquire_locks();
    void release_locks();
    bool assert_preconditions();

    vector<Precondition*> preconditions {};   // TODO add preconditions again
    vector<vertex_id_t> locks_to_aquire {};
    vector<vertex_id_t> vertices_to_delete {};
    vector<vertex_id_t> vertices_to_insert {};
    vector<edge_t> edges_to_delete {};
    vector<edge_t> edges_to_insert {};

};


#endif //LIVE_GRAPH_TWO_SNAPSHOTTRANSACTION_H
