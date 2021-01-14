//
// Created by per on 23.12.20.
//

#ifndef LIVE_GRAPH_TWO_READONLYTRANSACTION_H
#define LIVE_GRAPH_TWO_READONLYTRANSACTION_H


#include "Transaction.h"
#include "IllegalOperation.h"
#include "SnapshotTransaction.h"

class ReadOnlyTransaction : Transaction {
private:
    SnapshotTransaction transaction;

public:
    ReadOnlyTransaction(SnapshotTransaction trans) : transaction(trans) {
    }

    ~ReadOnlyTransaction();

    size_t vertex_count() override;

    void insert_vertex(vertex_id_t v) override {
      throw new IllegalOperation();
    };

    void delete_vertex(vertex_id_t v) override {
      throw new IllegalOperation();
    };

    vertex_id_t insert_vertex() override {
      throw IllegalOperation();
    }

    void delete_vertex() override {
      throw IllegalOperation();
    };

    void insert_edge(edge_t edge) override {
      throw IllegalOperation();
    };

    void delete_edge(edge_t edge) override {
      throw IllegalOperation();
    };

    size_t neighbourhood_size(vertex_id_t src) override {
      return transaction.neighbourhood_size(src);
    };

    void neighbourhood(vertex_id_t src, BatchedEdgeIterator& iter) override{
      return transaction.neighbourhood(src , iter);
    };

    void neighbourhood(vertex_id_t src, EdgeIterator& iter) override {
      return transaction.neighbourhood(src , iter);
    };

    void* raw_neighbourhood(vertex_id_t src) override {
      return transaction.raw_neighbourhood(src);
    };

    void intersect_neighbourhood(vertex_id_t a, vertex_id_t b, vector<dst_t>& out) override {
      return transaction.intersect_neighbourhood(a, b, out);
    };

    bool has_edge(edge_t edge) override {
      return transaction.has_edge(edge);
    };

    void bulkload(const SortedCSRDataSource& src) override {
      throw IllegalOperation();
    };

    void report_storage_size() override {
      transaction.report_storage_size();
    };

};


#endif //LIVE_GRAPH_TWO_READONLYTRANSACTION_H
