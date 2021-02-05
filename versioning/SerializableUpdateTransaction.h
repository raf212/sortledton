//
// Created by per on 23.12.20.
//

#ifndef LIVE_GRAPH_TWO_SERIALIZABLEUPDATETRANSACTION_H
#define LIVE_GRAPH_TWO_SERIALIZABLEUPDATETRANSACTION_H

#include "Transaction.h"
#include "IllegalOperation.h"
#include "SnapshotTransaction.h"
#include <memory>
#include <utils/NotImplemented.h>

class SerializableUpdateTransaction : public Transaction {
private:
    SnapshotTransaction transaction;

public:
    SerializableUpdateTransaction(SnapshotTransaction trans) : transaction(trans) {}

    void register_precondition(Precondition* c) { transaction.register_precondition(c); };
    bool execute() { return transaction.execute(); };

    size_t vertex_count() override {
      throw IllegalOperation();
    }

    bool has_vertex(vertex_id_t v) override { throw IllegalOperation(); };
    void insert_vertex(vertex_id_t v) override { return transaction.insert_vertex(v); };
    void delete_vertex(vertex_id_t v) override { return transaction.delete_vertex(v); };

    size_t edge_count() override { throw IllegalOperation(); };

    void insert_edge(edge_t edge) override { return transaction.insert_edge(edge); };
    void delete_edge(edge_t edge) override { return transaction.delete_edge(edge); };;

    size_t neighbourhood_size(vertex_id_t src) override {
      throw IllegalOperation();
    };

    void neighbourhood(vertex_id_t src, BatchedEdgeIterator& iter) override {
      throw IllegalOperation();
    };

    void neighbourhood(vertex_id_t src, EdgeIterator& iter) override {
      throw IllegalOperation();
    };

    void* raw_neighbourhood(vertex_id_t src) override {
      throw IllegalOperation();
    };

    void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t>& out) override {
      throw IllegalOperation();
    };

    bool has_edge(edge_t edge) override {
      throw IllegalOperation();
    };

    void bulkload(const SortedCSRDataSource& src) override {
      throw IllegalOperation();
    };

    void report_storage_size() override {
      throw NotImplemented();
    };

    version_t get_version() const override  {
      return transaction.get_version();
    };
};


#endif //LIVE_GRAPH_TWO_SERIALIZABLEUPDATETRANSACTION_H
