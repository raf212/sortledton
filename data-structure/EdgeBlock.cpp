//
// Created by per on 23.03.21.
//

#include "EdgeBlock.h"

thread_local ulong calls_to_gc = 0;
thread_local ulong multiple_versions_counter = 0;
thread_local ulong pruned_multiple_versions = 0;