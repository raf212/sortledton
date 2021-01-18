//
// Created by per on 23.12.20.
//

#include <iostream>
#include "TransactionManager.h"

SnapshotTransaction TransactionManager::getSnapshotTransaction(VersionedTopologyInterface* ti) {
  lock_guard<mutex> l(global_lock);
  version_t v = version.fetch_add(1);

  if (v < min_version) {  // This is the case if there was no active tranaction in the system before.
    min_version = v;
  }

  active_versions.insert(version);
  return SnapshotTransaction(v, ti);
}

ReadOnlyTransaction TransactionManager::getReadOnlyTransaction(VersionedTopologyInterface* ti) {
  return ReadOnlyTransaction(getSnapshotTransaction(ti));
}

SerializableUpdateTransaction TransactionManager::getWriteOnlyUpdateTransaction(VersionedTopologyInterface* ti) {
  return SerializableUpdateTransaction(getSnapshotTransaction(ti));
}

void TransactionManager::transactionCompleted(const Transaction &transaction) {
  lock_guard<mutex> l(global_lock);
  auto v = transaction.get_version();
  auto min = *(--active_versions.rend());
  active_versions.erase(v);
  if (v == min) {
    if (active_versions.empty()) {
      min_version = numeric_limits<version_t>::min();
    } else {
      min_version = *(--active_versions.rend());
    }
  }
}

SnapshotTransaction TransactionManager::getSnapshotTransaction(VersionedTopologyInterface *ti, version_t v) {
//  cerr << "Warning: creating snapshot transaction with custom version, this is not save in connection with GC, use only if you know what you are doing." << endl;
  return SnapshotTransaction(v, ti);
}
