//
// Created by per on 13.01.21.
//

#ifndef LIVE_GRAPH_TWO_SIZEVERSIONCHAINENTRY_H
#define LIVE_GRAPH_TWO_SIZEVERSIONCHAINENTRY_H

#include <data_types.h>

class SizeVersionChainEntry {
public:
    SizeVersionChainEntry* next;
    version_t version;
    uint32_t current_size;

    SizeVersionChainEntry(version_t version, uint32_t current_size, SizeVersionChainEntry* next);

    /**
     * Traverses a given version chain until it finds the correct version.
     *
     * @param version version to read.
     * @return a pointer to the entry for version
     */
    SizeVersionChainEntry* traverse(version_t version);
};


#endif //LIVE_GRAPH_TWO_SIZEVERSIONCHAINENTRY_H
