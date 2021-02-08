//
// Created by per on 23.12.20.
//

#include <iostream>
#include "TransactionManager.h"

thread_local size_t TransactionManager::thread_id = 0;

SnapshotTransaction TransactionManager::getSnapshotTransaction(VersionedTopologyInterface* ti) {
  if (active_snapshots[thread_id] != NO_TRANSACTION) {
    throw IllegalOperation("Cannot have more than one transaction open per thread.");
  }
  active_snapshots[thread_id] = version.fetch_add(1);

  return SnapshotTransaction(active_snapshots[thread_id], ti);
}

void TransactionManager::getSnapshotTransaction(VersionedTopologyInterface *ti, SnapshotTransaction &existing_transaction_object) {
  if (active_snapshots[thread_id] != NO_TRANSACTION) {
    throw IllegalOperation("Cannot have more than one transaction open per thread.");
  }
  active_snapshots[thread_id] = version.fetch_add(1);
  existing_transaction_object.clear();
  existing_transaction_object.set_version(active_snapshots[thread_id]);
}

void TransactionManager::transactionCompleted(const Transaction &transaction) {
  if (transaction.get_version() != active_snapshots[thread_id]) {
    throw IllegalOperation("Thread tried to complete transaction, it did not open.");
  }
  active_snapshots[thread_id] = NO_TRANSACTION;
}

SnapshotTransaction TransactionManager::getSnapshotTransaction(VersionedTopologyInterface *ti, version_t v) {
//  cerr << "Warning: creating snapshot transaction with custom version, this is not save in connection with GC, use only if you know what you are doing." << endl;
  return SnapshotTransaction(v, ti);
}

TransactionManager::TransactionManager(uint max_threads) : max_threads(max_threads),
  active_snapshots(max_threads, NO_TRANSACTION),
  thread_id_in_use(max_threads, false) {
    stopped.store(false);
    min_version_updater = thread(&TransactionManager::run_min_version_updater, this, MIN_VERSION_UPDATER_INTERVAL);
}

void TransactionManager::register_thread(size_t id) {
  lock_guard<mutex> l(thread_registry_lock);
  if (thread_id_in_use[id]) {
    throw IllegalOperation("Tyring to reuse a thread id.");
  }
  thread_id_in_use[id] = true;
  thread_id = id;
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
    this_thread::sleep_for(chrono::microseconds(interval));
  }
}

TransactionManager::~TransactionManager() {
  stopped.store(true);
  min_version_updater.join();
}

void TransactionManager::deregister_thread(size_t id) {
  lock_guard<mutex> l(thread_registry_lock);
  if (!thread_id_in_use[id]) {
    throw IllegalOperation("Trying to deregister a thread that has not been registered");
  }
  if (active_snapshots[id] != NO_TRANSACTION) {
    throw IllegalOperation("Trying to deregister a thread with an active transaction");
  }
  thread_id_in_use[id] = false;
}

