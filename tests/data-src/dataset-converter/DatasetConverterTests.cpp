
#include "gtest/gtest.h"

#include <string>
#include <algorithm>

#include "../Asserts.h"
#include "../../../data-src/dataset-converter/DatasetConverter.h"

namespace testing {

    using namespace std;

    vector<pair<dst_t, dst_t>> example_graph{
            {2, 3},
            {2, 4},
            {3, 4},
            {3, 5},
            {3, 8},
            {5, 6},
            {5, 8},
            {6, 7},
            {6, 8},
            {6, 9},
            {6, 10},
            {7, 9},
    };

    vector<pair<dst_t, dst_t>> example_graph_densified{
            {0, 1},
            {0, 2},
            {1, 2},
            {1, 3},
            {1, 4},
            {3, 4},
            {3, 5},
            {5, 4},
            {5, 6},
            {5, 7},
            {5, 8},
            {6, 7},

    };

    vector<pair<dst_t, dst_t>> example_graph_densified_undirected{
            {0, 1},
            {0, 2},
            {1, 0},
            {1, 2},
            {1, 3},
            {1, 4},
            {2, 0},
            {2, 1},
            {3, 1},
            {3, 4},
            {3, 5},
            {4, 1},
            {4, 3},
            {4, 5},
            {5, 3},
            {5, 4},
            {5, 6},
            {5, 7},
            {5, 8},
            {6,5 },
            {6, 7},
            {7,5},
            {7,6},
            {8, 5}
    };

    TEST(DatasetConverterTestSuite, ParseDirected) {
      char* argv[8] = {"converter", "--densify", "--insert_percentage",  "0.0", "--delete_percentage", "0.0",  "/space/fuchs/shared/datasets/example-undirected.e" ,"/space/fuchs/shared/graph-two-testing/"};
      size_t argc = 8;

      DatasetConverter conv(argc, argv);
      conv.run();

      SortedCSRDataSource csr;
      csr.read_from_binary_file("/space/fuchs/shared/graph-two-testing/base.csr");

      Asserts::assert_csr(csr, example_graph_densified);
    }

    TEST(DatasetConverterTestSuite, ParseUndirected) {
      char* argv[5] = {"converter", "--densify", "--make_undirected",
                       "/space/fuchs/shared/datasets/example-undirected.e", "/space/fuchs/shared/graph-two-testing/"};
      size_t argc = 5;

      DatasetConverter conv(argc, argv);
      conv.run();

      SortedCSRDataSource csr;
      csr.read_from_binary_file("/space/fuchs/shared/graph-two-testing/undirected_base.csr");

      Asserts::assert_csr(csr, example_graph_densified_undirected);
    }
}
