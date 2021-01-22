//
// Created by per on 23.12.20.
//

#include <iostream>
#include "TransactionManager.h"

SnapshotTransaction TransactionManager::getSnapshotTransaction(VersionedTopologyInterface* ti, size_t thread_id) {
  // TODO do not allow to open more than one transaction per thread.
  active_snapshots[thread_id] = version.fetch_add(1);

  return SnapshotTransaction(active_snapshots[thread_id], ti);
}

ReadOnlyTransaction TransactionManager::getReadOnlyTransaction(VersionedTopologyInterface* ti, size_t thread_id) {
  return ReadOnlyTransaction(getSnapshotTransaction(ti, thread_id));
}

SerializableUpdateTransaction TransactionManager::getWriteOnlyUpdateTransaction(VersionedTopologyInterface* ti, size_t thread_id) {
  return SerializableUpdateTransaction(getSnapshotTransaction(ti, thread_id));
}

void TransactionManager::transactionCompleted(const Transaction &transaction, size_t thread_id) {
  active_snapshots[thread_id] = NO_TRANSACTION;
}

SnapshotTransaction TransactionManager::getSnapshotTransaction(VersionedTopologyInterface *ti, version_t v, size_t thread_id) {
//  cerr << "Warning: creating snapshot transaction with custom version, this is not save in connection with GC, use only if you know what you are doing." << endl;
  return SnapshotTransaction(v, ti);
}

TransactionManager::TransactionManager(uint threads) : threads(threads) {
  active_snapshots = vector<version_t>(threads, NO_TRANSACTION);
  min_version_updater = thread(&TransactionManager::run_min_version_updater, this, 2000);
}

size_t TransactionManager::register_thread() {
  lock_guard<mutex> l(global_lock);
  auto e = thread_id_mapping.find(this_thread::get_id());
  if (e != thread_id_mapping.end()) {
    return e->second;
  }
  auto id = last_thread_id++;
  thread_id_mapping.insert({this_thread::get_id(), id});
  return id;
}

version_t TransactionManager::getMinActiveVersion() {
  return min_version;
}

void TransactionManager::update_min_version() {
  min_version = min(version.load(), *min_element(active_snapshots.begin(), active_snapshots.end()));
}

void TransactionManager::run_min_version_updater(uint interval) {
  while (!stopped.load()) {
    update_min_version();
    this_thread::sleep_for(chrono::milliseconds(interval));
  }
}

TransactionManager::~TransactionManager() {
  stopped.store(true);
  min_version_updater.join();
}
