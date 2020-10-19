//
// Created by per on 14.10.20.
//

#ifndef LIVE_GRAPH_TWO_FILTEREDVECTORITERATOR_H
#define LIVE_GRAPH_TWO_FILTEREDVECTORITERATOR_H


#include <data_types.h>
#include "EdgeIterator.h"

class FilteredVectorIterator : public EdgeIterator {
public:
    void initialize(dst_t* data, size_t size);
    bool has_next();
    dst_t next();

protected:
    dst_t* data;
    dst_t* end;

    const dst_t empty = std::numeric_limits<dst_t>::max();
};


#endif //LIVE_GRAPH_TWO_FILTEREDVECTORITERATOR_H
