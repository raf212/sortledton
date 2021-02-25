//
// Created by per on 13.01.21.
//

#include "SizeVersionChainEntry.h"

#include <cassert>

SizeVersionChainEntry *SizeVersionChainEntry::traverse(version_t version, uint depth = 0) {
  while (this->next != nullptr && this->version > version) {
    if (depth < 20) {
      return this->traverse(version, depth + 1);
    } else {
      assert(false);
    }

  }
  return this;
}

SizeVersionChainEntry::SizeVersionChainEntry(version_t version, uint32_t current_size, SizeVersionChainEntry *next)
                                             : next(next), version(version), current_size(current_size) {

}
