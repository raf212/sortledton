//
// Created by per on 13.01.21.
//

#include "SizeVersionChainEntry.h"

SizeVersionChainEntry *SizeVersionChainEntry::traverse(version_t version) {
  while (this->next != nullptr && this->version > version) {
    return this->traverse(version);
  }
  return this;
}

SizeVersionChainEntry::SizeVersionChainEntry(version_t version, uint32_t current_size, SizeVersionChainEntry *next)
                                             : version(version), next(next), current_size(current_size) {

}
