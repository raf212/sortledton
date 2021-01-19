//
// Created by per on 23.12.20.
//

#include <iostream>
#include "TransactionManager.h"

SnapshotTransaction TransactionManager::getSnapshotTransaction(VersionedTopologyInterface* ti) {
  // TODO do not allow to open more than one transaction per thread.
  size_t id = thread_id_mapping.find(this_thread::get_id())->second;
  active_snapshots[id] = version.fetch_add(1);

  return SnapshotTransaction(active_snapshots[id], ti);
}

ReadOnlyTransaction TransactionManager::getReadOnlyTransaction(VersionedTopologyInterface* ti) {
  return ReadOnlyTransaction(getSnapshotTransaction(ti));
}

SerializableUpdateTransaction TransactionManager::getWriteOnlyUpdateTransaction(VersionedTopologyInterface* ti) {
  return SerializableUpdateTransaction(getSnapshotTransaction(ti));
}

void TransactionManager::transactionCompleted(const Transaction &transaction) {
  auto id = thread_id_mapping.find(this_thread::get_id())->second;
  active_snapshots[id] = NO_TRANSACTION;
  min_version = min(version.load(), *min_element(active_snapshots.begin(), active_snapshots.end()));
}

SnapshotTransaction TransactionManager::getSnapshotTransaction(VersionedTopologyInterface *ti, version_t v) {
//  cerr << "Warning: creating snapshot transaction with custom version, this is not save in connection with GC, use only if you know what you are doing." << endl;
  return SnapshotTransaction(v, ti);
}

TransactionManager::TransactionManager(uint threads) : threads(threads) {
  active_snapshots = vector<version_t>(threads, NO_TRANSACTION);
}

void TransactionManager::register_thread() {
  lock_guard<mutex> l(global_lock);
  thread_id_mapping.insert({this_thread::get_id(), last_thread_id++});
}
