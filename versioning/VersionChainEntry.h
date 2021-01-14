//
// Created by per on 23.12.20.
//

#ifndef LIVE_GRAPH_TWO_VERSIONCHAINENTRY_H
#define LIVE_GRAPH_TWO_VERSIONCHAINENTRY_H


#include <data_types.h>

class VersionChainEntry {
public:
    VersionChainEntry* next;
    version_t version;
    bool deletion;


    VersionChainEntry(version_t version, bool deletion, VersionChainEntry* next);

    /**
     * Traverses a given version chain until it finds the correct version.
     *
     * @param version version to read.
     * @return a pointer to the entry for version
     */
    virtual VersionChainEntry* traverse(version_t version);
};


#endif //LIVE_GRAPH_TWO_VERSIONCHAINENTRY_H
