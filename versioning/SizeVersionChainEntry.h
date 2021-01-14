//
// Created by per on 13.01.21.
//

#ifndef LIVE_GRAPH_TWO_SIZEVERSIONCHAINENTRY_H
#define LIVE_GRAPH_TWO_SIZEVERSIONCHAINENTRY_H


#include "VersionChainEntry.h"

class SizeVersionChainEntry : VersionChainEntry {
public:
    uint32_t current_size;
    SizeVersionChainEntry* traverse(version_t version) override;

    SizeVersionChainEntry(version_t version, uint32_t current_size, bool deletion, SizeVersionChainEntry* next);
};


#endif //LIVE_GRAPH_TWO_SIZEVERSIONCHAINENTRY_H
