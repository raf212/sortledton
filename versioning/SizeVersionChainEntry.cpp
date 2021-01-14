//
// Created by per on 13.01.21.
//

#include "SizeVersionChainEntry.h"

SizeVersionChainEntry *SizeVersionChainEntry::traverse(version_t version) {
  return (SizeVersionChainEntry*) VersionChainEntry::traverse(version);
}

SizeVersionChainEntry::SizeVersionChainEntry(version_t version, uint32_t current_size, bool deletion,
                                             SizeVersionChainEntry *next)
                                             : VersionChainEntry(version, deletion, next), current_size(current_size) {

}
