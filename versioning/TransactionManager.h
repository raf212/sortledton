//
// Created by per on 23.12.20.
//

#ifndef LIVE_GRAPH_TWO_TRANSACTIONMANAGER_H
#define LIVE_GRAPH_TWO_TRANSACTIONMANAGER_H


#include <atomic>
#include <mutex>
#include <set>

#include "Transaction.h"
#include "SerializableUpdateTransaction.h"
#include "ReadOnlyTransaction.h"

#define FIRST_VERSION ((version_t) 0)

class TransactionManager {

public:
    SerializableUpdateTransaction getWriteOnlyUpdateTransaction(VersionedTopologyInterface& ti);
    ReadOnlyTransaction getReadOnlyTransaction(VersionedTopologyInterface& ti);
    SnapshotTransaction getSnapshotTransaction(VersionedTopologyInterface& ti);

    void transactionCompleted(SnapshotTransaction& transaction);

private:
    mutex global_lock {};
    atomic<version_t> version {1};
    set<version_t> active_versions;

    version_t min_version { numeric_limits<version_t>::max()};

};


#endif //LIVE_GRAPH_TWO_TRANSACTIONMANAGER_H
