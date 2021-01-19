//
// Created by per on 23.12.20.
//

#ifndef LIVE_GRAPH_TWO_TRANSACTIONMANAGER_H
#define LIVE_GRAPH_TWO_TRANSACTIONMANAGER_H


#include <atomic>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_map>

#include "Transaction.h"
#include "SerializableUpdateTransaction.h"
#include "ReadOnlyTransaction.h"

#define NO_TRANSACTION numeric_limits<version_t>::max()

class TransactionManager {

public:
    explicit TransactionManager(uint threads);

    size_t register_thread();

    SerializableUpdateTransaction getWriteOnlyUpdateTransaction(VersionedTopologyInterface* ti, size_t thread_id);
    ReadOnlyTransaction getReadOnlyTransaction(VersionedTopologyInterface* ti, size_t thread_id);
    SnapshotTransaction getSnapshotTransaction(VersionedTopologyInterface* ti, version_t v, size_t thread_id);
    SnapshotTransaction getSnapshotTransaction(VersionedTopologyInterface* ti, size_t thread_id);

    void transactionCompleted(const Transaction& transaction, size_t thread_id);

    version_t getMinActiveVersion();

private:
    uint threads;
    uint last_thread_id =0;
    unordered_map<thread::id, size_t> thread_id_mapping;
    mutex global_lock;

    vector<version_t> active_snapshots;
    atomic<version_t> version {1};
    version_t min_version { numeric_limits<version_t>::min()};
};


#endif //LIVE_GRAPH_TWO_TRANSACTIONMANAGER_H
