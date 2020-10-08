//
// Created by per on 08.10.20.
//

#ifndef LIVE_GRAPH_TWO_ASSERTS_H
#define LIVE_GRAPH_TWO_ASSERTS_H

#include "../../data-src/SortedCSRDataSource.h"


namespace testing {

    class Asserts {
    public:
        static void assert_csr(SortedCSRDataSource &src, vector<pair<dst_t, dst_t>> &edges);
    };

}

#endif //LIVE_GRAPH_TWO_ASSERTS_H
