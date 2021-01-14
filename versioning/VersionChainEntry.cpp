//
// Created by per on 23.12.20.
//

#include "VersionChainEntry.h"

VersionChainEntry *VersionChainEntry::traverse(version_t version) {
  while (this->next != nullptr && this->version > version) {
    return this->traverse(version);
  }
  return this;
}

VersionChainEntry::VersionChainEntry(version_t version, bool deletion, VersionChainEntry* next)
  : version(version), deletion(deletion), next(next) {

}
