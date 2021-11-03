//
// Created by per on 23.12.20.
//

#include <iostream>
#include "TransactionManager.h"

#include "EdgeBlock.h"
#include "VersioningBlockedSkipListAdjacencyList.h"

thread_local size_t TransactionManager::thread_id = 0;

SnapshotTransaction TransactionManager::getSnapshotTransaction(VersionedTopologyInterface* ti, bool write_only) {
  return SnapshotTransaction(this, write_only, ti);
}

void TransactionManager::getSnapshotTransaction(VersionedTopologyInterface *ti, bool write_only, SnapshotTransaction &existing_transaction_object) {
  existing_transaction_object.clear();
  if (!write_only) {
    existing_transaction_object.set_read_timestamp(draw_timestamp(false));
  }
}

void TransactionManager::transactionCompleted(const Transaction &transaction) {
  if (transaction.get_version() != active_snapshots[thread_id] && transaction.get_commit_version() != active_snapshots[thread_id]) {
    throw IllegalOperation("Thread tried to complete transaction, it did not open.");
  }
  active_snapshots[thread_id] = NO_TRANSACTION;
}

TransactionManager::TransactionManager(uint max_threads) : max_threads(max_threads) {
    reset_max_threads(max_threads);
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
  cout << "Thread " << id << " made " << calls_to_gc << " calls to gc." << endl;
  cout << "Thread " << id << " wrote " << multiple_versions_counter << " version chains." << endl;
  cout << "Thread " << id << " cleaned up " << pruned_multiple_versions << " version chains." << endl;
  VersioningBlockedSkipListAdjacencyList::print_statistics(id);
  lock_guard<mutex> l(thread_registry_lock);
  if (!thread_id_in_use[id]) {
    throw IllegalOperation("Trying to deregister a thread that has not been registered");
  }
  if (active_snapshots[id] != NO_TRANSACTION) {
    throw IllegalOperation("Trying to deregister a thread with an active transaction");
  }
  thread_id_in_use[id] = false;
}

void TransactionManager::reset_max_threads(uint max_threads) {
  lock_guard<mutex> l(thread_registry_lock);
  for (bool in_use : thread_id_in_use) {
    if (in_use) {
      throw IllegalOperation("Cannot change max_threads while any threads are registered");
    }
  }
  active_snapshots = vector<version_t>(max_threads, NO_TRANSACTION);
  thread_id_in_use = vector<bool>(max_threads, false);
}

version_t TransactionManager::draw_timestamp(bool commit_timestamp) {
  if (!commit_timestamp && active_snapshots[thread_id] != NO_TRANSACTION) {
    throw IllegalOperation("Cannot have more than one transaction open per thread.");
  }

  if (commit_timestamp && active_snapshots[thread_id] == NO_TRANSACTION) {
    active_snapshots[thread_id] = version.fetch_add(1);
    return active_snapshots[thread_id];
  } else if (commit_timestamp) {
    return version.fetch_add(1);  // Return a new commit version for a SnapshotTransaction which has a read version already.
  } else {
    active_snapshots[thread_id] = version.fetch_add(1);
    return active_snapshots[thread_id];
  }
}
